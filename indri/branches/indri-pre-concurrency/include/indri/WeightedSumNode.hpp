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
// WeightedSumNode
//
// 31 March 2004 -- tds
//
// Implements the InQuery #wsum operator.
//
// Note that this class transforms the probabilities
// out of log space and back into log space, which
// could cause a loss of precision.
//

#ifndef INDRI_WEIGHTEDSUMNODE_HPP
#define INDRI_WEIGHTEDSUMNODE_HPP

#include "indri/BeliefNode.hpp"
#include <vector>
#include "indri/greedy_vector"
#include "indri/ScoredExtentResult.hpp"

class WeightedSumNode : public BeliefNode {
private:
  std::vector<BeliefNode*> _children;
  std::vector<double> _weights;
  greedy_vector<ScoredExtentResult> _scores;
  std::string _name;

public:
  WeightedSumNode( const std::string& name );

  int nextCandidateDocument();
  double maximumScore();
  double maximumBackgroundScore();

  const greedy_vector<ScoredExtentResult>& score( int documentID, int begin, int end, int documentLength );
  void annotate( class Annotator& annotator, int documentID, int begin, int end );
  bool hasMatch( int documentID );
  void addChild( double weight, BeliefNode* child );
  const std::string& getName() const;
};

#endif // INDRI_WEIGHTEDSUMNODE_HPP
