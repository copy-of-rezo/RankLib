
//
// BulkTree
//
// 4 March 2005 -- tds
//

#include "lemur/lemur-compat.hpp"
#include "indri/indri-platform.h"
#include <assert.h>
#include <vector>
#include "indri/File.hpp"
#include "indri/delete_range.hpp"
#include "indri/BulkTree.hpp"

// add pair at begin block, add length at end of block
// 

const int BULK_BLOCK_SIZE = 16*1024;

int BulkBlock::_remainingCapacity() {
  int startDataSize = _dataEnd();
  return BULK_BLOCK_SIZE - startDataSize - count()*2*sizeof(UINT16);
}

int BulkBlock::_dataEnd() {
  return _valueEnd( count()-1 );
}

int BulkBlock::_keyEnd( int index ) {
  assert( index < count() );

  if( index <= -1 ) {
    return sizeof(UINT16);
  }

  UINT16* blockEnd = (UINT16*) (_buffer + BULK_BLOCK_SIZE);
  UINT16  keyEnd = blockEnd[ -(index*2+2) ];

  return keyEnd;
}

int BulkBlock::_keyStart( int index ) {
  return _valueEnd( index-1 );
}

int BulkBlock::_valueStart( int index ) {
  return _keyEnd( index );
}

int BulkBlock::_valueEnd( int index ) {
  assert( index < count() );

  if( index <= -1 ) {
    return sizeof(UINT16);
  }

  UINT16* blockEnd = (UINT16*) (_buffer + BULK_BLOCK_SIZE);
  UINT16  valueEnd = blockEnd[ -(index*2+1) ];

  return valueEnd;
}

bool BulkBlock::_canInsert( int keyLength, int dataLength ) {
  return _remainingCapacity() >= (keyLength + dataLength + 2*sizeof(UINT16));
}

void BulkBlock::_storeKeyValueLength( int insertPoint, int keyLength, int valueLength ) {
  UINT16* blockEnd = (UINT16*) (_buffer + BULK_BLOCK_SIZE);
  int cnt = count();

  blockEnd[ -(cnt*2+2) ] = (UINT16) keyLength + insertPoint;
  blockEnd[ -(cnt*2+1) ] = (UINT16) valueLength + keyLength + insertPoint;
}

int BulkBlock::_compare( const char* one, int oneLength, const char* two, int twoLength ) {
  int result = memcmp( one, two, lemur_compat::min( oneLength, twoLength ) );

  if( result != 0 ) {
    return result;
  }

  return oneLength - twoLength;
}

int BulkBlock::_find( const char* key, int keyLength, bool& exact ) {
  int left = 0;
  int right = count() - 1;

  while( right - left > 1 ) {
    int middle = left + (right - left) / 2;
    int middleKeyStart = _keyStart( middle );
    int middleKeyEnd = _keyEnd( middle );
    const char* middleKey = _buffer + middleKeyStart;

    int result = _compare( key, keyLength, middleKey, middleKeyEnd - middleKeyStart );

    if( result < 0 ) {
      right = middle;
    } else if( result > 0 ) {
      left = middle;
    } else {
      exact = true;
      return middle;
    }
  }

  const char* leftKey = _keyStart( left ) + _buffer;
  int leftLength = _keyEnd( left ) - _keyStart( left );
  int leftResult = _compare( key, keyLength, leftKey, leftLength );

  const char* rightKey = _keyStart( right ) + _buffer;
  int rightLength = _keyEnd( right ) - _keyStart( right );
  int rightResult = _compare( key, keyLength, rightKey, rightLength );

  // matches the left key
  if( leftResult == 0 ) {
    exact = true;
    return left;
  }

  // matches the right key
  if( rightResult == 0 ) {
    exact = true;
    return right;
  }

  // bigger than the right key; choose right
  if( rightResult > 0 ) {
    exact = false;
    return right;
  }

  // smaller than the left key; invalid!
  if( leftResult < 0 ) {
    exact = false;
    return -1;
  }

  return left;
}


