
//
// DiskIndex
//
// 8 December 2004 -- tds
//

#include "indri/DiskIndex.hpp"
#include "lemur/Keyfile.hpp"
#include "indri/DiskDocListIterator.hpp"
#include "indri/DiskDocExtentListIterator.hpp"
#include "indri/DiskDocListFileIterator.hpp"
#include "indri/Path.hpp"
#include "indri/Parameters.hpp"
#include "indri/DiskDocumentDataIterator.hpp"
#include "indri/CombinedVocabularyIterator.hpp"

#include "indri/DiskFrequentVocabularyIterator.hpp"
#include "indri/DiskKeyfileVocabularyIterator.hpp"
#include "indri/DiskTermListFileIterator.hpp"

void indri::index::DiskIndex::_readManifest( const std::string& path ) {
  Parameters manifest;
  manifest.loadFile( path );

  Parameters corpus = manifest["corpus"];

  _corpusStatistics.totalDocuments = (int) corpus["total-documents"];
  _corpusStatistics.totalTerms = (INT64) corpus["total-terms"];
  _corpusStatistics.uniqueTerms = (int) corpus["unique-terms"];
  _infrequentTermBase = (int) corpus["frequent-terms"];
  _documentBase = (int) corpus["document-base"];

  if( manifest.exists("fields") ) {
    Parameters fields = manifest["fields"];

    if( fields.exists("field") ) {
      Parameters field = fields["field"];

      for( int i=0; i<fields.size(); i++ ) {
        bool numeric = field[i].get( "isNumeric", false );
        int documentCount = field[i].get("total-documents", 0 );
        INT64 totalCount = field[i].get("total-terms", INT64(0) );
        std::string name = field[i].get( "name", "" );

        _fieldData.push_back( FieldStatistics( name, numeric, totalCount, documentCount ) );
      }
    }
  }
}

//
// open
//

void indri::index::DiskIndex::open( const std::string& base, const std::string& relative ) {
  _path = relative;

  std::string path = Path::combine( base, relative );

  std::string frequentStringPath = Path::combine( path, "frequentString" );
  std::string infrequentStringPath = Path::combine( path, "infrequentString" );
  std::string frequentIDPath = Path::combine( path, "frequentID" );
  std::string infrequentIDPath = Path::combine( path, "infrequentID" );
  std::string frequentTermsDataPath = Path::combine( path, "frequentTerms" );
  std::string documentLengthsPath = Path::combine( path, "documentLengths" );
  std::string documentStatisticsPath = Path::combine( path, "documentStatistics" );
  std::string invertedFilePath = Path::combine( path, "invertedFile" );
  std::string directFilePath = Path::combine( path, "directFile" );
  std::string manifestPath = Path::combine( path, "manifest" );

  _readManifest( manifestPath );

  _frequentStringToTerm.openRead( frequentStringPath );
  _infrequentStringToTerm.openRead( infrequentStringPath );

  _frequentIdToTerm.openRead( frequentIDPath );
  _infrequentIdToTerm.openRead( infrequentIDPath );
  _frequentTermsData.openRead( frequentTermsDataPath );

  _documentLengths.openRead( documentLengthsPath );
  _documentStatistics.openRead( documentStatisticsPath );

  _invertedFile.openRead( invertedFilePath );
  _directFile.openRead( directFilePath );

  _lengthsBuffer.cache( 0, _documentLengths.size() );

  for( int field=1; field <= _fieldData.size(); field++ ) {
    std::stringstream fieldFilename;
    fieldFilename << "field" << field;
    std::string fieldPath = Path::combine( path, fieldFilename.str() );

    File* fieldFile = new File();
    fieldFile->openRead( fieldPath );

    _fieldFiles.push_back( fieldFile );
  }
}

//
// close
//

void indri::index::DiskIndex::close() {
  delete_vector_contents( _fieldFiles );

  _frequentStringToTerm.close();
  _infrequentStringToTerm.close();

  _frequentIdToTerm.close();
  _infrequentIdToTerm.close();

  _frequentTermsData.close();

  _documentLengths.close();
  _documentStatistics.close();

  _invertedFile.close();
  _directFile.close();
}

//
// _fetchTermData
//

indri::index::DiskTermData* indri::index::DiskIndex::_fetchTermData( int termID ) {
  char buffer[16*1024];
  int actual;

  bool result = _frequentIdToTerm.get( termID, buffer, actual, sizeof buffer );

  if( !result ) {
    result = _infrequentIdToTerm.get( termID - _infrequentTermBase, buffer, actual, sizeof buffer );

    if( !result )
      return 0;
  }
  assert( result );

  RVLDecompressStream stream( buffer, actual );
  return disktermdata_decompress( stream, _fieldData.size(), DiskTermData::WithString | DiskTermData::WithOffsets );
}

//
// _fetchTermData
//

