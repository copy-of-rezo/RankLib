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
// NotNode
//
// 26 August 2004 -- tds
//

#ifndef INDRI_NOTNODE_HPP
#define INDRI_NOTNODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/greedy_vector"
#include <string>

class NotNode : public BeliefNode {
private:
  BeliefNode* _child;
  greedy_vector<ScoredExtentResult> _extents;
  greedy_vector<bool> _matches;
  std::string _name;

public:
  NotNode( const std::string& name, BeliefNode* child );

  double maximumBackgroundScore();
  double maximumScore();
  const greedy_vector<ScoredExtentResult>& score( int documentID, int start, int end, int documentLength );
  bool hasMatch( int documentID );
  const greedy_vector<bool>& hasMatch( int documentID, const greedy_vector<Extent>& matchExtents );

  void annotate( class Annotator& annotator, int documentID, int begin, int end );
  
  int nextCandidateDocument();
  void indexChanged( indri::index::Index& index );
  const std::string& getName() const; 
};

#endif // INDRI_NOTNODE_HPP
