
//
// Index
//
// 15 November 2004 -- tds
//

#ifndef INDRI_INDEX_HPP
#define INDRI_INDEX_HPP

#include <string>

#include "indri/DocListIterator.hpp"
#include "indri/DocExtentListIterator.hpp"
#include "indri/DocListFileIterator.hpp"
#include "indri/FieldListIterator.hpp"
#include "indri/VocabularyIterator.hpp"
#include "indri/TermList.hpp"
#include "indri/TermListFileIterator.hpp"

namespace indri {
  namespace index {
    class Index {
    public:
      /// Field data
      struct FieldDescription {
        /// name of the field
        std::string name;
        /// does the field contain numeric data
        bool numeric;
      };
      
      //
      // Counts
      //
      
      virtual int documentBase() = 0;

      virtual int term( const char* term ) = 0;
      virtual int term( const std::string& term ) = 0;
      virtual std::string term( int termID ) = 0;

      virtual int field( const char* fieldName ) = 0;
      virtual int field( const std::string& fieldName ) = 0;
      virtual std::string field( int fieldID ) = 0;

      virtual int documentLength( int documentID ) = 0;
      virtual UINT64 documentCount() = 0;
      virtual UINT64 documentCount( const std::string& term ) = 0;
      virtual UINT64 uniqueTermCount() = 0;

      virtual UINT64 termCount( const std::string& term ) = 0;
      virtual UINT64 termCount() = 0;

      virtual UINT64 fieldTermCount( const std::string& field ) = 0;
      virtual UINT64 fieldTermCount( const std::string& field, const std::string& term ) = 0;

      virtual UINT64 fieldDocumentCount( const std::string& field ) = 0;
      virtual UINT64 fieldDocumentCount( const std::string& field, const std::string& term ) = 0;

      //
      // Lists
      //
      
      virtual DocListIterator* docListIterator( int termID ) = 0;
      virtual DocListIterator* docListIterator( const std::string& term ) = 0;
      virtual DocListFileIterator* docListFileIterator() = 0;
      virtual DocExtentListIterator* fieldListIterator( int fieldID ) = 0;
      virtual DocExtentListIterator* fieldListIterator( const std::string& field ) = 0;
      virtual const TermList* termList( int documentID ) = 0;
      virtual TermListFileIterator* termListFileIterator() = 0;
      virtual VocabularyIterator* vocabularyIterator() = 0;
    };
  }
}

#endif // INDRI_INDEX_HPP