indri::index::DiskTermData* indri::index::DiskIndex::_fetchTermData( const char* term ) {
  char buffer[16*1024];
  int actual;
  int adjust = 0;

  bool result = _frequentStringToTerm.get( term, buffer, actual, sizeof buffer );

  if( !result ) {
    result = _infrequentStringToTerm.get( term, buffer, actual, sizeof buffer );

    if( !result )
      return 0;

    adjust = _infrequentTermBase;
  }
  assert( result );
  RVLDecompressStream stream( buffer, actual );

  indri::index::DiskTermData* diskTermData = disktermdata_decompress( stream,
                                                                      _fieldData.size(),
                                                                      DiskTermData::WithTermID | DiskTermData::WithOffsets );
  diskTermData->termID += adjust;
  return diskTermData;
}

//
// path
//

const std::string& indri::index::DiskIndex::path() {
  return _path;
}

//
// documentBase
//

int indri::index::DiskIndex::documentBase() {
  return _documentBase;
}

//
// term
//

int indri::index::DiskIndex::term( const char* t ) {
  indri::index::DiskTermData* diskTermData = _fetchTermData( t );
  int termID = diskTermData->termID;
  ::disktermdata_delete( diskTermData );
  return termID;
}

//
// term
//

int indri::index::DiskIndex::term( const std::string& t ) {
  return term( t.c_str() );
}

//
// term
//

std::string indri::index::DiskIndex::term( int termID ) {
  std::string result;
  indri::index::DiskTermData* diskTermData = _fetchTermData( termID );

  if( diskTermData ) {
    result = diskTermData->termData->term;
    ::disktermdata_delete( diskTermData );
  }

  return result;
}

//
// documentLength
//

int indri::index::DiskIndex::documentLength( int documentID ) {
  int documentOffset = documentID - _documentBase;

  if( documentOffset < 0 || _corpusStatistics.totalDocuments <= documentOffset ) 
    return 0;

  int length;
  UINT64 offset = sizeof(UINT32) * documentOffset;

  size_t actual = _lengthsBuffer.read( &length, offset, sizeof(UINT32) );
  assert( actual == sizeof(UINT32) );
  assert( length >= 0 );
  return length;
}

//
// documentCount
//

UINT64 indri::index::DiskIndex::documentCount() {
  return _corpusStatistics.totalDocuments;
}

//
// documentCount
//

UINT64 indri::index::DiskIndex::documentCount( const std::string& term ) {
  indri::index::DiskTermData* diskTermData = _fetchTermData( term.c_str() );
  UINT64 count = 0;

  if( diskTermData ) {
    count = diskTermData->termData->corpus.documentCount;
    ::disktermdata_delete( diskTermData );
  }

  return count;
}

//
// termCount
//

UINT64 indri::index::DiskIndex::termCount() {
  return _corpusStatistics.totalTerms;
}

//
// uniqueTermCount
//

UINT64 indri::index::DiskIndex::uniqueTermCount() {
  return _corpusStatistics.uniqueTerms;
}

//
// field
//

std::string indri::index::DiskIndex::field( int fieldID ) {
  if( fieldID == 0 || fieldID > _fieldData.size() )
    return "";

  return _fieldData[fieldID-1].name;
}

//
// field
//

int indri::index::DiskIndex::field( const char* name ) {
  for( int i=0; i<_fieldData.size(); i++ ) {
    if( _fieldData[i].name == name )
      return i+1;
  }

  return 0;
}

//
// field
//

int indri::index::DiskIndex::field( const std::string& fieldName ) {
  return field( fieldName.c_str() );
}

//
// termCount
//

UINT64 indri::index::DiskIndex::termCount( const std::string& t ) {
  DiskTermData* diskTermData = _fetchTermData( t.c_str() );
  UINT64 count = 0;

  if( diskTermData ) { 
    count = diskTermData->termData->corpus.totalCount;
    ::disktermdata_delete( diskTermData );
  }

  return count;
}

//
// fieldTermCount
//

UINT64 indri::index::DiskIndex::fieldTermCount( const std::string& f, const std::string& t ) {
  DiskTermData* diskTermData = _fetchTermData( t.c_str() );
  int index = field( f );
  UINT64 count = 0;

  if( diskTermData && index ) {
    count = diskTermData->termData->fields[index-1].totalCount;
    ::disktermdata_delete( diskTermData );
  }

  return count;
}

//
// fieldTermCount
//

UINT64 indri::index::DiskIndex::fieldTermCount( const std::string& f ) {
  int index = field( f );
  UINT64 count = 0;

  if( index )
    count = _fieldData[index-1].totalCount;

  return count;
}

//
// fieldDocumentCount
//

UINT64 indri::index::DiskIndex::fieldDocumentCount( const std::string& f ) {
  int index = field( f );
  UINT64 count = 0;
  
  if( index )
    count = _fieldData[index-1].documentCount;
  return count;
}

//
// fieldDocumentCount
//

