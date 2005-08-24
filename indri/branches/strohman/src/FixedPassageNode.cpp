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
// FixedPassageNode
//
// 23 February 2005 -- tds
//

#include "indri/FixedPassageNode.hpp"
#include "indri/Annotator.hpp"
#include "lemur/lemur-compat.hpp"

indri::infnet::FixedPassageNode::FixedPassageNode( const std::string& name, indri::infnet::BeliefNode* child, int windowSize, int increment ) :
  _name(name),
  _child(child),
  _windowSize(windowSize),
  _increment(increment)
{
}

int indri::infnet::FixedPassageNode::nextCandidateDocument() {
  return _child->nextCandidateDocument();
}

double indri::infnet::FixedPassageNode::maximumBackgroundScore() {
  return INDRI_TINY_SCORE;
}

double indri::infnet::FixedPassageNode::maximumScore() {
  return INDRI_HUGE_SCORE;
}

//
// score
//

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::FixedPassageNode::score( int documentID, int begin, int end, int documentLength ) {
  // we're going to run through the field list, etc.
  _scores.clear();

  // find out how to chop up [begin,end] appropriately into passages 
  indri::index::Extent e( begin, end );
  _buildSubextents( e );

  // loop through the subextents, scoring each one
  for( int i=0; i<_subextents.size(); i++ ) {
    int scoreBegin = _subextents[i].begin;
    int scoreEnd = _subextents[i].end;
  
    const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _child->score( documentID, scoreBegin, scoreEnd, documentLength );

    for( int i=0; i<childResults.size(); i++ ) {
      indri::api::ScoredExtentResult result( childResults[i].score, documentID, scoreBegin, scoreEnd );
      _scores.push_back( result );
    }
  }

  return _scores;
}

//
// annotate
//

void indri::infnet::FixedPassageNode::annotate( indri::infnet::Annotator& annotator, int documentID, int begin, int end ) {
  annotator.add(this, documentID, begin, end);

  // round down to find where the passage starts
  indri::index::Extent e( begin, end );
  _buildSubextents( e ); 

  for( int i=0; i<_subextents.size(); i++ ) {
    _child->annotate( annotator, documentID, _subextents[i].begin, _subextents[i].end );
  }
}

//
// hasMatch
//

bool indri::infnet::FixedPassageNode::hasMatch( int documentID ) {
  return _child->hasMatch( documentID );
}

//
// _addSubextents
//

void indri::infnet::FixedPassageNode::_addSubextents( const indri::index::Extent& extent ) {
  int beginPassage = (extent.begin / _increment) * _increment;
  int endPassage = beginPassage + _windowSize;

  while( beginPassage <= extent.end - _windowSize ) {
    int begin = lemur_compat::max( beginPassage, extent.begin );
    int end = lemur_compat::min( endPassage, extent.end );

    _subextents.push_back( indri::index::Extent( begin, end ) );

    beginPassage += _increment;
    endPassage = beginPassage + _windowSize;
  }

  // final passage; may overlap more than other passages
  if( beginPassage < extent.end ) {
    int begin = lemur_compat::max( extent.end - _windowSize, extent.begin );
    int end = extent.end; 
    _subextents.push_back( indri::index::Extent( begin, end ) );
  }
}

//
// _buildSubextents
//

void indri::infnet::FixedPassageNode::_buildSubextents( const indri::index::Extent& extent ) {
  _subextents.clear();
  _addSubextents( extent );
}

//
// _buildSubextents
//

void indri::infnet::FixedPassageNode::_buildSubextents( const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  _subextents.clear();

  for( size_t i=0; i<extents.size(); i++ ) {
    _addSubextents( extents[i] );
  }
}

//
// hasMatch
//

const indri::utility::greedy_vector<bool>& indri::infnet::FixedPassageNode::hasMatch( int documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  _matches.clear();
  _matches.resize( extents.size(), false );

  // to match, we split up each extent into its passage components, then check to see if there are sub-matches there.
  size_t i=0; 
  size_t j=0;

  _buildSubextents( extents );

  // now that we have subextents, ask the child for regions that have results
  const indri::utility::greedy_vector<bool>& childMatches = _child->hasMatch( documentID, _subextents );

  // walk the extents and subextents simultaneously
  i = 0;
  j = 0;

  while( i < childMatches.size() && j < extents.size() ) {
    if( _subextents[i].begin < extents[j].begin ) {
      i++;    
      continue;
    }

    if( _subextents[i].end > extents[j].end ) {
      j++;
      continue;
    }

    assert( _subextents[i].begin >= extents[j].begin );
    assert( _subextents[i].end <= extents[j].end );

    if( childMatches[i] ) {
      _matches[i] = true;
    }
  }

  return _matches;
}

const std::string& indri::infnet::FixedPassageNode::getName() const {
  return _name;
}

void indri::infnet::FixedPassageNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

