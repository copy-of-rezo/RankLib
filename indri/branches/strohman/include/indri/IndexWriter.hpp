
//
// IndexWriter
//
// 26 November 2004 -- tds
//

#ifndef INDRI_INDEXWRITER_HPP
#define INDRI_INDEXWRITER_HPP

#include <vector>
#include <utility>
#include <queue>

#include "lemur/lemur-compat.hpp"
#include "indri/indri-platform.h"
#include "indri/greedy_vector"
#include "indri/TermData.hpp"
#include "lemur/Keyfile.hpp"
#include "indri/Index.hpp"
#include "indri/VocabularyMap.hpp"
#include "indri/DocListFileIterator.hpp"
#include "indri/File.hpp"
#include "indri/SequentialWriteBuffer.hpp"
#include "indri/CorpusStatistics.hpp"
#include "indri/FieldStatistics.hpp"
#include "indri/TermBitmap.hpp"
#include "indri/TermRecorder.hpp"
#include "indri/TermTranslator.hpp"
#include "indri/BulkTree.hpp"

struct WriterIndexContext {
  struct less {
  private:
    indri::index::DocListFileIterator::iterator_greater _iterator_greater;
  
  public:
    bool operator () ( const WriterIndexContext* const&  one, const WriterIndexContext* const& two ) const {
      return _iterator_greater( one->iterator, two->iterator );
    }
  };

  WriterIndexContext( indri::index::Index* _index ) {
    bitmap = new indri::index::TermBitmap;
    index = _index;
    wasInfrequentCount = 0;
    wasFrequentCount = 0;

    if( index->iteratorLock() )
      index->iteratorLock()->lock();
    
    iterator = index->docListFileIterator();
    iterator->startIteration();

    newlyFrequent = new indri::index::TermRecorder;
    oldFrequent = new indri::index::TermRecorder;
    oldInfrequent = new HashTable<int, int>;

    // DEBUG
    sequenceCount = 0;
  }

  ~WriterIndexContext() {
    delete iterator;

    if( index->iteratorLock() )
      index->iteratorLock()->unlock();

    delete oldFrequent;
    delete newlyFrequent;
    delete oldInfrequent;
    delete bitmap;
  }

  indri::index::DocListFileIterator* iterator;
  indri::index::TermBitmap* bitmap;
  indri::index::Index* index;

  int wasFrequentCount;
  int wasInfrequentCount;
  int sequenceCount;
  indri::index::TermRecorder* newlyFrequent;
  indri::index::TermRecorder* oldFrequent;
  HashTable<int, int>* oldInfrequent;
};

typedef std::priority_queue<WriterIndexContext*,
                            std::vector<WriterIndexContext*>,
                            WriterIndexContext::less> invertedlist_pqueue;

namespace indri {
  namespace index {
    class IndexWriter {
    private:
      struct disktermdata_count_greater {
        bool operator () ( const DiskTermData* one, const DiskTermData* two ) const {
          return one->termData->corpus.totalCount > two->termData->corpus.totalCount;
        }
      };

      struct disktermdata_alpha_less {
        bool operator () ( const DiskTermData* one, const DiskTermData* two ) const {
          return strcmp( one->termData->term, two->termData->term ) < 0;
        }
      };

      struct keyfile_pair {
        BulkTreeWriter* stringMap;
        BulkTreeWriter* idMap;
      };

      keyfile_pair _infrequentTerms;
      keyfile_pair _frequentTerms;
      File _frequentTermsData;

      File _documentStatistics;
      File _documentLengths;

      File _invertedFile;
      File _directFile;

      SequentialWriteBuffer* _invertedOutput;

      greedy_vector<indri::index::DiskTermData*> _topTerms;
      Buffer _termDataBuffer;

      int _isFrequentCount;
      int _documentBase;
      indri::index::CorpusStatistics _corpus;
      std::vector<indri::index::Index::FieldDescription> _fields;
      std::vector<indri::index::FieldStatistics> _fieldData;

      void _writeManifest( const std::string& path );
      void _writeSkip( SequentialWriteBuffer* buffer, int document, int length );
      void _writeBatch( SequentialWriteBuffer* buffer, int document, int length, Buffer& data );

      void _writeFieldLists( std::vector<indri::index::Index*>& indexes, const std::string& path );
      void _writeFieldList( const std::string& fileName, int fieldIndex, std::vector<indri::index::DocExtentListIterator*> iterators );

      void _pushInvertedLists( greedy_vector<WriterIndexContext*>& lists, invertedlist_pqueue& queue );
      void _fetchMatchingInvertedLists( greedy_vector<WriterIndexContext*>& lists, invertedlist_pqueue& queue );
      void _writeStatistics( greedy_vector<WriterIndexContext*>& lists, indri::index::TermData* termData, UINT64& startOffset );
      void _writeInvertedLists( std::vector<WriterIndexContext*>& contexts );

      void _storeIdEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData );
      void _storeStringEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData );

      void _storeTermEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData );
      void _storeFrequentTerms();
      void _addInvertedListData( greedy_vector<WriterIndexContext*>& lists, indri::index::TermData* termData, Buffer& listBuffer, UINT64& endOffset );
      void _storeMatchInformation( greedy_vector<WriterIndexContext*>& lists, int sequence, indri::index::TermData* termData, UINT64 startOffset, UINT64 endOffset );

      int _lookupTermID( BulkTreeWriter& keyfile, const char* term );

      void _buildIndexContexts( std::vector<WriterIndexContext*>& contexts, std::vector<indri::index::Index*>& indexes );
      
      void _writeDirectLists( std::vector<WriterIndexContext*>& contexts );
      void _writeDirectLists( WriterIndexContext* context,
                              SequentialWriteBuffer* directOutput,
                              SequentialWriteBuffer* lengthsOutput,
                              SequentialWriteBuffer* dataOutput );

      indri::index::TermTranslator* _buildTermTranslator( BulkTreeWriter& newInfrequentTerms,
                                                          BulkTreeWriter& newFrequentTerms,
                                                          indri::index::TermRecorder& oldFrequentTermsRecorder,
                                                          HashTable<int, int>* oldInfrequent,
                                                          indri::index::TermRecorder& newFrequentTermsRecorder,
                                                          indri::index::Index* index,
                                                          indri::index::TermBitmap* bitmap );

      enum {
        TOPDOCS_DOCUMENT_COUNT = 1000,
        FREQUENT_TERM_COUNT = 1000
      };

    public:
      IndexWriter();
      void write( indri::index::Index& index,
                  std::vector<indri::index::Index::FieldDescription>& fields,
                  const std::string& fileName );
      void write( std::vector<indri::index::Index*>& indexes,
                  std::vector<indri::index::Index::FieldDescription>& fields,
                  const std::string& fileName );
    };
  }
}

#endif // INDRI_INDEXWRITER_HPP




 