BulkBlock::BulkBlock( bool leaf ) {
  _buffer = new char[BULK_BLOCK_SIZE];
  *(UINT16*) _buffer = (leaf ? (1<<15) : 0);
}

BulkBlock::~BulkBlock() {
  delete[] _buffer;
}

int BulkBlock::count() {
  return (*(UINT16*)_buffer) & ~(1<<15);
}

bool BulkBlock::leaf() {
  return ((*(UINT16*)_buffer) & (1<<15)) != 0;
}

bool BulkBlock::insert( const char* key, int keyLength, const char* data, int dataLength ) {
  if( _canInsert( keyLength, dataLength ) == false )
    return false;
  
  int insertPoint = _dataEnd();
  memcpy( _buffer + insertPoint, key, keyLength );
  memcpy( _buffer + insertPoint + keyLength, data, dataLength );

  _storeKeyValueLength( insertPoint, keyLength, dataLength );
  (*(UINT16*)_buffer) += 1;

  return true;
}

bool BulkBlock::getIndex( int index, char* key, int& keyActual, int keyLength, char* value, int& valueActual, int valueLength ) {
  int count = *(UINT16*) _buffer;

  keyActual = 0;
  valueActual = 0;

  if( index < 0 || index >= count )
    return false;

  if( key ) {
    int keyStart = _keyStart( index );
    int keyEnd = _keyEnd( index );

    keyActual = lemur_compat::min( keyEnd - keyStart, keyLength );
    memcpy( key, _buffer + keyStart, keyActual );
  }

  if( value ) {
    int valueStart = _valueStart( index );
    int valueEnd = _valueEnd( index );
    
    valueActual = lemur_compat::min( valueEnd - valueStart, valueLength );
    memcpy( value, _buffer + valueStart, valueActual );
  }

  return true;
}

bool BulkBlock::findGreater( const char* key, int keyLength, char* value, int& actualLength, int valueBufferLength ) {
  bool exact;
  int index = _find( key, keyLength, exact );
  actualLength = 0;

  if( index < 0 )
    return false;

  int keyActual;
  return getIndex( index, 0, keyActual, 0, value, actualLength, valueBufferLength );
}

bool BulkBlock::find( const char* key, int keyLength, char* value, int& actualLength, int valueBufferLength ) {
  bool exact;
  int index = _find( key, keyLength, exact );
  actualLength = 0;

  if( index < 0 || !exact )
    return false;

  int keyActual;
  return getIndex( index, 0, keyActual, 0, value, actualLength, valueBufferLength );
}

bool BulkBlock::insertFirstKey( BulkBlock& block, UINT32 blockID ) {
  assert( block.count() > 0 );

  int startKey = block._keyStart( 0 );
  int endKey = block._keyEnd( 0 );

  return insert( block._buffer + startKey, endKey - startKey, (const char*) &blockID, sizeof(blockID) );
}

void BulkBlock::clear() {
  *(UINT16*) _buffer = (leaf() ? (1<<15) : 0);
}

char* BulkBlock::data() {
  return _buffer;
}

int BulkBlock::dataSize() {
  return BULK_BLOCK_SIZE;
}

// ==============
// BulkTreeWriter
// ==============


void BulkTreeWriter::_flush( int blockIndex ) {
  BulkBlock& flusher = *_blocks[blockIndex];

  if( blockIndex < _blocks.size() - 1 ) {
    BulkBlock& parent = *_blocks[blockIndex+1];
    
    if( ! parent.insertFirstKey( flusher, _blockID ) ) {
      _flush( blockIndex+1 );
      parent.insertFirstKey( flusher, _blockID );
    } 
  } else {
    _blocks.push_back( new BulkBlock );
    _blocks.back()->insertFirstKey( flusher, _blockID );
  }

  _write.write( flusher.data(), BulkBlock::dataSize() );
  flusher.clear();
  _blockID++;
  _flushLevel = lemur_compat::max( blockIndex, _flushLevel );
}

