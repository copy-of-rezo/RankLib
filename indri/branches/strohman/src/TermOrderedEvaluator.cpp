
//
// TermOrderedEvaluator
//
// 25 July 2006 -- tds
//

#include "indri/Index.hpp"
#include "indri/Walker.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/Repository.hpp"
#include "indri/TermOrderedEvaluator.hpp"
#include "indri/delete_range.hpp"
#include "indri/TermScoreFunction.hpp"
#include "indri/ContextSimpleCountAccumulator.hpp"
#include "indri/TermScoreFunctionFactory.hpp"
#include <vector>
#include <map>
#include <math.h>

class WSumTermScoreFunction : public indri::query::TermScoreFunction {
private:
  std::vector< std::pair<double, indri::query::TermScoreFunction*> > _functions;

public:
  void addFunction( double weight, TermScoreFunction* function ) {
    _functions.push_back( std::make_pair(weight, function) );
  }

  double scoreOccurrence( double occurrences, int contextLength ) {
    double result = 0;

    for( int i=0; i<_functions.size(); i++ ) {
      TermScoreFunction* f = _functions[i].second;
      double w = _functions[i].first;
      result += w * exp( f->scoreOccurrence( occurrences, contextLength ) );
    }

    return log(result);
  }

  double scoreOccurrence( double occurrences, int contextLength, double documentOccurrences, int documentLength ) {
    double result = 0;

    for( int i=0; i<_functions.size(); i++ ) {
      TermScoreFunction* f = _functions[i].second;
      double w = _functions[i].first;
      result += w * exp(f->scoreOccurrence( occurrences, contextLength, documentOccurrences, documentLength ));
    }

    return log(result);
  }
};

class WeightTermScoreFunction : public indri::query::TermScoreFunction {
private:
  std::vector< std::pair<double, indri::query::TermScoreFunction*> > _functions;

public:
  void addFunction( double weight, TermScoreFunction* function ) {
    _functions.push_back( std::make_pair(weight, function) );
  }

  double scoreOccurrence( double occurrences, int contextLength ) {
    double result = 0;

    for( int i=0; i<_functions.size(); i++ ) {
      TermScoreFunction* f = _functions[i].second;
      double w = _functions[i].first;
      result += w * f->scoreOccurrence( occurrences, contextLength );
    }

    return result;
  }

  double scoreOccurrence( double occurrences, int contextLength, double documentOccurrences, int documentLength ) {
    double result = 0;

    for( int i=0; i<_functions.size(); i++ ) {
      TermScoreFunction* f = _functions[i].second;
      double w = _functions[i].first;
      result += w * f->scoreOccurrence( occurrences, contextLength, documentOccurrences, documentLength );
    }

    return result;
  }
};

indri::query::TermScoreFunction* indri::infnet::TermOrderedEvaluator::_buildTermScoreFunction( const std::string& smoothing, double occurrences, double contextSize ) const {
  double collectionFrequency;

  if( occurrences ) {
    collectionFrequency = double(occurrences) / double(contextSize);
  } else {
    // this is something that never happens in our collection, so we assume that it
    // happens somewhat less often than 1./collectionSize.  I picked 1/(2*collectionSize)
    // because it seemed most appropriate
    collectionFrequency = 1.0 / double(contextSize*2.);
  }
  
  indri::query::TermScoreFunction* function = indri::query::TermScoreFunctionFactory::get( smoothing, collectionFrequency );

  if( function == 0 )
    return 0;

  return function;
}

indri::infnet::TermOrderedEvaluator::~TermOrderedEvaluator() {
  indri::utility::delete_map_contents( _scoreMap );
  indri::utility::delete_map_contents( _listMap );
}

indri::infnet::TermOrderedEvaluator::TermOrderedEvaluator( indri::collection::Repository& repository )
  : 
  _repository(repository)
{
}

