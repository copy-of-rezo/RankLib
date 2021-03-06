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
// RMExpander
//
// 18 Aug 2004 -- dam
//

#ifndef INDRI_RMEXPANDER_HPP
#define INDRI_RMEXPANDER_HPP

#include <string>
#include <vector>
#include <map>

#include "indri/QueryExpander.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/Parameters.hpp"

class RMExpander : public QueryExpander  {
public:
  RMExpander( QueryEnvironment * env , Parameters& param );

  virtual std::string expand( std::string originalQuery , std::vector<ScoredExtentResult>& results );
};

#endif
