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
// LocalQueryServer
//
// 15 March 2004 -- tds
//

#include "indri/LocalQueryServer.hpp"
#include "indri/QuerySpec.hpp"
#include "lemur/lemur-platform.h"
#include "lemur/lemur-compat.hpp"
#include <vector>

#include "indri/UnnecessaryNodeRemoverCopier.hpp"
#include "indri/ContextSimpleCountCollectorCopier.hpp"
#include "indri/FrequencyListCopier.hpp"
#include "indri/DagCopier.hpp"

#include "indri/InferenceNetworkBuilder.hpp"
#include "indri/InferenceNetwork.hpp"

#include "indri/CompressedCollection.hpp"
#include "indri/delete_range.hpp"
#include "indri/WeightFoldingCopier.hpp"

#include "indri/Appliers.hpp"

//
// Response objects
//

class LocalQueryServerResponse : public QueryServerResponse {
private:
  InferenceNetwork::MAllResults _results;

public:
  LocalQueryServerResponse( const InferenceNetwork::MAllResults& results ) :
    _results(results) {
  }
  
  InferenceNetwork::MAllResults& getResults() {
    return _results;
  }
};

class LocalQueryServerDocumentsResponse : public QueryServerDocumentsResponse {
private:
  std::vector<ParsedDocument*> _documents;

public:
  LocalQueryServerDocumentsResponse( const std::vector<ParsedDocument*>& results )
    :
    _documents(results)
  {
  }

  // caller's responsibility to delete these results
  std::vector<ParsedDocument*>& getResults() {
    return _documents;
  }
};

class LocalQueryServerMetadataResponse : public QueryServerMetadataResponse {
private:
  std::vector<std::string> _metadata;

public:
  LocalQueryServerMetadataResponse( const std::vector<std::string>& metadata ) :
    _metadata(metadata)
  {
  }

  std::vector<std::string>& getResults() {
    return _metadata;
  }
};

class LocalQueryServerVectorsResponse : public QueryServerVectorsResponse {
private:
  std::vector<DocumentVector*> _vectors;

public:
  LocalQueryServerVectorsResponse( int vectorCount ) {
    _vectors.reserve( vectorCount );
  }

  void addVector( DocumentVector* vec ) {
    _vectors.push_back( vec );
  }

  // caller deletes DocumentVector objects
  std::vector<DocumentVector*>& getResults() {
    return _vectors;
  }
};

//
// Class code
//

LocalQueryServer::LocalQueryServer( Repository& repository ) :
  _repository(repository)
{
}

//
// _indexWithDocument
//

indri::index::Index* LocalQueryServer::_indexWithDocument( int documentID ) {
  std::vector<indri::index::Index*> indexes = _repository.indexes();

  for( int i=0; i<indexes.size(); i++ ) {
    int lowerBound = indexes[i]->documentBase();
    int upperBound = indexes[i]->documentBase() + indexes[i]->documentCount();
    
    if( lowerBound <= documentID && upperBound > documentID ) {
      return indexes[i];
    }
  }
  
  return 0;
}

//
// document
//

ParsedDocument* LocalQueryServer::document( int documentID ) {
  CompressedCollection* collection = _repository.collection();
  ParsedDocument* document = collection->retrieve( documentID );
  return document;
}

std::string LocalQueryServer::documentMetadatum( int documentID, const std::string& attributeName ) {
  CompressedCollection* collection = _repository.collection();
  return collection->retrieveMetadatum( documentID, attributeName );
}

QueryServerMetadataResponse* LocalQueryServer::documentMetadata( const std::vector<int>& documentIDs, const std::string& attributeName ) {
  std::vector<std::string> result;

  std::vector<std::pair<int, int> > docSorted;
  for( unsigned int i=0; i<documentIDs.size(); i++ ) {
    docSorted.push_back( std::make_pair( documentIDs[i], i ) );
  }
  std::sort( docSorted.begin(), docSorted.end() );

  for( unsigned int i=0; i<docSorted.size(); i++ ) {
    result.push_back( documentMetadatum(docSorted[i].first, attributeName) );
  }

  std::vector<std::string> actual;
  actual.resize( documentIDs.size() );
  for( unsigned int i=0; i<docSorted.size(); i++ ) {
    actual[docSorted[i].second] = result[i];
  }

  return new LocalQueryServerMetadataResponse( actual );
}

QueryServerDocumentsResponse* LocalQueryServer::documents( const std::vector<int>& documentIDs ) {
  std::vector<ParsedDocument*> result;
  for( unsigned int i=0; i<documentIDs.size(); i++ ) {
    result.push_back( document(documentIDs[i]) );
  }
  return new LocalQueryServerDocumentsResponse( result );
}

INT64 LocalQueryServer::termCount() {
  std::vector<indri::index::Index*> indexes = _repository.indexes();
  INT64 total = 0;

  for( int i=0; i<indexes.size(); i++ ) {
    total += indexes[i]->termCount();
  }

  return total;
}

