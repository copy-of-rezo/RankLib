/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// DocumentIteratorFactory
//
// 5 August 2004 -- tds
//

#include "indri/DocumentIteratorFactory.hpp"

#include "indri/PDFDocumentExtractor.hpp"
#include "indri/TaggedDocumentIterator.hpp"
#include "indri/TextDocumentExtractor.hpp"

#ifdef WIN32
// put in indri or indri/win?
#include "indri/WordDocumentExtractor.hpp"
#include "indri/PowerPointDocumentExtractor.hpp"
#endif

#include <iostream>
#include <locale>
#include "lemur/Exception.hpp"

#define TYPE_TAGGED   ( "Tagged Document Collection" )
#define TYPE_PDF      ( "Adobe PDF" )
#define TYPE_WORD     ( "Microsoft Word" )
#define TYPE_PPT      ( "Microsoft PowerPoint" )
#define TYPE_TEXT     ( "Text" )


DocumentIterator* DocumentIteratorFactory::get( const std::string& type, const char* startDocTag, const char* endDocTag, const char* startMetadataTag ) {
  std::string preferred = preferredName( type );
  DocumentIterator* result = 0;

  if( preferred == TYPE_TAGGED ) {
    TaggedDocumentIterator* iter = new TaggedDocumentIterator();
    iter->setTags( startDocTag, endDocTag, startMetadataTag );
    result = iter;
  } else if( preferred == TYPE_PDF ) {
    result = new PDFDocumentExtractor();
  } else if( preferred == TYPE_TEXT ) {
    result = new TextDocumentExtractor();
  }
  
#ifdef WIN32
  else if( preferred == TYPE_WORD ) {
    result = new WordDocumentExtractor();
  } else if( preferred == TYPE_PPT ) {
    result = new PowerPointDocumentExtractor();
  }
#endif

  if( !result )
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, type + " is an unknown DocumentIterator type." );

  return result;
}

DocumentIterator* DocumentIteratorFactory::get( const std::string& type ) {
  return get(type, 0, 0, 0);
}

std::string DocumentIteratorFactory::preferredName( const std::string& type ) {
  if( type == "tagged" || type == TYPE_TAGGED ) {
    return TYPE_TAGGED;
  } else if( type == "pdf" || type == "adobe pdf" || type == TYPE_PDF ) {
    return TYPE_PDF;
  } else if( type == "doc" || type == "msword" || type == "word" || type == "microsoft word" || type == TYPE_WORD ) {
    return TYPE_WORD;
  } else if( type == "ppt" || type == "powerpoint" || type == "msppt" || type == "microsoft powerpoint" || type == TYPE_PPT ) {
    return TYPE_PPT;
  } else if( type == "txt" || type == "text" || type == TYPE_TEXT ) {
    return TYPE_TEXT;
  }

  return "";
}
