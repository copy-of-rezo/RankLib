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
// ListBeliefNode
//
// 6 July 2004 -- tds
//

#include "indri/ListBeliefNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

// computes the length of the scored context
int ListBeliefNode::_contextLength( int begin, int end ) {
  //
  // There are two possible contexts at work here.  Consider the query
  // #combine[sentence]( dog.(paragraph) )
  //
  // In this case, the context for scoring is text in a paragraph field, but
  // this text will be scored for every sentence.  The paragraph field will
  // be represented by <_context>, and the sentence to be scored
  // will be represented by the term offsets <begin> and <end>.
  //

  if( !_context )
    return end - begin;

  int contextLength = 0;
  const greedy_vector<Extent>& extents = _context->extents();

  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin > end )
      break;

    if( extents[i].end < begin )
      continue; 

    // make sure to restrict the extents here to match the scored region
    int extentBegin = lemur_compat::max( extents[i].begin, begin );
    int extentEnd = lemur_compat::min( extents[i].end, end );

    contextLength += extentEnd - extentBegin;
  }

  return contextLength;
}

int ListBeliefNode::_contextOccurrences( int begin, int end ) {
  const greedy_vector<Extent>& extents = _list.extents();
  int count = 0;
  int lastEnd = 0;

  // look for all occurrences within bounds and that don't overlap
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= begin &&
        extents[i].end <= end &&
        extents[i].begin >= lastEnd ) {
      count++;
      lastEnd = extents[i].end;
    }
  }

  return count;
}

int ListBeliefNode::_documentOccurrences() {
  assert( _raw ); // score() maintains this invariant
  const greedy_vector<Extent>& extents = _raw->extents();
  int count = 0;
  int lastEnd = 0;

  // look for all occurrences within bounds and that don't overlap
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= lastEnd ) {
      count++;
      lastEnd = extents[i].end;
    }
  }

  return count;
}

ListBeliefNode::ListBeliefNode( const std::string& name, ListIteratorNode& child, ListIteratorNode* context, ListIteratorNode* raw, TermScoreFunction& scoreFunction, double maximumBackgroundScore, double maximumScore )
  :
  _name(name),
  _scoreFunction(scoreFunction),
  _maximumScore(maximumScore),
  _maximumBackgroundScore(maximumBackgroundScore),
  _documentSmoothing(false),
  _context(context),
  _raw(raw),
  _list(child)
{
  _maximumScore = INDRI_HUGE_SCORE;
}

int ListBeliefNode::nextCandidateDocument() {
  return _list.nextCandidateDocument();
}

double ListBeliefNode::maximumBackgroundScore() {
  return _maximumBackgroundScore;
}

double ListBeliefNode::maximumScore() {
  return _maximumScore;
}

const greedy_vector<ScoredExtentResult>& ListBeliefNode::score( int documentID, int begin, int end, int documentLength ) {
  int contextSize = _contextLength( begin, end );
  int occurrences = _contextOccurrences( begin, end );
  int documentOccurrences = _raw ? _documentOccurrences() : occurrences;
  double score = 0;
  
  score = _scoreFunction.scoreOccurrence( occurrences, contextSize, documentOccurrences, documentLength );

  _scores.clear();
  _scores.push_back( ScoredExtentResult( score, documentID, begin, end ) );

  return _scores;
}

void ListBeliefNode::annotate( Annotator& annotator, int documentID, int begin, int end ) {
  const greedy_vector<Extent>& extents = _list.extents();
  int count = 0;

  // mark the begin and end points for this list
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= begin &&
        extents[i].end <= end ) {
      annotator.add( this, documentID, extents[i].begin, extents[i].end );
      _list.annotate( annotator, documentID, extents[i].begin, extents[i].end );
    }
  }
}

bool ListBeliefNode::hasMatch( int documentID ) {
  return _list.extents().size() > 0;
}

const std::string& ListBeliefNode::getName() const {
  return _name;
}


