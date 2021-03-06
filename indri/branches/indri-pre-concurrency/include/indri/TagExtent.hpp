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
// TagExtent
//
// 4 February 2004 -- tds
//

#ifndef INDRI_TAGEXTENT_HPP
#define INDRI_TAGEXTENT_HPP

struct TagExtent {
  const char* name;
  unsigned int begin;
  unsigned int end;
  INT64 number;
};

#endif // INDRI_TAGEXTENT_HPP
