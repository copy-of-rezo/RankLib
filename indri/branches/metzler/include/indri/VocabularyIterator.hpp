
//
// VocabularyIterator
//
// 23 November 2004 -- tds
//

#ifndef INDRI_VOCABULARYITERATOR_HPP
#define INDRI_VOCABULARYITERATOR_HPP

#include "indri/TermData.hpp"
#include "indri/DiskTermData.hpp"

namespace indri {
  namespace index { 
    class VocabularyIterator {
    public:
      virtual ~VocabularyIterator() {};
      
      virtual void startIteration() = 0;
      virtual bool finished() = 0;
      virtual bool nextEntry() = 0;
      virtual DiskTermData* currentEntry() = 0;
    };
  }
}
    
#endif // INDRI_VOCABULARYITERATOR_HPP



