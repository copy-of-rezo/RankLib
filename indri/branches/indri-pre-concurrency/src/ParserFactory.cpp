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
// ParserFactory
//
// 6 August 2004 -- tds
//

#include "indri/ParserFactory.hpp"
#include "indri/HTMLParser.hpp"
#include "indri/TextParser.hpp"
#include "lemur/Exception.hpp"

#define PARSER_HTML ("HTML")
#define PARSER_TEXT ("Text")
#define PARSER_XML ("XML")

//
// Directions for adding your own parser:
//   1. Add a #define at the top of the file (see PARSER_HTML as an example) that gives a normalized
//        name to your parser.  This is the name that you'd want to show up in a user interface.
//   1. Modify preferredName() to return the normalized name for your parser.
///       The idea here is to admit as many names as possible (including possibly misspellings)
//        in parameter files, but still keeping a nice name around in case someone wants to
//        print something to the screen.
//   2. Modify get() to return a copy of your parser.
//        Use the parsers map to keep a copy of your parser around.  The ParserFactory destructor
//        will delete any parsers you create.
//

std::string ParserFactory::preferredName( const std::string& name ) {
  if( name[0] == 'h' || name[0] == 'H' ) {
    return PARSER_HTML;
  } else if( name[0] == 'x' || name[0] == 'X' || name[0] == 's' || name[0] == 'S' ) {
    // accept 'XML' or 'SGML' here
    return PARSER_XML;
  } else if( name[0] == 't' || name[0] == 'T' ) {
    return PARSER_TEXT;
  }

  return "";
}

indri::Parser* ParserFactory::get( const std::string& name ) {
  std::vector<std::string> empty;
  std::map<std::string,std::string> mempty;

  return get( name, empty, empty, empty, empty, mempty );
}

indri::Parser* ParserFactory::get( const std::string& name,
                                   const std::vector<std::string>& includeTags,
                                   const std::vector<std::string>& excludeTags,
                                   const std::vector<std::string>& indexTags,
                                   const std::vector<std::string>& metadataTags,
                                   const std::map<std::string, std::string>& conflations )
{
  indri::Parser* parser;
  std::string preferred = preferredName( name );

  if( preferred == PARSER_HTML ) {
    parser = new HTMLParser();
  } else if( preferred == PARSER_XML ) {
    parser = new TaggedTextParser();
  } else if( preferred == PARSER_TEXT ) {
    parser = new TextParser();
  } else {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, name + " is not a known parser." );
  }

  parser->setTags( includeTags, excludeTags, indexTags, metadataTags, conflations );
  return parser;
}
