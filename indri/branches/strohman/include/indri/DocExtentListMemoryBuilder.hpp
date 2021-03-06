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
// DocExtentListMemoryBuilder
//
// 24 November 2004 -- tds
//

#ifndef INDRI_DOCEXTENTLISTMEMORYBUILDER_HPP
#define INDRI_DOCEXTENTLISTMEMORYBUILDER_HPP

#include "indri/indri-platform.h"
#include "indri/greedy_vector"
#include "indri/DocExtentListIterator.hpp"
#include <utility>
#include "lemur/RVLCompress.hpp"

namespace indri {
  namespace index {
    struct DocExtentListMemoryBuilderSegment {
      DocExtentListMemoryBuilderSegment( char* b, char* d, char* c ) {
        base = b;
        data = d;
        capacity = c;
      }

      char* base;
      char* data;
      char* capacity;
    };

    class DocExtentListMemoryBuilderIterator : public DocExtentListIterator {
      const indri::utility::greedy_vector< DocExtentListMemoryBuilderSegment, 4 >* _lists;
      indri::utility::greedy_vector< DocExtentListMemoryBuilderSegment, 4 >::const_iterator _current;
      indri::index::DocExtentListIterator::DocumentExtentData _data;
      
      const char* _list;
      const char* _listEnd;
      bool _numeric;
      bool _finished;

    public:
      void reset( const indri::utility::greedy_vector< DocExtentListMemoryBuilderSegment, 4 >& lists, bool numeric );
      void reset( class DocExtentListMemoryBuilder& builder );

      DocExtentListMemoryBuilderIterator( const class DocExtentListMemoryBuilder& builder ); 
      
      void startIteration();
      bool finished() const;
      bool nextEntry( int documentID );
      bool nextEntry();
      indri::index::DocExtentListIterator::DocumentExtentData* currentEntry();
    };

    class DocExtentListMemoryBuilder {
    public:
      typedef DocExtentListMemoryBuilderIterator iterator;

      int _documentFrequency;
      int _extentFrequency;

      indri::utility::greedy_vector< DocExtentListMemoryBuilderSegment, 4 > _lists;

      char* _list;
      char* _listBegin;
      char* _listEnd;

      int _lastLocation;
      int _lastDocument;
      int _lastExtentFrequency;

      char* _documentPointer;
      char* _locationCountPointer;

      bool _numeric;

      inline size_t _compressedSize( int documentID, int begin, int end, INT64 number );
      inline void _safeAddLocation( int documentID, int begin, int end, INT64 number );
      void _growAddLocation( int documentID, int begin, int end, INT64 number, size_t newDataSize );
      void _grow();
      void _terminateDocument();

    public:
      DocExtentListMemoryBuilder( bool numeric );
      ~DocExtentListMemoryBuilder();

      void addLocation( int documentID, int begin, int end, INT64 number );
      void addLocation( int documentID, int begin, int end );

      void clear();
      bool empty();

      int documentFrequency() const;
      int extentFrequency() const;
      size_t memorySize() const;

      void flush();
      iterator* getIterator();
    };
  }
}

#endif // INDRI_DOCEXTENTLISTMEMORYBUILDER_HPP