indri::infnet::InferenceNetwork::MAllResults indri::infnet::TermOrderedEvaluator::getResults() {
  return _results;
}

void indri::infnet::TermOrderedEvaluator::defaultAfter( indri::lang::Node* node ) {
  assert( "bad news" && 0 );
}

void indri::infnet::TermOrderedEvaluator::after( indri::lang::IndexTerm* term ) {
  if( _listMap.find( term ) == _listMap.end() ) {
    bool stopword = false;
    std::string processed = term->getText();
    InvertedList* list = new InvertedList;
    
    // stem and stop the word
    if( term->getStemmed() == false ) {
      processed = _repository.processTerm( term->getText() );
      stopword = processed.length() == 0;
    }
    
    // if it isn't a stopword, we can try to get it from the index
    if( !stopword ) {
      indri::collection::Repository::index_state state = _repository.indexes();
      
      for( int i=0; i<state->size(); i++ ) {
        indri::index::Index* index = (*state)[i];
        int termID = index->term( processed );
        
        if( termID == 0 )
          continue;
        
        indri::index::DocListIterator* iterator = index->docListIterator( termID );
        iterator->startIteration();
        indri::index::DocListIterator::DocumentData* data;
        
        for( ; !iterator->finished(); iterator->nextEntry() ) {
          data = iterator->currentEntry();
          list->documents.push_back(data->document);
          
          for( int i=0; i<data->positions.size(); i++ ) {
            int pos = data->positions[i];
            list->positions.push_back( Extent(pos, pos+1) );
          }
          list->counts.push_back(data->positions.size());
        }
        
        delete iterator;
      }
    }
    
    _listMap[term] = list;
  }
}

void indri::infnet::TermOrderedEvaluator::after( indri::lang::Field* field ) {
  if( _listMap.find( field ) == _listMap.end() ) {
    InvertedList* list = new InvertedList;
    indri::collection::Repository::index_state state = _repository.indexes();
    
    for( int i=0; i<state->size(); i++ ) {
      indri::index::Index* index = (*state)[i];
      int fieldID = index->field( field->getFieldName() );
      
      if( fieldID == 0 )
        continue;
      
      indri::index::DocExtentListIterator* iterator = index->fieldListIterator( fieldID );
      iterator->startIteration();
      indri::index::DocExtentListIterator::DocumentExtentData* data;
      
      for( ; !iterator->finished(); iterator->nextEntry() ) {
        data = iterator->currentEntry();
        list->documents.push_back(data->document);
        
        for( int i=0; i<data->extents.size(); i++ ) {
          list->positions.push_back( Extent(data->extents[i].begin, data->extents[i].end) );
        }
        list->counts.push_back(data->extents.size());
      }
    }
    
    _listMap[field] = list;
  }
} 

void indri::infnet::TermOrderedEvaluator::after( indri::lang::RawScorerNode* rawScorerNode ) {
  if( _scoreMap.find( rawScorerNode ) == _scoreMap.end() ) {
    InvertedList* raw = _listMap[rawScorerNode->getRawExtent()];
    InvertedList* context = _listMap[rawScorerNode->getContext()];
    
    indri::query::TermScoreFunction* function = 0;

    function = _buildTermScoreFunction( rawScorerNode->getSmoothing(),
                                        rawScorerNode->getOccurrences(),
                                        rawScorerNode->getContextSize() );

    indri::collection::Repository::index_state state = _repository.indexes();
    indri::index::Index* index = (*state)[0];
    int iIndex = 0;
    int indexEnd = index->documentBase() + index->documentCount();
    ScoredList* list = new ScoredList;
    list->function = function;
    std::vector<indri::api::ScoredExtentResult>* scores = &list->results;
    
    // BUGBUG: ignore context for now
    for( int i=0; i<raw->counts.size(); i++ ) {
      int document = raw->documents[i];
      int count = raw->counts[i];
      
      while( document > indexEnd ) {
        iIndex++;
        index = (*state)[iIndex];
      }
      int length = index->documentLength( document );
      
      double score = function->scoreOccurrence( count, length );
      scores->push_back( indri::api::ScoredExtentResult( score, document, 0, length ) );
    }
    
    _scoreMap[rawScorerNode] = list;
  }
}