INT64 LocalQueryServer::termCount( const std::string& term ) {
  std::string stem = _repository.processTerm( term );
  return stemCount( stem );
}

INT64 LocalQueryServer::stemCount( const std::string& stem ) {
  std::vector<indri::index::Index*> indexes = _repository.indexes();
  INT64 total = 0;

  for( int i=0; i<indexes.size(); i++ ) {
    total += indexes[i]->termCount( stem );
  }

  return total;
}

INT64 LocalQueryServer::termFieldCount( const std::string& term, const std::string& field ) {
  std::string stem = _repository.processTerm( term );
  return stemFieldCount( stem, field );
}

INT64 LocalQueryServer::stemFieldCount( const std::string& stem, const std::string& field ) {
  std::vector<indri::index::Index*> indexes = _repository.indexes();
  INT64 total = 0;

  for( int i=0; i<indexes.size(); i++ ) {
    total += indexes[i]->fieldTermCount( field, stem );
  }

  return total;
}

std::string LocalQueryServer::termName( int term ) {
  indri::index::Index* index = _repository.indexes()[0];
  return index->term( term );
}

int LocalQueryServer::termID( const std::string& term ) {
  indri::index::Index* index = _repository.indexes()[0];
  std::string processed = _repository.processTerm( term );
  return index->term( processed.c_str() );
}

std::vector<std::string> LocalQueryServer::fieldList() {
  // TODO: fix this method
  assert( 0 && "This method is currently not in service" );

  std::vector<std::string> results;
  /*
  std::vector<indri::index::Index*> indexes = _repository.indexes();
  IndriIndex* index = _repository.index();

  for( unsigned int i=1; ; i++ ) {
    const char* fieldName = index->field(i);
    
    if( !strcmp( fieldName, "[OOV]" ) )
      break;

    results.push_back( std::string(fieldName) );
  }
*/
  return results;
}

int LocalQueryServer::documentLength( int documentID ) {
  indri::index::Index* index = _indexWithDocument( documentID );

  if( index ) {
    return index->documentLength( documentID );
  }

  return 0;
}

INT64 LocalQueryServer::documentCount() {
  std::vector<indri::index::Index*> indexes = _repository.indexes();
  INT64 total = 0;
  
  for( int i=0; i<indexes.size(); i++ ) {
    total += indexes[i]->documentCount();
  }
  
  return total;
}

INT64 LocalQueryServer::documentCount( const std::string& term ) {
  std::vector<indri::index::Index*> indexes = _repository.indexes();
  INT64 total = 0;
  
  for( int i=0; i<indexes.size(); i++ ) {
    total += indexes[i]->documentCount( term );
  }
  
  return total;
}

QueryServerResponse* LocalQueryServer::runQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested, bool optimize ) {
  // use UnnecessaryNodeRemover to get rid of window nodes, ExtentAnd nodes and ExtentOr nodes
  // that only have one child
  ApplyCopiers<UnnecessaryNodeRemoverCopier> unnecessary( roots );

  // run the contextsimplecountcollectorcopier to gather easy stats
  ApplyCopiers<ContextSimpleCountCollectorCopier> contexts( unnecessary.roots() );

  // use frequency-only nodes where appropriate
  ApplyCopiers<FrequencyListCopier> frequency( contexts.roots(), _cache );

  // fold together any nested weight nodes
  ApplyCopiers<WeightFoldingCopier> weight( frequency.roots() );

  // make all this into a dag
  ApplySingleCopier<DagCopier> dag( weight.roots(), _repository );

  std::vector<indri::lang::Node*>& networkRoots = dag.roots();

  if( !optimize ) {
    // we may be asked not to perform optimizations that might
    // drastically change the structure of the tree; for instance,
    // annotation queries may ask for this
    networkRoots = contexts.roots();
  }

  // build an inference network
  InferenceNetworkBuilder builder( _repository, _cache, resultsRequested );
  ApplyWalker<InferenceNetworkBuilder> buildWalker( networkRoots, &builder );

  InferenceNetwork* network = builder.getNetwork();
  InferenceNetwork::MAllResults result;
  result = network->evaluate();

  return new LocalQueryServerResponse( result );
}

QueryServerVectorsResponse* LocalQueryServer::documentVectors( const std::vector<int>& documentIDs ) {
  LocalQueryServerVectorsResponse* response = new LocalQueryServerVectorsResponse( documentIDs.size() );
  std::map<int, std::string> termIDStringMap;

  for( size_t i=0; i<documentIDs.size(); i++ ) {
    indri::index::Index* index = _indexWithDocument( documentIDs[i] );
    const indri::index::TermList* termList = index->termList( documentIDs[i] );
    DocumentVector* result = new DocumentVector( index, termList, termIDStringMap );
    delete termList;
    response->addVector( result );
  }

  return response;
}
