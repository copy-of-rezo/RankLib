/*==========================================================================
 * Copyright (c) 2009  University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// WARCDocumentIterator
//
// 03 Mar 2009 -- dmf
//

#include "indri/WARCDocumentIterator.hpp"
#include <iostream>
#include "lemur/Exception.hpp"
#include "indri/XMLNode.hpp"

indri::parse::WARCDocumentIterator::WARCDocumentIterator() {
  _in = 0;
  _metaBuffer.grow (512 * 1024);
  _recordID = 0;
  _contentLength = 0;
  _recordIDLength = 0;
  _contentLengthLength = 0;
  _warcMeta = "warc";
  _dochdr = "dochdr";
  _docnoString = "docno";
  _recordID = "WARC-Record-ID:";
  _recordIDLength = (int)strlen(_recordID);
  _contentLength = "Content-Length:";
  _contentLengthLength = (int)strlen(_contentLength);
  _targetURI = "WARC-Target-URI:";
  _targetURILength = strlen(_targetURI);
  _trecID = "WARC-TREC-ID:";
  _trecIDLength = strlen(_trecID);
}

indri::parse::WARCDocumentIterator::~WARCDocumentIterator() {
  close();
}

void indri::parse::WARCDocumentIterator::open( const std::string& filename ) {
  _fileName = filename;
  _in = fopen( filename.c_str(), "r" );
  if( !_in )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );

  // Consume the first WARC record (type warcinfo)
  // should verify the WARC-Type is warcinfo
  // if not, it's a partial file or broken... bleah.
  // read until the WARC-Record-ID line.
  // parse the urn:uri value for the file unique identifier (ugly)
  // read until the Content-Length line.
  // parse the length (atoi)
  // read until an empty line
  // read length bytes into record

  char* beginLine;
  size_t lineLength;  
  bool result;
  int contentLength = 0;
  _buffer.clear();
  do {
    result = _readLine( beginLine, lineLength );
    if (! result) break;
    if (!strncmp(_recordID, beginLine, _recordIDLength)) {
      // parse out the uuid
      // strip ' <urn:uuid:' from the front
      // strip '>\n' from the back.
      _warcUUID.assign(beginLine + _recordIDLength + 11, 
                       lineLength - (_recordIDLength + 12 + 2));
    } else if (!strncmp(_contentLength, beginLine, _contentLengthLength)) {
      // parse out the content length;
      contentLength = atoi(beginLine + _contentLengthLength) + 1;
    }
  } while (result && (strlen(beginLine) > 1)) ;
  
  if (result) {
    // consume (and discard) the record.
    fseek(_in, contentLength, SEEK_CUR);
  }
  // file pointer is now positioned to read the first response record.
}

void indri::parse::WARCDocumentIterator::close() {
  if( _in )
    fclose( _in );
  _in = 0;
}

bool indri::parse::WARCDocumentIterator::_readLine( char*& beginLine, 
                                                    size_t& lineLength ) {
  lineLength = 0;
  size_t actual;
  // make a buffer of a reasonable size so we're not always allocating
  if( _buffer.size() < 1024*1024 )
    _buffer.grow( 1024*1024 );
  // if we're running out of room, add 25MB
  if( (_buffer.size() -  _buffer.position()) < 512*1024 ) {
    _buffer.grow( _buffer.size() + 1024*1024*25 );
  }
  if( _buffer.position() ) {
    // get rid of null terminator from previous call
    _buffer.unwrite(1);
  }

  size_t readAmount = _buffer.size() - _buffer.position() - 2;

  // fetch next document line
  char* buffer = _buffer.write( readAmount );
  char* result = fgets( buffer, (int)readAmount, _in );
 
  if(!result) {
    return false;
  }

  actual = strlen(buffer);
  lineLength += actual; 
  _buffer.unwrite( readAmount - actual );

  // all finished reading
  *_buffer.write(1) = 0;
  beginLine = _buffer.front() + _buffer.position() - lineLength - 1;

  return true;
}

indri::parse::UnparsedDocument* indri::parse::WARCDocumentIterator::nextDocument() {
  _document.metadata.clear();
  _buffer.clear();
  _metaBuffer.clear();
  char* beginLine;
  size_t lineLength;  
  bool result;

  // Consume the WARC record, put into WARC metadata
  // read until the WARC-Record-ID line.
  // parse the urn:uri value for the record unique identifier (ugly)
  // combine with the file uuid to make a DOCNO
  // should verify the WARC-Type is response
  // if not, the record should be skipped (are there any?)
  // read the WARC-Target-URI to get the url for the front of the DOCHDR.

  // read until an empty line
  // read the DOCHDR metadata
  // read until the Content-Length line.
  // parse the length (atoi)
  // read until an empty line
  // read length bytes into content.
  // read two newlines
  std::string uuid;
  std::string trecDocno = "";
  int contentLength = 0;

  std::string uri;
  do {
    result = _readLine( beginLine, lineLength );
    if (! result) break;
    if (!strncmp(_trecID, beginLine, _trecIDLength)) {
        // have a trec id to use
        trecDocno.assign(beginLine + _trecIDLength + 1, 
                         lineLength - (_trecIDLength + 2));

      } else if (!strncmp(_recordID, beginLine, _recordIDLength)) {
      // parse out the uuid
      // strip ' <urn:uuid:' from the front
      // strip '>\n' from the back.
      uuid.assign(beginLine + _recordIDLength + 11, 
                  lineLength - (_recordIDLength + 12 + 2));
    } else if (! strncmp(_targetURI, beginLine, _targetURILength) ) {
      // save the url to put into the dochdr field for HTMLParser
      uri.assign(beginLine + _targetURILength + 1, 
                 lineLength - (_targetURILength + 1));
    }
  } while( result && (strlen(beginLine) > 1) );
  
  if( !result ) {
    // didn't find a record, so we're done
    return 0;
  }
  // copy whatever we've read so far into the _metaBuffer.
  memcpy( _metaBuffer.write(_buffer.position()), _buffer.front(), 
          _buffer.position() * sizeof(char));
  *_metaBuffer.write(1) = 0; // terminate it.

  // make a WARC metadata tag
  indri::parse::MetadataPair warcMetadata;
  warcMetadata.key = _warcMeta;
  warcMetadata.value = _metaBuffer.front();
  warcMetadata.valueLength = _metaBuffer.position();
  _document.metadata.push_back(warcMetadata);

  // add a tag for DOCNO
  // may want a different strategy for getting docnos.
  // these be ugly.
  if (trecDocno.size() > 0) 
    sprintf(_docno, "%s", trecDocno.c_str());
  else
    sprintf(_docno, "%s-%s", _warcUUID.c_str(), uuid.c_str());
  indri::parse::MetadataPair docnoMetadata;
  docnoMetadata.key = _docnoString;
  docnoMetadata.value = _docno;
  docnoMetadata.valueLength = strlen(_docno) + 1;
  _document.metadata.push_back(docnoMetadata);
  
  // now read the dochdr metadata.
  // make a DOCHDR metadata tag
  indri::parse::MetadataPair dochdrMetadata;
  dochdrMetadata.key = _dochdr;
  dochdrMetadata.value = _metaBuffer.front() + _metaBuffer.position();
  // put in the uri
  memcpy( _metaBuffer.write(uri.size()), uri.c_str(), 
          uri.size() * sizeof(char));
  dochdrMetadata.valueLength = uri.size();
  do {
    result = _readLine( beginLine, lineLength );
    if (! result) break;
    if (!strncmp(_contentLength, beginLine, _contentLengthLength)) {
      // parse out the content length;
      contentLength = atoi(beginLine + _contentLengthLength) + 1;
    }
    // copy to the metadata buffer
    memcpy( _metaBuffer.write(lineLength), beginLine, 
            lineLength * sizeof(char) );
    dochdrMetadata.valueLength += lineLength;
  } while( result && (strlen(beginLine) > 1) );
  
  if( !result ) {
    // didn't find a record, so we're done
    return 0;
  }
  *_metaBuffer.write(1) = 0; // terminate it.
  dochdrMetadata.valueLength += 1; // add the terminator
  _document.metadata.push_back(dochdrMetadata);

  // from now on, everything is text
  int startDocument = (int)_buffer.position() - 1;
  _buffer.unwrite(1);
  int numRead = fread(_buffer.write(contentLength), sizeof(char), 
                      contentLength, _in);
  if (numRead != contentLength) 
    {
      // bad things
      LEMUR_THROW(LEMUR_IO_ERROR, "Short read." );
    }
  // prune the second trailing newline that follows a record;
  fseek(_in, 1 , SEEK_CUR);
  // terminate the string
  *_buffer.write(1) = 0;

  _document.content = _buffer.front() + startDocument;
  _document.contentLength = contentLength;
  _document.text = _buffer.front();
  _document.textLength = _buffer.position();
  return &_document;
}