//
// ContextCounterNode
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::ContextCounterNode* contextCounter ) {
  InvertedList* raw = _listMap[ contextCounter->getRawExtent() ];
  InvertedList* context = _listMap[ contextCounter->getContext() ];

  UINT64 rawCount = 0;
  UINT64 contextCount = 0;

  if( !context ) {
    for( int i=0; i<raw->counts.size(); i++ ) {
      rawCount += raw->counts[i];
    }
  } else {
    // first, count the size of the context list
    for( int i=0; i<context->positions.size(); i++ ) {
      contextCount += (context->positions[i].end - context->positions[i].begin);
    }

    int j = 0;
    int countPos = 0;
    int contextDocPos = 0;
    // then, count the occurrences that are within the context list
    for( int i=0; i<raw->documents.size(); i++ ) {
      int document = raw->documents[i];

      // advance the context up until this point
      while( j < context->documents.size() && context->documents[j] < document ) {
        contextDocPos += context->counts[j];
        j++;
      }

      if( j > context->documents.size() )
        break;

      int startContextPos = contextDocPos;
      int endContextPos = contextDocPos + context->counts[j];

      int startDocumentPos = countPos;
      int endDocumentPos = countPos + raw->counts[i];

      int ex = 0;
      int lastEnd = 0;

      for( int k = startDocumentPos; k < endDocumentPos; k++ ) {
        // move through the contexts until we find one that might work
        while( context->positions[ex].end < raw->positions[k].begin ||
               (context->positions[ex].end == raw->positions[k].begin &&
               raw->positions[k].end > raw->positions[k].begin ) ) {
          ex++;
          if( ex >= endContextPos ) break;
        }

        if( ex < endContextPos &&
          raw->positions[k].begin >= context->positions[ex].begin &&
          raw->positions[k].end <= context->positions[ex].end &&
          raw->positions[k].begin >= lastEnd ) {
            rawCount++;
            lastEnd = raw->positions[k].end;
        }
      }

      countPos += raw->counts[i];
    }
  }

  _results[contextCounter->nodeName()][ "occurrences" ].push_back( indri::api::ScoredExtentResult( rawCount, 0 ) );
  _results[contextCounter->nodeName()][ "contextSize" ].push_back( indri::api::ScoredExtentResult( contextCount, 0 ) );
}

//
// ContextSimpleCounterNode
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::ContextSimpleCounterNode* contextSimpleCounterNode ) {
    ContextSimpleCountAccumulator* contextCount = 0;

    contextCount = new ContextSimpleCountAccumulator( contextSimpleCounterNode->nodeName(),
                                                      contextSimpleCounterNode->terms(),
                                                      contextSimpleCounterNode->field(),
                                                      contextSimpleCounterNode->context() );
                                                      
    indri::collection::Repository::index_state state = _repository.indexes();
    
    for( int i=0; i<state->size(); i++ ) {
      indri::index::Index* index = (*state)[i];
      contextCount->indexChanged(*index);
    }
    
    _results[contextCount->getName()] = contextCount->getResults();
    delete contextCount;
}

//
// ScoreAccumulatorNode
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::ScoreAccumulatorNode* accumulator ) {
  if( _scoreMap.find(accumulator) == _scoreMap.end() ) {
    ScoredList* list = _scoreMap[accumulator->getChild()];
    std::sort( list->results.begin(), list->results.end() );
    _results[accumulator->nodeName()]["scores"] = list->results;  
  }
}

