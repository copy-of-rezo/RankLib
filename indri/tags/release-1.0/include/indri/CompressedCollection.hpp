/*==========================================================================
  Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.

  Use of the Lemur Toolkit for Language Modeling and Information Retrieval
  is subject to the terms of the software license set forth in the LICENSE
  file included with this software, and also available at
  http://www.cs.cmu.edu/~lemur/license.html 
  as well as the conditions below.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  3. The names "Indri", "Center for Intelligent Information Retrieval", 
  "CIIR", and "University of Massachusetts" must not be used to
  endorse or promote products derived from this software without
  prior written permission. To obtain permission, contact
  indri-info@ciir.cs.umass.edu.

  4. Products derived from this software may not be called "Indri" nor 
  may "Indri" appear in their names without prior written permission of 
  the University of Massachusetts. To obtain permission, contact 
  indri-info@ciir.cs.umass.edu.

  THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY OF MASSACHUSETTS AND OTHER
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
  THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGE.
  ==========================================================================
*/


//
// CompressedCollection.hpp
//
// 12 May 2004 -- tds
//

#ifndef INDRI_COMPRESSEDCOLLECTION_HPP
#define INDRI_COMPRESSEDCOLLECTION_HPP

#include "indri/Collection.hpp"
#include "lemur/string-set.h"
#include <string>
#include "lemur/File.hpp"
#include "lemur/Keyfile.hpp"
#include "lemur/WriteBuffer.hpp"
#include "indri/Buffer.hpp"
#include "indri/HashTable.hpp"

class CompressedCollection : public Collection {
private:
  Keyfile _lookup;
  File _storage;
  WriteBuffer* _output;
  Buffer _positionsBuffer;
  struct z_stream_s* _stream;
  HashTable<const char*, Keyfile*> _metalookups;
  String_set* _strings;

  void _writePositions( ParsedDocument* document, int& keyLength, int& valueLength );
  void _writeMetadataItem( ParsedDocument* document, int i, int& keyLength, int& valueLength );
  void _writeText( ParsedDocument* document, int& keyLength, int& valueLength );

  void _readPositions( ParsedDocument* document, const void* positionData, int positionDataLength );

public:
  CompressedCollection();
  ~CompressedCollection();

  void create( const std::string& fileName );
  void create( const std::string& fileName, const std::vector<std::string>& indexedFields );
  void open( const std::string& fileName );
  void openRead( const std::string& fileName );
  void close();

  ParsedDocument* retrieve( int documentID );
  std::string retrieveMetadatum( int documentID, const std::string& attributeName );
  void addDocument( int documentID, ParsedDocument* document );
};

#endif // INDRI_COMPRESSEDCOLLECTION_HPP
