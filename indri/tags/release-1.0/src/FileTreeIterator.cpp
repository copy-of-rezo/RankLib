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
// FileTreeIterator
//
// 26 May 2004 -- tds
//

#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include <iostream>

// static construction
FileTreeIterator FileTreeIterator::_end;

FileTreeIterator::FileTreeIterator() {
}

FileTreeIterator::FileTreeIterator( const std::string& path ) {
  _stack.push( new DirectoryIterator( path ) );

  while( Path::isDirectory( *(*_stack.top()) ) ) {
    _stack.push( new DirectoryIterator( **_stack.top() ) );
  }
}

FileTreeIterator::~FileTreeIterator() {
  while( _stack.size() ) {
    delete _stack.top();
    _stack.pop();
  }
}

void FileTreeIterator::_nextCandidate() {
  // go to the next file.  If the current directory is complete,
  // go up levels until we find a directory with stuff left in it
  while( _stack.size() ) {
    DirectoryIterator& top = (*_stack.top());
    top++;

    if( top == DirectoryIterator::end() ) {
      delete _stack.top();
      _stack.pop();
    } else {
      break;
    }
  }
}

void FileTreeIterator::_next() {
  _nextCandidate();

  // need to make sure we've found a file
  while( _stack.size() ) {
    DirectoryIterator& top = (*_stack.top());
    
    if( top == DirectoryIterator::end() ) {
      _nextCandidate();
      continue;
    } 

    if( Path::isFile( *top ) ) {
      // found a file, so we're done
      break;
    }

    // have to recurse
    DirectoryIterator* child = new DirectoryIterator( *top );
    _stack.push(child);
  }
}

void FileTreeIterator::operator ++ ( int ) {
  _next();
}

void FileTreeIterator::operator ++ () {
  _next();
}

const std::string& FileTreeIterator::operator* () {
  DirectoryIterator& top = (*_stack.top());
  return *top;
}

bool FileTreeIterator::operator== ( const FileTreeIterator& other ) const {
  return ( &other == &_end ) && ( _stack.size() == 0 );
}

bool FileTreeIterator::operator!= ( const FileTreeIterator& other ) const {
  return ! this->operator== ( other );
}

const FileTreeIterator& FileTreeIterator::end() {
  return _end;
}