//
// CombineNode
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::CombineNode* combine ) {
  if( _scoreMap.find(combine) == _scoreMap.end() ) {
      std::vector<ScoredList*> lists;
      std::vector<double> weights;
      WeightTermScoreFunction* function = new WeightTermScoreFunction();
      
      for( int i=0; i<combine->getChildren().size(); i++ ) {
        double normalizedWeight = 1.0 / combine->getChildren().size();
        ScoredList* list = _scoreMap[combine->getChildren()[i]];
        lists.push_back( list );
        weights.push_back( normalizedWeight );
        function->addFunction( normalizedWeight, list->function );
      }
      
      ScoredList* result = new ScoredList;
      result->function = 0;

      indri::collection::Repository::index_state state = _repository.indexes();
      std::vector<int> indexes;
      int length;
      
      for( int j=0; j<lists.size(); j++ ) {
        indexes.push_back(0);
      }

      while(true) {
        int minDocument = MAX_INT32;
      
        for( int i=0; i<lists.size(); i++ ) {
          // minDocument
          if( indexes[i] > -1 && lists[i]->results[indexes[i]].document < minDocument ) {
            minDocument = lists[i]->results[indexes[i]].document;
            length = lists[i]->results[indexes[i]].end - lists[i]->results[indexes[i]].begin;
          }
        }
        
        if (minDocument == MAX_INT32)
          break;
        
        double score = 0;
        
        for( int i=0; i<lists.size(); i++ ) {
          if( indexes[i] > -1 && lists[i]->results[indexes[i]].document == minDocument ) {
            score += lists[i]->results[indexes[i]].score * weights[i];
            indexes[i]++;
            if( indexes[i] >= lists[i]->results.size() )
              indexes[i] = -1;
          } else {
            score += lists[i]->function->scoreOccurrence( 0, length );
          }
        }
        
        result->results.push_back( indri::api::ScoredExtentResult( score, minDocument, 0, length ) );
      }
      
      result->function = function;
      _scoreMap[combine] = result;
  }
}


//
// WeightNode
//


void indri::infnet::TermOrderedEvaluator::after( indri::lang::WeightNode* weight ) {
  if( _scoreMap.find(weight) == _scoreMap.end() ) {
      std::vector<ScoredList*> lists;
      std::vector<double> weights;
      
      double total = 0;
      
      for( int i=0; i<weight->getChildren().size(); i++ ) {
        total += weight->getChildren()[i].first;
      }
      
      WeightTermScoreFunction* function = new WeightTermScoreFunction;

      for( int i=0; i<weight->getChildren().size(); i++ ) {
        double normalizedWeight = weight->getChildren()[i].first / total;
        ScoredList* list = _scoreMap[weight->getChildren()[i].second];
        lists.push_back( list );
        weights.push_back( normalizedWeight );
        function->addFunction( normalizedWeight, list->function );
      }
      
      ScoredList* result = new ScoredList;
      result->function = 0;

      indri::collection::Repository::index_state state = _repository.indexes();
      std::vector<int> indexes;
      int length;
      
      for( int j=0; j<lists.size(); j++ ) {
        if( lists[j]->results.size() == 0 )
          indexes.push_back(-1);
        else
          indexes.push_back(0);
      }

      while(true) {
        int minDocument = MAX_INT32;
      
        for( int i=0; i<lists.size(); i++ ) {
          // minDocument
          if( indexes[i] > -1 && lists[i]->results[indexes[i]].document < minDocument ) {
            minDocument = lists[i]->results[indexes[i]].document;
            length = lists[i]->results[indexes[i]].end - lists[i]->results[indexes[i]].begin;
          }
        }
        
        if (minDocument == MAX_INT32)
          break;
        
        double score = 0;
        
        for( int i=0; i<lists.size(); i++ ) {
          if( indexes[i] > -1 && lists[i]->results[indexes[i]].document == minDocument ) {
            score += lists[i]->results[indexes[i]].score * weights[i];
            indexes[i]++;
            if( indexes[i] >= lists[i]->results.size() )
              indexes[i] = -1;
          } else {
            score += lists[i]->function->scoreOccurrence( 0, length );
          }
        }
        
        result->results.push_back( indri::api::ScoredExtentResult( score, minDocument, 0, length ) );
      }
      
      result->function = function;
      _scoreMap[weight] = result;
  }
}

