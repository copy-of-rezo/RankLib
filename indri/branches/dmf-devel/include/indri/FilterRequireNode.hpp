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
// FilterRequireNode
//
// 13 April 2004 -- tds
//

#ifndef INDRI_FILTERREQUIRENODE_HPP
#define INDRI_FILTERREQUIRENODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/ListIteratorNode.hpp"
#include "indri/Extent.hpp"

class FilterRequireNode : public BeliefNode {
private:
  ListIteratorNode* _filter;
  BeliefNode* _required;
  greedy_vector<ScoredExtentResult> _extents;
  std::string _name;

public:
  FilterRequireNode( const std::string& name, ListIteratorNode* filter, 
		     BeliefNode* required );

  double maximumBackgroundScore();
  double maximumScore();
  const greedy_vector<ScoredExtentResult>& score( int documentID, int start, int end, int documentLength );
  bool hasMatch( int documentID );
  int nextCandidateDocument();
  const std::string& getName() const;
  void annotate( class Annotator& annotator, int documentID, int begin, int end );
};

#endif // INDRI_FILTERREQUIRENODE_HPP
