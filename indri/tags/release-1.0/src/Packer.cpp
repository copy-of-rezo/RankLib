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
// Packer
//
// 22 March 2004 -- tds
//

#include "indri/Packer.hpp"
#include "indri/QuerySpec.hpp"
#include <sstream>
#include "indri/XMLWriter.hpp"

namespace indri {
  namespace lang {
    Packer::Packer() {
      _packedNodes = new XMLNode( "query" );
    }

    Packer::node_element* Packer::_getElement( class indri::lang::Node* node ) {
      if( _elements.find( node ) == _elements.end() ) {
        _elements[node] = new node_element( node );
      }

      return _elements[node];
    }

    XMLNode* Packer::_getNodeReference( class indri::lang::Node* node, const ::std::string& name ) {
      if( !node )
        return 0;

      XMLNode* reference = new XMLNode( name, node->nodeName() );

      if( _getElement(node)->flushed == false ) {
        node->pack(*this);
      }

      return reference;
    }

    void Packer::pack( class indri::lang::Node* root ) {
      node_element* element = _getElement( root );
      element->xmlNode->addAttribute( "root", "true" );
      root->pack(*this);
    }

    void Packer::pack( std::vector<indri::lang::Node*>& roots ) {
      for( unsigned int i=0; i<roots.size(); i++ ) {
        pack( roots[i] );
      }
    }

    void Packer::before( class indri::lang::Node* someNode ) {
      node_element* element = _getElement(someNode);
      _stack.push( element );
    }

    void Packer::after( class indri::lang::Node* someNode ) {
      node_element* element = _stack.top();

      assert( element->languageNode == someNode );
      if( !element->flushed ) {
        element->xmlNode->addAttribute( "name", someNode->nodeName() );
        element->xmlNode->addAttribute( "type", someNode->typeName() );
        // we don't send the query text across the wire, because it might have
        // some characters in it that would mess up the XML serialization
        _packedNodes->addChild( element->xmlNode );
        element->flushed = true;
      }

      _stack.pop();
    }

    void Packer::put( const char* name, double value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      std::stringstream valueToString;
      valueToString << value;

      if( !element->flushed )
        element->xmlNode->addAttribute( name, valueToString.str() );
    }

    void Packer::put( const char* name, bool value ) {
      put( name, UINT64(value) );
    }

    void Packer::put( const char* name, int value ) {
      put( name, UINT64(value) );
    }

    void Packer::put( const char* name, unsigned int value ) {
      put( name, UINT64(value) );
    }

    void Packer::put( const char* name, UINT64 value ) {
      put( name, INT64(value) );
    }

    void Packer::put( const char* name, INT64 value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      std::stringstream valueToString;
      valueToString << value;

      if( !element->flushed )
        element->xmlNode->addAttribute( name, valueToString.str() );
    }

    void Packer::put( const char* name, const ::std::string& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed )
        element->xmlNode->addAttribute( name, value );
    }

    void Packer::put( const char* name, const ::std::vector<int>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) { 
        XMLNode* node = new XMLNode( name );

        for( unsigned int i=0; i<value.size(); i++ ) {
          ::std::stringstream intToString;
          intToString << value[i];

          node->addChild( new XMLNode( "int", intToString.str() ) );
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<double>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) { 
        XMLNode* node = new XMLNode( name );

        for( unsigned int i=0; i<value.size(); i++ ) {
          ::std::stringstream doubleToString;
          doubleToString << value[i];

          node->addChild( new XMLNode( "double", doubleToString.str() ) );
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<RawExtentNode*>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) {
        XMLNode* node = new XMLNode( name );

        for( unsigned int i=0; i<value.size(); i++ ) {
          XMLNode* child = _getNodeReference( value[i], "noderef" );
          node->addChild(child);
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<ScoredExtentNode*>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) {
        XMLNode* node = new XMLNode( name );

        for( unsigned int i=0; i<value.size(); i++ ) {
          XMLNode* child = _getNodeReference( value[i], "noderef" );
          node->addChild(child);
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, Node* value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) {
        XMLNode* node = _getNodeReference( value, name );
        if( node )
          element->xmlNode->addChild( node );
      }
    }

    std::string Packer::toString() {
      std::string output;
      XMLWriter writer( _packedNodes );
      writer.write( output );
      return output;
    }

    XMLNode* Packer::xml() {
      return _packedNodes;
    }
  }
}