//
// WSumNode
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::WSumNode* weight ) {
  if( _scoreMap.find(weight) == _scoreMap.end() ) {
      std::vector<ScoredList*> lists;
      std::vector<double> weights;
      
      double total = 0;
      WSumTermScoreFunction* function = new WSumTermScoreFunction();
      
      for( int i=0; i<weight->getChildren().size(); i++ ) {
        total += weight->getChildren()[i].first;
      }
      
      for( int i=0; i<weight->getChildren().size(); i++ ) {
        double normalizedWeight = weight->getChildren()[i].first / total;
        ScoredList* list = _scoreMap[weight->getChildren()[i].second];
        lists.push_back( list );
        weights.push_back( normalizedWeight );
        function->addFunction( normalizedWeight, list->function );
      }
      
      ScoredList* result = new ScoredList;
      result->function = 0;

      indri::collection::Repository::index_state state = _repository.indexes();
      std::vector<int> indexes;
      int length;
      
      for( int j=0; j<lists.size(); j++ ) {
        if (lists[j]->results.size() > 0)
          indexes.push_back(0);
        else
          indexes.push_back(-1);
      }

      while(true) {
        int minDocument = MAX_INT32;
      
        for( int i=0; i<lists.size(); i++ ) {
          // minDocument
          if( indexes[i] > -1 && lists[i]->results[indexes[i]].document < minDocument ) {
            minDocument = lists[i]->results[indexes[i]].document;
            length = lists[i]->results[indexes[i]].end - lists[i]->results[indexes[i]].begin;
          }
        }
        
        if (minDocument == MAX_INT32)
          break;
        
        double score = 0;
        
        for( int i=0; i<lists.size(); i++ ) {
          if( indexes[i] > -1 && lists[i]->results[indexes[i]].document == minDocument ) {
            score += exp( lists[i]->results[indexes[i]].score ) * weights[i];
            indexes[i]++;
            if( indexes[i] >= lists[i]->results.size() )
              indexes[i] = -1;
          } else {
            score += exp( lists[i]->function->scoreOccurrence( 0, length ));
          }
        }
        
        score = log( score );
        result->results.push_back( indri::api::ScoredExtentResult( score, minDocument, 0, length ) );
      }
      
      result->function = function;
      _scoreMap[weight] = result;
  }
}

//
// _listMaxDocument
//

 int indri::infnet::TermOrderedEvaluator::_listMaxDocument( std::vector<TermOrderedEvaluator::InvertedList*>& lists, std::vector<int>& indexes ) {
  int maxDocument = 0;
  
  for( int i=0; i<lists.size(); i++ ) {
    if( indexes[i] < 0 )
      return MAX_INT32;
      
    if( lists[i]->documents[indexes[i]] > maxDocument )
      maxDocument = lists[i]->documents[indexes[i]];
  }
  
  return maxDocument;
}

//
// _allSameDocument
//

bool indri::infnet::TermOrderedEvaluator::_allSameDocument( std::vector<TermOrderedEvaluator::InvertedList*>& lists, std::vector<int>& indexes ) {
  int firstDoc = lists[0]->documents[indexes[0]];
  
  for( int i=1; i<lists.size(); i++ ) {
    if( lists[i]->documents[indexes[i]] != firstDoc )
      return false;
  }

  return true;
}

//
// _advanceToDocument
//

void indri::infnet::TermOrderedEvaluator::_advanceToDocument( const std::vector<TermOrderedEvaluator::InvertedList*>& lists, std::vector<int>& dind, std::vector<int>& cind, int doc ) {
  for( int i=0; i<lists.size(); i++ ) {
    if( lists[i]->documents[dind[i]] < doc ) {
      cind[i] += lists[i]->counts[dind[i]];
      dind[i]++;
      if( dind[i] >= lists[i]->documents.size() )
        dind[i] = -1;
    }
  }
}

