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
// Parameters
//
// 30 April 2004 -- tds
//

#include "indri/Parameters.hpp"
#include "indri/XMLReader.hpp"
#include "indri/XMLNode.hpp"
#include "indri/XMLWriter.hpp"
#include <set>
#include <iostream>
#include <fstream>
#include "lemur/Exception.hpp"

static Parameters* _parametersSingleton = 0;

Parameters& Parameters::instance() {
  if( !_parametersSingleton )
    _parametersSingleton = new Parameters();
  return *_parametersSingleton;
}

void Parameters::_parseNextSegment( std::string& segment, int& arrayIndex, int& endOffset, const std::string& path, int beginOffset ) {
  endOffset = path.find( '.', beginOffset );
  arrayIndex = -1;

  if( endOffset < 0 )
    segment = path.substr( beginOffset );
  else
    segment = path.substr( beginOffset, endOffset );

  int openBracket = segment.find( '[' );
  int closeBracket = segment.find( ']' );

  if( openBracket >= 0 && closeBracket > openBracket ) {
    arrayIndex = 0;

    for( int i=openBracket+1; i<closeBracket; i++ ) {
      arrayIndex *= 10;
      arrayIndex += segment[i] - '0';
    }

    segment = segment.substr( 0, openBracket );
  }
}

Parameters::parameter_value* Parameters::_getSegment( const std::string& segment, int arrayIndex, Parameters::parameter_value* from ) {
  parameter_value* current = 0;

  // if we're at an array node, move to array[0]
  if( from->array.size() ) {
    from = from->array[0];
  }

  parameter_value::MValue::iterator iter = from->table.find(segment);

  if( iter != from->table.end() ) {
    current = (*iter).second;

    if( current->array.size() > 0 && arrayIndex >= 0 ) {
      if( unsigned(arrayIndex) < current->array.size() ) {
        current = current->array[arrayIndex];
      } else {
        current = 0;
      }
    }
  }

  return current;
}

Parameters::parameter_value* Parameters::_createPath( const std::string& path ) {
  int endOffset = 0;
  int arrayIndex = 0;
  std::string segment;
  parameter_value* root = _getRoot();
  parameter_value* last = root;
  parameter_value* next = last;

  for( int startOffset = 0; endOffset >= 0; startOffset = endOffset+1 ) {
    _parseNextSegment( segment, arrayIndex, endOffset, path, startOffset );
    next = _getSegment( segment, arrayIndex, last );

    if( !next ) {
      // we couldn't find a next path segment, so we'll have to create it
      parameter_value::MValue::iterator iter = last->table.find(segment);

      if( iter == last->table.end() ) {
        // the segment name wasn't in the table, so we'll make a new parameter_value for it
        parameter_value* newSegment = new parameter_value;
        last->table[segment] = newSegment;
        next = newSegment;
      } else {
        // found the segment name; the problem must have been with the arrayIndex
        assert( arrayIndex >= 0 );
        next = iter->second;
      }

      // if the 'next' node is an array already, or if we're trying to
      // use an array index with it, we're going to have to build something
      if( arrayIndex >= 0 || next->array.size() != 0 ) {
        // make sure it's an array already
        next->convertToArray();

        // make dummy array entries if there aren't any there already
        // this is for the case when the user wants param[5] but we don't have
        // param[4] yet, for instance.
        while( unsigned(arrayIndex) >= next->array.size() ) {
          next->array.push_back( new parameter_value );
        }

        // now we've got something set up for the next loop
        next = next->array[arrayIndex];
      }
    }

    last = next;
  }

  return last;
}

Parameters::parameter_value* Parameters::_getPath( const std::string& path, Parameters::parameter_value* last, int offset ) {
  int endOffset; 
  int arrayIndex = 0;
  std::string segment;

  _parseNextSegment( segment, arrayIndex, endOffset, path, offset );
  parameter_value* next = _getSegment( segment, arrayIndex, last );

  if( next ) {
    if( endOffset == std::string::npos ) {
      // we have a value, and we're at the end of the path, so return it
      return next;
    } else {
      // we have a value, but more path left, so keep going
      return _getPath( path, next, endOffset+1 );
    }
  } else {
    return 0;
  }
}

Parameters::parameter_value* Parameters::_getRoot() {
  assert( _collection );
  return _collection;
}

Parameters::Parameters() :
  _collection(new parameter_value),
  _owned(true)
{
}

Parameters::Parameters( const Parameters& other ) :
  _collection(other._collection),
  _owned(false)
{
}

Parameters::Parameters( parameter_value* value ) :
  _collection(value),
  _owned(false)
{
}

Parameters::~Parameters() {
  if( _owned )
    delete _collection;
}

