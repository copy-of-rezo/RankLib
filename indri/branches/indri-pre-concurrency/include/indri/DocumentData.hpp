/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// KeyfileDocumentData
//
// 4 January 2004 -- tds
//

#ifndef INDRI_KEYFILEDOCUMENTDATA_HPP
#define INDRI_KEYFILEDOCUMENTDATA_HPP

#include "lemur/File.hpp"

namespace indri {
  namespace index {
    struct DocumentData {
      DocumentData() : offset(0), byteLength(0), indexedLength(0), uniqueTermCount(0) {}

      File::offset_type offset; // offset into the dt file where we'll find the KeyfileTermList 
      int byteLength;       // length in bytes of the KeyfileTermList
      int indexedLength;    // the length of the document without stopwords
      int uniqueTermCount;  // number of unique terms found in this document
    };
  }
}

#endif // INDRI_KEYFILEDOCUMENTDATA_HPP


