%module indri
%{
#include "indri/indri-platform.h"
#include "lemur/lemur-compat.hpp"
#include "indri/QueryEnvironment.hpp"
#include "lemur/Exception.hpp"
// remap overloaded method names.
#define onetermCount termCount
#define onedocumentCount documentCount
#define runQuerydocset runQuery
#define runAnnotatedQuerydocset runAnnotatedQuery
#define documentsdocids documents
#define documentMetadatadocids documentMetadata
%}

%include "typemaps.i"
%include "std_string.i"
%include "exception.i"
%include "indritypemaps.i"


class ScoredExtentResult {
  double score;
  int document;
  int begin;
  int end;
};

struct TermExtent {
  int begin;
  int end;
};

// omit key_equal class.
// map this directly into a PHP array
// force value to char *.
// don't even need a wrapper class
/*
struct MetadataPair {
  char* key;
  char* value;
  int valueLength;
};
*/
class ParsedDocument {
  const char* text;
  size_t textLength;

  greedy_vector<char*> terms;
  greedy_vector<TagExtent> tags;
  greedy_vector<TermExtent> positions;
  greedy_vector<MetadataPair> metadata;

};


class QueryAnnotationNode {
  std::string name;
  std::string type;
  std::string queryText;
  std::vector<QueryAnnotationNode*> children;
};


class QueryAnnotation {
public:
  const QueryAnnotationNode* getQueryTree() const;
  const EvaluatorNode::MResults& getAnnotations() const;
  const std::vector<ScoredExtentResult>& getResults() const;
};

// need separate map for individual methods.
%define setEx(method) 
%exception method {
  try {
    $action
  } catch( Exception& e ) {
//    SWIG_exception( SWIG_RuntimeError, e.what().c_str() );
      // get a warning message rather than abort the script.
      zend_error(E_WARNING, e.what().c_str());
    RETURN_NULL() ;
  }
}
%enddef
setEx(QueryEnvironment::runQuery);
setEx(QueryEnvironment::runQuerydocset);
setEx(QueryEnvironment::runAnnotatedQuery);
setEx(QueryEnvironment::runAnnotatedQuerydocset);
setEx(QueryEnvironment::documents);
setEx(QueryEnvironment::documentsdocids);
setEx(QueryEnvironment::documentMetadata)
setEx(QueryEnvironment::documentMetadatadocids);

class QueryEnvironment {
public:
  void addServer( const std::string& hostname );
  void addIndex( const std::string& pathname );
  void close();
  
  void setMemory( UINT64 memory );
  void setScoringRules( const std::vector<std::string>& rules );
  void setStopwords( const std::vector<std::string>& stopwords );

  std::vector<ScoredExtentResult> runQuery( const std::string& query, int resultsRequested );
  std::vector<ScoredExtentResult> runQuerydocset( const std::string& query, const std::vector<DOCID_T>& documentSet, int resultsRequested );
  QueryAnnotation* runAnnotatedQuery( const std::string& query, int resultsRequested );
  QueryAnnotation* runAnnotatedQuerydocset( const std::string& query, const std::vector<DOCID_T>& documentSet, int resultsRequested );

  std::vector<ParsedDocument*> documentsdocids( const std::vector<DOCID_T>& documentIDs );
  std::vector<ParsedDocument*> documents( const std::vector<ScoredExtentResult>& results );

  std::vector<std::string> documentMetadatadocids( const std::vector<DOCID_T>& documentIDs, const std::string& attributeName );
  std::vector<std::string> documentMetadata( const std::vector<ScoredExtentResult>& documentIDs, const std::string& attributeName );

  INT64 termCount();
  INT64 onetermCount( const std::string& term );
  INT64 stemCount( const std::string& term );
  INT64 termFieldCount( const std::string& term, const std::string& field );
  INT64 stemFieldCount( const std::string& term, const std::string& field );
  std::vector<std::string> fieldList();
  INT64 documentCount();
  INT64 onedocumentCount( const std::string& term );
};