//
// UWNode
//

static void add_window_list( indri::infnet::TermOrderedEvaluator::InvertedList* result, 
                             indri::infnet::TermOrderedEvaluator::InvertedList* one, 
                             indri::infnet::TermOrderedEvaluator::InvertedList* two,
                             int windowSize,
                             bool ordered ) {
  int i=0;
  int j=0;
  int onePos = 0;
  int twoPos = 0;

  while( i < one->documents.size() && j < two->documents.size() ) {
    // find the same document
    if( one->documents[i] != two->documents[j] ) {
      int oneDoc = one->documents[i];
      int twoDoc = two->documents[j];

      if( oneDoc < twoDoc ) {
        onePos += one->counts[i];
        i++;
      } else {
        twoPos += two->counts[j];
        j++;
      }

      continue;
    }

    // check to make sure we're not finished
    if( i >= one->documents.size() || j >= two->documents.size() )
      break;

    int document = one->documents[i];

    // now we're in the document itself, so iterate down the first list
    int onePosEnd = onePos + one->counts[i];
    int twoPosEnd = twoPos + two->counts[j];

    // possibilities:
    //    the new term is before, inside, or after a current extent
    //    we need to check all three.  Also, we need to put the results out
    //    in sorted order

    indri::utility::greedy_vector<indri::infnet::TermOrderedEvaluator::Extent> extents;
    int lastStart = 0;

    for( int k = onePos; k < onePosEnd; k++ ) {
      int begin = one->positions[k].begin;
      int end = one->positions[k].end;

      for( int l = lastStart; l < twoPosEnd; l++ ) {
        int innerBegin = two->positions[l].begin;
        int innerEnd = two->positions[l].end;

        if( !ordered ) {
          if( innerBegin + windowSize < begin )
            lastStart = l + 1;

          if( innerBegin - windowSize > begin )
              break;
        } else {
          // the next word has to occur after the last one
          if( innerBegin < end ) {
            lastStart = l + 1;
            continue;
          }

          // the gap can't be larger than windowSize
          if( innerBegin - end > windowSize )
            break;
        }

        int totalBegin = std::min( innerBegin, begin );
        int totalEnd = std::max( innerEnd, end );

        if( !ordered && totalEnd - totalBegin > windowSize )
          continue;

        extents.push_back( indri::infnet::TermOrderedEvaluator::Extent(totalBegin, totalEnd) );
      }  

      if( lastStart == twoPosEnd )
        break;
    }

    std::sort( extents.begin(), extents.end() );

    if( extents.size() ) {
      result->counts.push_back( extents.size() );
      result->documents.push_back( document );
      for( int k=0; k<extents.size(); k++ ) {
        result->positions.push_back( extents[k] );
      }
    }

    // advance documents
    i++;
    j++;
  }
}

void indri::infnet::TermOrderedEvaluator::after( indri::lang::UWNode* uwNode ) {
  if( _listMap.find(uwNode) == _listMap.end() ) {
    const std::vector<indri::lang::RawExtentNode*>& children = uwNode->getChildren();
    std::vector< std::pair<UINT64, InvertedList*> > lists;
    int windowSize = uwNode->getWindowSize();

    // fetch the lists
    for( int i=0; i<children.size(); i++ ) {
      InvertedList* list = _listMap[children[i]];
      UINT64 count = list->totalCount();
      lists.push_back( std::make_pair( count, list ) );
    }

    // sort the lists
    std::sort( lists.begin(), lists.end() );
    
    // copy the first list into a window list
    InvertedList* result = new InvertedList;
    add_window_list( result, lists[0].second, lists[1].second, windowSize, false );
    lists.erase( lists.begin() );
    lists.erase( lists.begin() );

    while( lists.size() > 0 ) {
      InvertedList* previous = result;
      result = new InvertedList;
      add_window_list( result, lists[0].second, previous, windowSize, false );
      lists.erase( lists.begin() );
      delete previous;
    }

    _listMap[uwNode] = result;
  }
}

