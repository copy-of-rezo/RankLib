/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// TagList
//
// March 2004 -- metzler
//

#include "indri/Tag.hpp"
#include <stdio.h>
#include <string.h>
#include <indri/greedy_vector>
#include "indri/TagExtent.hpp"
#include <iostream>
#include "indri/MetadataPair.hpp"
#include "indri/Buffer.hpp"

#ifndef _TAGLIST_HPP
#define _TAGLIST_HPP

class TagList {
private:
  struct tag_entry {
    const char* name;
    const char* conflation;
    int next;
    int begin;
    int end;
  };

  greedy_vector<tag_entry> _tags;
  int _openList;

public:
  TagList() {
    clear();
  }

  void clear() {
    _tags.clear();
    _openList = -1;
  }

  // we assume here that name is more or less immutable
  // so we can store a pointer to it.  This is a reasonable
  // assumption, because if the tag is indexed, its name is
  // in a hash table somewhere, and we can just point to that
  // name copy.
  void addTag(const char *name, const char* conflation, int begin) {
    // because of conflations, all kinds of messy stuff
    // happens if there's already an open tag with the same
    // conflation as this one.  Therefore, we have to go looking
    // for all open tags with this conflation; if there are any,
    // this tag doesn't get added.
    int list = _openList;

    while( list >= 0 ) {
      tag_entry& entry = _tags[list];

      if( !strcmp( entry.conflation, conflation ) ) {
        // we already have one of these
        return;
      }
   
      list = entry.next;
    }

    // all clear now to add the tag:
    tag_entry t;
    t.name = name;
    t.conflation = conflation;
    t.begin = begin;
    t.end = -1;
    t.next = _openList;
    _tags.push_back(t);
    _openList = _tags.size()-1;
  }

  void endTag(const char *name, const char* conflation, int end) {
    int list = _openList;
    int prev = -1;

    while( list >= 0 ) {
      tag_entry& entry = _tags[list];

      if( !strcmp( entry.name, name ) ) {
        // found a tag to close
        entry.end = end;
        int next = entry.next;

        // unlink from open list
        if( prev == -1 ) {
          _openList = next;
        } else {
          _tags[prev].next = next;
        }
        
        return;
      } else {
        // this wasn't the tag, so keep looking
        prev = list;
        list = entry.next;
      }
    }
  }

  void writeTagList( greedy_vector<TagExtent>& tags ) {
    // look through the tags vector; they're already in sorted order by open
    // position.  Only add closed tags.

    for( unsigned int i=0; i<_tags.size(); i++ ) {
      tag_entry& entry = _tags[i];

      if( entry.end > 0 ) {
        TagExtent extent;
        extent.begin = entry.begin;
        extent.end = entry.end;
        extent.name = entry.conflation;
        extent.number = 0;
        tags.push_back(extent);
      }
    }
  }

  // in this case, we'll treat the list of tags in this list
  // as if they were offsets into a metadata list
  void writeMetadataList( greedy_vector<MetadataPair>& pairs, Buffer& buffer, const char* docText ) {
    for( unsigned int i=0; i<_tags.size(); i++ ) {
      tag_entry& entry = _tags[i];

      if( entry.end > 0 ) {
        MetadataPair pair;
        
        // copy the text into a buffer
        int length = entry.end - entry.begin;
        char* spot = buffer.write(length+1);
        strncpy( spot, docText + entry.begin, length);
        spot[length] = 0;

        pair.key = entry.conflation;
        pair.value = spot;
        pair.valueLength = length+1;

        // docno is special -- its value must be stripped
        if( !strcmp( pair.key, "docno" ) ) {
          pair.stripValue();
        }

        pairs.push_back(pair);
      }
    }
  }

};

#endif