UINT64 indri::index::DiskIndex::fieldDocumentCount( const std::string& f, const std::string& t ) {
  DiskTermData* diskTermData = _fetchTermData( t.c_str() );
  int index = field( f );
  UINT64 count = 0;

  if( diskTermData && index ) {
    count = diskTermData->termData->fields[index-1].documentCount;
    ::disktermdata_delete( diskTermData );
  }

  return count;
}

//
// docListIterator
//

indri::index::DocListIterator* indri::index::DiskIndex::docListIterator( int termID ) {
  // find out where the iterator starts and ends
  DiskTermData* data = _fetchTermData( termID );

  // if no such term, quit
  if( !data )
    return 0;

  INT64 startOffset = data->startOffset;
  INT64 length = data->length;
  ::disktermdata_delete( data );

  // truncate the length argument at 1MB, use it to pick a size for the readbuffer
  length = lemur_compat::min<INT64>( length, 1024*1024 );

  return new DiskDocListIterator( new SequentialReadBuffer( _invertedFile, length ), startOffset, 0 );
}

//
// docListIterator 
//

indri::index::DocListIterator* indri::index::DiskIndex::docListIterator( const std::string& term ) {
  // find out where the iterator starts and ends
  DiskTermData* data = _fetchTermData( term.c_str() );

  // if no such term, quit
  if( !data )
    return 0;

  INT64 startOffset = data->startOffset;
  INT64 length = data->length;
  ::disktermdata_delete( data );

  // truncate the length argument at 1MB, use it to pick a size for the readbuffer
  length = lemur_compat::min<INT64>( length, 1024*1024 );

  return new DiskDocListIterator( new SequentialReadBuffer( _invertedFile, length ), startOffset, _fieldData.size() );
}

//
// docListFileIterator
//

indri::index::DocListFileIterator* indri::index::DiskIndex::docListFileIterator( ) {
  return new DiskDocListFileIterator( _invertedFile, _fieldData.size() );
}

//
// fieldListIterator
//

indri::index::DocExtentListIterator* indri::index::DiskIndex::fieldListIterator( int fieldID ) {
  if( fieldID == 0 || fieldID > _fieldData.size() ) {
    return 0;
  }

  File* fieldFile = _fieldFiles[fieldID-1];
  return new DiskDocExtentListIterator( new SequentialReadBuffer( *fieldFile ), 0 );
}

//
// fieldListIterator
//

indri::index::DocExtentListIterator* indri::index::DiskIndex::fieldListIterator( const std::string& fieldName ) {
  int fieldID = field( fieldName );
  
  if( fieldID == 0 )
    return 0;

  File* fieldFile = _fieldFiles[fieldID-1];
  return new DiskDocExtentListIterator( new SequentialReadBuffer( *fieldFile ), 0 );
}

//
// termListFileIterator
//

const indri::index::TermList* indri::index::DiskIndex::termList( int documentID ) {
  indri::index::DocumentData documentData;

  // read the appropriate offset information from the disk document statistics file
  _documentStatistics.read( &documentData, (documentID-1)*sizeof(DocumentData), sizeof(DocumentData) );
  
  TermList* termList = new TermList;
  char* buffer = new char[documentData.byteLength];

  _directFile.read( buffer, documentData.offset, documentData.byteLength );
  termList->read( buffer, documentData.byteLength );

  delete buffer;
  return termList;
}

//
// termListFileIterator
//

indri::index::TermListFileIterator* indri::index::DiskIndex::termListFileIterator() {
  return new indri::index::DiskTermListFileIterator( _directFile );
}

//
// vocabularyIterator
//

indri::index::VocabularyIterator* indri::index::DiskIndex::vocabularyIterator() {
  return new indri::index::CombinedVocabularyIterator( frequentVocabularyIterator(),
                                                       infrequentVocabularyIterator(),
                                                       _infrequentTermBase );
}

//
// frequentVocabularyIterator
//

indri::index::VocabularyIterator* indri::index::DiskIndex::frequentVocabularyIterator() {
  return new indri::index::DiskFrequentVocabularyIterator( _frequentTermsData, _fieldData.size() );
}

//
// infrequentVocabularyIterator
//

indri::index::VocabularyIterator* indri::index::DiskIndex::infrequentVocabularyIterator() {
  return new indri::index::DiskKeyfileVocabularyIterator( _infrequentTermBase, _infrequentIdToTerm, _lock, _fieldData.size() );
}

//
// documentDataIterator
//

indri::index::DocumentDataIterator* indri::index::DiskIndex::documentDataIterator() {
  return new indri::index::DiskDocumentDataIterator( _documentStatistics );
}

//
// iteratorLock
//

Lockable* indri::index::DiskIndex::iteratorLock() {
  return 0;
}

//
// statisticsLock
//

Lockable* indri::index::DiskIndex::statisticsLock() {
  return &_lock;
}