//
// ODNode
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::ODNode* odNode ) {
  if( _listMap.find(odNode) == _listMap.end() ) {
    const std::vector<indri::lang::RawExtentNode*>& children = odNode->getChildren();
    std::vector< InvertedList* > lists;
    int windowSize = odNode->getWindowSize();

    // fetch the lists
    for( int i=0; i<children.size(); i++ ) {
      InvertedList* list = _listMap[children[i]];
      lists.push_back( list );
    }

    // copy the first list into a window list
    InvertedList* result = new InvertedList;
    add_window_list( result, lists[0], lists[1], windowSize, true );
    lists.erase( lists.begin() );
    lists.erase( lists.begin() );

    while( lists.size() > 0 ) {
      InvertedList* previous = result;
      result = new InvertedList;
      add_window_list( result, previous, lists[0], windowSize, true );
      delete previous;
      lists.erase( lists.begin() );
    }

    _listMap[odNode] = result;
  }
}

//
// ExtentInside
//

void indri::infnet::TermOrderedEvaluator::after( indri::lang::ExtentInside* extentInside ) {
  if( _listMap.find(extentInside) == _listMap.end() ) {
    InvertedList* result = new InvertedList;
    
    InvertedList* inner = _listMap[extentInside->getInner()];
    InvertedList* outer = _listMap[extentInside->getOuter()];
    
    std::vector<int> indexes;
    indexes.resize( 2, 0 );
    std::vector<int> posindexes;
    
    std::vector<InvertedList*> lists;
    lists.push_back(inner);
    lists.push_back(outer);
    
    int document = 0;
    
    while(true) {
      // advance to same document here
      while( !_allSameDocument( lists, indexes ) ) {
        document = _listMaxDocument( lists, indexes );
        if( document == MAX_INT32 )
          break;
        _advanceToDocument( lists, indexes, posindexes, document );
      }
      
      if( document == MAX_INT32 )
        break;
        
      int outerPos = posindexes[1];
      int outerPosEnd = outer->counts[indexes[1]] + outerPos;
      
      int innerPos = posindexes[0];
      int innerPosEnd = inner->counts[indexes[0]] + innerPos;
      
      int document;
      int count = 0;
      
      for( ; innerPos < innerPosEnd; innerPos++ ) {
        int innerBegin = inner->positions[innerPos].begin;
        int innerEnd = inner->positions[innerPos].end;
                
        if( outer->positions[outerPos].begin > innerBegin )
          continue;
          
        while( outer->positions[outerPos].end < innerEnd ) {
          outerPos++;
          if( outerPos > outerPosEnd )
            break;
        }

        if( outerPos > outerPosEnd )
          break;
          
        int outerBegin = outer->positions[outerPos].begin;
        int outerEnd = outer->positions[outerPos].end;
        
        if( innerBegin >= outerBegin && innerEnd <= outerEnd ) {
          result->positions.push_back( Extent( innerBegin, innerEnd ) );
          count++;
        }
      }
      
      if( count ) {
        result->counts.push_back( count );
        result->documents.push_back( document );
      }
    }

    _listMap[extentInside] = result;
  }
}


    
    // ContextCounterNode
    // ContextSimpleCounterNode
    // ExtentInside
    // WeightedExtentOr
    // ExtentOr
    // ExtentAnd
    // UWNode
    // ODNode
    // FilReqNode
    // FilRejNode
    // TermFrequencyScorerNode
    // ScoreAccumulatorNode
    // ListAccumulator
    // WeightNode
    // WAndNode
    // WSumNode
