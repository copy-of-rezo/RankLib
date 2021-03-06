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
// ExtentInsideNode
//
// 28 July 2004 -- tds
//

#include "indri/ExtentInsideNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

indri::infnet::ExtentInsideNode::ExtentInsideNode( const std::string& name, ListIteratorNode* inner, ListIteratorNode* outer ) :
  _inner(inner),
  _outer(outer),
  _name(name)
{
}

void indri::infnet::ExtentInsideNode::prepare( int documentID ) {
  _extents.clear();

  if( !_inner || !_outer )
    return;

  const indri::utility::greedy_vector<indri::index::Extent>& inExtents = _inner->extents();
  const indri::utility::greedy_vector<indri::index::Extent>& outExtents = _outer->extents();

  indri::utility::greedy_vector<indri::index::Extent>::const_iterator innerIter = inExtents.begin();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator outerIter = outExtents.begin();

  while( innerIter != inExtents.end() && outerIter != outExtents.end() ) {
    if( outerIter->contains( *innerIter ) ) {
      _extents.push_back( *innerIter );
      innerIter++;
    } else if( outerIter->begin <= innerIter->begin ) {
      outerIter++;
    } else { 
      innerIter++;
    }
  }
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::ExtentInsideNode::extents() {
  return _extents;
}

int indri::infnet::ExtentInsideNode::nextCandidateDocument() {
  return lemur_compat::max( _inner->nextCandidateDocument(), _outer->nextCandidateDocument() );
}

const std::string& indri::infnet::ExtentInsideNode::getName() const {
  return _name;
}

void indri::infnet::ExtentInsideNode::annotate( class Annotator& annotator, int documentID, int begin, int end ) {
  annotator.addMatches( _extents, this, documentID, begin, end );
  
  for( unsigned int i=0; i<_extents.size(); i++ ) {
    _inner->annotate( annotator, documentID, _extents[i].begin, _extents[i].end );
    _outer->annotate( annotator, documentID, _extents[i].begin, _extents[i].end );
  }
}

void indri::infnet::ExtentInsideNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}