void BulkTreeWriter::_flushAll() {
  // note: _flushLevel may grow during this loop
  int originalSize = _blocks.size();

  for( int i=0; i<_blocks.size(); i++ ) {
    bool hasNotBeenFlushed = (i > _flushLevel);
    int count = _blocks[i]->count();

    if( count == 1 && hasNotBeenFlushed )
      break;

    if( count )
      _flush( i );
  }
}

BulkTreeWriter::BulkTreeWriter() :
  _write( _file, 1024*1024 )
{
  _blockID = 0;
  _blocks.push_back( new BulkBlock(true) );
}

BulkTreeWriter::~BulkTreeWriter() {
  delete_vector_contents( _blocks );
}

void BulkTreeWriter::create( const std::string& filename ) {
  _file.create( filename );
}

void BulkTreeWriter::put( UINT32 key, const char* value, int valueLength ) {
  put( (const char*) &key, sizeof(key), value, valueLength );
}

void BulkTreeWriter::put( const char* key, const char* value, int valueLength ) {
  put( key, strlen(key), value, valueLength );
}

void BulkTreeWriter::put( const char* key, int keyLength, const char* value, int valueLength ) {
  bool simple = _blocks.front()->insert( key, keyLength, value, valueLength );

  if( !simple ) {
    _flush( 0 );
    _blocks.front()->insert( key, keyLength, value, valueLength );
  }
}

bool BulkTreeWriter::get( const char* key, int keyLength, char* value, int& actual, int valueLength ) {
  BulkTreeReader reader( _file, _blockID*BulkBlock::dataSize() );
  return reader.get( key, keyLength, value, actual, valueLength );
}

bool BulkTreeWriter::get( const char* key, char* value, int& actual, int valueLength ) {
  BulkTreeReader reader( _file, _blockID*BulkBlock::dataSize() );
  return reader.get( key, value, actual, valueLength );
}

bool BulkTreeWriter::get( UINT32 key, char* value, int& actual, int valueLength ) {
  BulkTreeReader reader( _file, _blockID*BulkBlock::dataSize() );
  return reader.get( key, value, actual, valueLength );
}

void BulkTreeWriter::close() { 
  _flushAll();
  _write.flush();
  _file.close();
}

void BulkTreeWriter::flush() {
  _flushAll();
  _write.flush();
}

// ==============
// BulkTreeReader
// ==============

void BulkTreeReader::_fetch( BulkBlock& block, UINT32 id ) {
  _file->read( block.data(), id*BulkBlock::dataSize(), BulkBlock::dataSize() );
}

BulkTreeReader::BulkTreeReader( File& file ) :
  _file(&file),
  _ownFile(false)
{
}

BulkTreeReader::BulkTreeReader( File& file, UINT64 length ) :
  _file(&file),
  _ownFile(false),
  _fileLength(length)
{
}

BulkTreeReader::BulkTreeReader() :
  _ownFile(false),
  _file(0)
{
}

void BulkTreeReader::openRead( const std::string& filename ) {
  _file = new File;
  _file->open( filename );
  _fileLength = _file->size();
  _ownFile = true;
}

void BulkTreeReader::close() {
  if( _ownFile ) {
    _file->close();
    delete _file;
  }
}

bool BulkTreeReader::get( const char* key, int keyLength, char* value, int& actual, int valueLength ) {
  BulkBlock block;
  int rootID = int(_fileLength / BULK_BLOCK_SIZE) - 1;

  if( rootID < 0 )
    return false;

  int nextID = rootID;

  while( true ) {
    _fetch( block, nextID );

    if( block.leaf() )
      break;

    int actual;
    bool result = block.findGreater( key, keyLength, (char*) &nextID, actual, sizeof(nextID) );

    if( !result )
      return false;

    assert( actual == sizeof(nextID) );
  }

  // now we're at a leaf
  return block.find( key, keyLength, value, actual, valueLength );
}

bool BulkTreeReader::get( const char* key, char* value, int& actual, int valueLength ) {
  return get(key, strlen(key), value, actual, valueLength);
}

bool BulkTreeReader::get( UINT32 key, char* value, int& actual, int valueLength ) {
  return get( (const char*) &key, sizeof(key), value, actual, valueLength );
}
