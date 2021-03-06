/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.25
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package edu.umass.cs.indri;
import java.util.Map;
public class QueryEnvironment {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected QueryEnvironment(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(QueryEnvironment obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      indriJNI.delete_QueryEnvironment(swigCPtr);
    }
    swigCPtr = 0;
  }

  public void addServer(String hostname) {
    indriJNI.QueryEnvironment_addServer(swigCPtr, hostname);
  }

  public void addIndex(String pathname) {
    indriJNI.QueryEnvironment_addIndex(swigCPtr, pathname);
  }

  public void removeServer(String hostname) {
    indriJNI.QueryEnvironment_removeServer(swigCPtr, hostname);
  }

  public void removeIndex(String pathname) {
    indriJNI.QueryEnvironment_removeIndex(swigCPtr, pathname);
  }

  public void close() {
    indriJNI.QueryEnvironment_close(swigCPtr);
  }

  public void setMemory(long memory) {
    indriJNI.QueryEnvironment_setMemory(swigCPtr, memory);
  }

  public void setScoringRules(String[] rules) {
    indriJNI.QueryEnvironment_setScoringRules(swigCPtr, rules);
  }

  public void setStopwords(String[] stopwords) {
    indriJNI.QueryEnvironment_setStopwords(swigCPtr, stopwords);
  }

  public ScoredExtentResult[] runQuery(String query, int resultsRequested) {
  return indriJNI.QueryEnvironment_runQuery__SWIG_0(swigCPtr, query, resultsRequested);
}

  public ScoredExtentResult[] runQuery(String query, int[] documentSet, int resultsRequested) {
  return indriJNI.QueryEnvironment_runQuery__SWIG_1(swigCPtr, query, documentSet, resultsRequested);
}

  public QueryAnnotation runAnnotatedQuery(String query, int resultsRequested) {
    long cPtr = indriJNI.QueryEnvironment_runAnnotatedQuery__SWIG_0(swigCPtr, query, resultsRequested);
    return (cPtr == 0) ? null : new QueryAnnotation(cPtr, false);
  }

  public QueryAnnotation runAnnotatedQuery(String query, int[] documentSet, int resultsRequested) {
    long cPtr = indriJNI.QueryEnvironment_runAnnotatedQuery__SWIG_1(swigCPtr, query, documentSet, resultsRequested);
    return (cPtr == 0) ? null : new QueryAnnotation(cPtr, false);
  }

  public ParsedDocument[] documents(int[] documentIDs) {
  return indriJNI.QueryEnvironment_documents__SWIG_0(swigCPtr, documentIDs);
}

  public ParsedDocument[] documents(ScoredExtentResult[] results) {
  return indriJNI.QueryEnvironment_documents__SWIG_1(swigCPtr, results);
}

  public String[] documentMetadata(int[] documentIDs, String attributeName) {
  return indriJNI.QueryEnvironment_documentMetadata__SWIG_0(swigCPtr, documentIDs, attributeName);
}

  public String[] documentMetadata(ScoredExtentResult[] documentIDs, String attributeName) {
  return indriJNI.QueryEnvironment_documentMetadata__SWIG_1(swigCPtr, documentIDs, attributeName);
}

  public int[] documentIDsFromMetadata(String attributeName, String[] attributeValue) {
  return indriJNI.QueryEnvironment_documentIDsFromMetadata(swigCPtr, attributeName, attributeValue);
}

  public ParsedDocument[] documentsFromMetadata(String attributeName, String[] attributeValue) {
  return indriJNI.QueryEnvironment_documentsFromMetadata(swigCPtr, attributeName, attributeValue);
}

  public long termCount() {
    return indriJNI.QueryEnvironment_termCount__SWIG_0(swigCPtr);
  }

  public long termCount(String term) {
    return indriJNI.QueryEnvironment_termCount__SWIG_1(swigCPtr, term);
  }

  public long termFieldCount(String term, String field) {
    return indriJNI.QueryEnvironment_termFieldCount(swigCPtr, term, field);
  }

  public String[] fieldList() {
  return indriJNI.QueryEnvironment_fieldList(swigCPtr);
}

  public long documentCount() {
    return indriJNI.QueryEnvironment_documentCount__SWIG_0(swigCPtr);
  }

  public long documentCount(String term) {
    return indriJNI.QueryEnvironment_documentCount__SWIG_1(swigCPtr, term);
  }

  public DocumentVector[] documentVectors(int[] documentIDs) {
  return indriJNI.QueryEnvironment_documentVectors(swigCPtr, documentIDs);
}

  public QueryEnvironment() {
    this(indriJNI.new_QueryEnvironment(), true);
  }

}