Parameters Parameters::get( int index ) {
  if( ! exists(index) )
    LEMUR_THROW( LEMUR_IO_ERROR, "Required index didn't exist." );

  assert( exists( index ) );
  parameter_value* root = _getRoot();

  if( root->array.size() )
    return Parameters( root->array[index] );
  else
    return Parameters( root );
}

Parameters Parameters::get( const char* name ) {
  std::string conversion = name;
  return get(conversion);
}

Parameters Parameters::get( const std::string& name ) {
  assert( exists(name) );
  if( ! exists(name) )
    LEMUR_THROW( LEMUR_IO_ERROR, "Required parameter '" + name + "' was not specified." );

  parameter_value* root = _getRoot();
  parameter_value* current = _getPath(name, root);

  return Parameters( current );
}

bool Parameters::get( const std::string& name, bool def ) {
  if( !exists(name) )
    return def;
  else
    return bool(get(name));
}

int Parameters::get( const std::string& name, int def ) {
  if( !exists(name) )
    return def;
  else
    return int(get(name));
}

INT64 Parameters::get( const std::string& name, INT64 def ) {
  if( !exists(name) )
    return def;
  else
    return INT64(get(name));

}

double Parameters::get( const std::string& name, double def ) {
  if( !exists(name) )
    return def;
  else
    return double(get(name));
}

std::string Parameters::get( const std::string& name, const char* def ) {
  if( !exists(name) )
    return def;
  else
    return std::string(get(name));
}

std::string Parameters::get( const std::string& name, const std::string& def ) {
  if( !exists(name) )
    return def;
  else
    return std::string(get(name));
}

Parameters Parameters::operator[] ( const std::string& path ) {
  return get(path);
}

Parameters Parameters::operator[] ( int index ) {
  return get(index);
}

Parameters Parameters::operator[] ( const char* path ) {
  return get(path);
}

Parameters Parameters::append( const std::string& path ) {
  parameter_value* current = _createPath(path);
  parameter_value* slice = new parameter_value();

  if( !current->array.size() ) {
    current->convertToArray();
  }

  current->array.push_back( slice );
  return Parameters( slice );
}

void Parameters::set( const std::string& key, bool value ) {
  std::string strValue = value ? "true" : "false";
  set( key, strValue );
}

void Parameters::set( const std::string& key, UINT64 value ) {
  std::string v = i64_to_string(value);
  set( key, v );
}

void Parameters::set( const std::string& key, int value ) {
  std::stringstream s;
  s << value;
  set( key, s.str() );
}

void Parameters::set( const std::string& key, double value ) {
  std::stringstream s;
  s << value;
  set( key, s.str() );
}

void Parameters::set( const std::string& key, const char* value ) {
  parameter_value* path = _createPath( key );
  path->value = value;
}

void Parameters::set( const std::string& key, const std::string& value ) {
  parameter_value* path = _createPath( key );
  path->value = value;
}

void Parameters::set( const std::string& value ) {
  parameter_value* root = _getRoot();
  root->value = value;
}

void Parameters::remove( const std::string& path ) {
  parameter_value* root = _getRoot();
  int lastDot = path.rfind('.');
  std::string parentPath;
  std::string subpath = path;

  if( lastDot > 0 ) {
    parentPath = path.substr( 0, lastDot );
    subpath = path.substr( lastDot+1 );
  }

  parameter_value* parent = _getPath( parentPath, root );
  
  if( parent ) {
    // grab segment name and index
    std::string segment;
    int arrayIndex;
    int endOffset;

    _parseNextSegment( segment, arrayIndex, endOffset, subpath, 0 );
    parameter_value* child = parent->table[segment];

    if( child ) {
      if( child->array.size() <= 1 && arrayIndex == 0 ) {
        delete child;
        parent->table.erase( segment );
      } else if( child->array.size() > unsigned(arrayIndex) ) {
        delete child->array[arrayIndex];
        child->array.erase( child->array.begin() + arrayIndex );
      }
    }
  }
}

size_t Parameters::size() {
  parameter_value* root = _getRoot();

  if( ! root )
    return 0;

  if( root->array.size() )
    return root->array.size();

  if( root->value.length() )
    return 1;

  if( root->table.size() )
    return 1;

  return 0;
}

bool Parameters::exists( int index ) {
  return size() > unsigned(index);
}

bool Parameters::exists( const std::string& name ) {
  parameter_value* root = _getRoot();
  parameter_value* value = _getPath( name, root );

  return value ? true : false;
}

