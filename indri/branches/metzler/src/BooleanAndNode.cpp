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
// BooleanAndNode
//
// 16 November 2004 -- tds
// 

#include "indri/BooleanAndNode.hpp"
#include "indri/Annotator.hpp"

indri::infnet::BooleanAndNode::BooleanAndNode( const std::string& name, std::vector<indri::infnet::ListIteratorNode*>& children ) :
  _name(name),
  _lists(children)
{
}

void indri::infnet::BooleanAndNode::prepare( int documentID ) {
  _extents.clear();

  // check for and condition
  for( int i=0; i<_lists.size(); i++ ) {
    if( _lists[i]->extents().size() == 0 )
      return;
  }

  // if all here, make a null extent
  _extents.push_back( indri::index::Extent( 0, 1 ) ); // breaks match highlighting.
}

indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::BooleanAndNode::extents() {
  return _extents;
}

int indri::infnet::BooleanAndNode::nextCandidateDocument() {
  int document = _lists[0]->nextCandidateDocument();

  for( int i=1; i<_lists.size(); i++ ) {
    document = lemur_compat::max( document, _lists[i]->nextCandidateDocument() );
  }
    
  return document;
}

const std::string& indri::infnet::BooleanAndNode::getName() const {
  return _name;
}

void indri::infnet::BooleanAndNode::annotate( class indri::infnet::Annotator& annotator, int documentID, int begin, int end ) {
  annotator.addMatches( _extents, this, documentID, begin, end );
  /* Should have the same as ODN? Or just the inner loop like OrNode
  // _children are _lists in this context.
  for( size_t i=0; i<_extents.size(); i++ ) {
  for( size_t j=0; j<_children.size(); j++ ) {
  _children[j]->annotate( annotator, documentID, _extents[i].begin, _extents[i].end );
  }
  }

  */
}

void indri::infnet::BooleanAndNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