void Parameters::_loadXML( XMLNode* node ) {
  // this method should only be called on table nodes
  std::set<std::string> seen;
  std::set<std::string> arrays;
  std::set<std::string> appends;
  XMLNode* current = 0;

  // find out which ones are in the XML file, and which ones appear multiple times
  const std::vector<XMLNode*>& children = node->getChildren();

  for( size_t i=0; i<children.size(); i++ ) {
    XMLNode* child = children[i];
    std::string name = child->getName();

    if( seen.find(name) == seen.end() )
      seen.insert(name);
    else
      arrays.insert(name);

    if( child->getAttribute( "append" ) == "true" )
      appends.insert(name);
  }

  std::set<std::string>::iterator iter;
  
  // delete current items that don't need to be here
  for( iter = seen.begin(); iter != seen.end(); iter++ ) {
    if( appends.find(*iter) != appends.end() )
      continue;

    if( exists(*iter) && 
        get(*iter).size() > 1 ||
        arrays.find(*iter) != arrays.end() ) {
      remove(*iter);
      appends.insert(*iter);
    }
  }

  if( children.size() ) {
    for( size_t i=0; i<children.size(); i++ ) {
      XMLNode* child = children[i];
      std::string name = child->getName();

      if( appends.find(name) != appends.end() ) {
        append(name)._loadXML(child);
      } else {
        if( !exists(name) )
          set(name, "");
        get(name)._loadXML(child);
      }
    }
  } else {
    set(node->getValue());
  }
}

void Parameters::load( const std::string& text ) {
  XMLReader reader;

  try {
    std::auto_ptr<XMLNode> result( reader.read( text ) );
    _loadXML( result.get() );
  } catch( Exception& e ) {
    LEMUR_RETHROW( e, "Had trouble parsing parameter text" );
  }
}

void Parameters::loadFile( const std::string& filename ) {
  std::ifstream input;
  XMLReader reader;
  
  input.open( filename.c_str(), std::ifstream::in );

  if( input.rdstate() & std::ios::failbit )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open parameter file '" + filename + "' for reading." );

  input.seekg( 0, std::ios::end );
  size_t length = input.tellg();
  input.seekg( 0, std::ios::beg );
  std::auto_ptr<char> buffer( new char[length] );
  
  try {
    input.read( buffer.get(), length );
    std::auto_ptr<XMLNode> result( reader.read( buffer.get(), length ) );

    _loadXML( result.get() );
  } catch( Exception& e ) {
    LEMUR_RETHROW( e, "Had trouble parsing parameter file '" + filename + "'" );
  }

  input.close();
}

void Parameters::loadCommandLine( int argc, char** argv ) {
  Parameters current = *this;

  for( int i=1; i<argc; i++ ) {
    if( argv[i][0] == '-' ) {
      std::string keyValue = &argv[i][1];
      int equals = keyValue.find('=');
      std::string key = keyValue.substr( 0, equals );
      std::string value = keyValue.substr( equals+1 );

      current.append( key ).set( value );
    } else {
      std::string filename = argv[i];
      current.loadFile( filename );
    }
  }
}

void Parameters::_fillXML( XMLNode* node ) {
  parameter_value* root = _getRoot();
  std::map<std::string, parameter_value*>::iterator iter;

  if( root->table.size() == 0 ) {
    // this is a value node, so just fill in the value and return
    node->setValue( root->value );
  } else {
    // there is a subtree here, so we need to put that subtree into the node
    // we're going to loop over all the children and add them one at a time
    for( iter = root->table.begin(); iter != root->table.end(); iter++ ) {
      parameter_value* childparam = iter->second;
      assert( childparam );

      if( childparam->array.size() ) {
        // this child is actually an array
        // in XML form, we're just going to add lots of children
        // with the same name (iter->first)
        for( size_t i=0; i<childparam->array.size(); i++ ) {
          XMLNode* arrayChild = new XMLNode( iter->first );
          Parameters childSlice( childparam->array[i] );
          childSlice._fillXML( arrayChild );

          node->addChild( arrayChild );
        }
      } else {
        // this isn't an array
        XMLNode* simpleChild = new XMLNode( iter->first );
        Parameters childSlice( childparam );
        childSlice._fillXML( simpleChild );

        node->addChild( simpleChild );
      }
    }
  }
}

XMLNode* Parameters::toXML() {
  XMLNode* root = new XMLNode( "parameters" );
  _fillXML( root );
  return root;
}

void Parameters::write( std::string& text ) {
  std::auto_ptr<XMLNode> root( toXML() );
  XMLWriter writer( root.get() );
  writer.write( text );
}

void Parameters::writeFile( const std::string& filename ) {
  std::string text;
  write(text);

  std::ofstream out;
  out.open( filename.c_str(), std::ofstream::out );

  if( !out.good() )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file '" + filename + "' for writing." );

  out << text.c_str() << std::endl;
  out.close();
}

