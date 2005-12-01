/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.25
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package edu.umass.cs.indri;
import java.util.Map;
public class IndexEnvironment {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected IndexEnvironment(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(IndexEnvironment obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      indriJNI.delete_IndexEnvironment(swigCPtr);
    }
    swigCPtr = 0;
  }

  public IndexEnvironment() {
    this(indriJNI.new_IndexEnvironment(), true);
  }

  public void setDocumentRoot(String documentRoot) {
    indriJNI.IndexEnvironment_setDocumentRoot(swigCPtr, documentRoot);
  }

  public void setAnchorTextPath(String anchorTextRoot) {
    indriJNI.IndexEnvironment_setAnchorTextPath(swigCPtr, anchorTextRoot);
  }

  public void setOffsetMetadataPath(String offsetMetadataRoot) {
    indriJNI.IndexEnvironment_setOffsetMetadataPath(swigCPtr, offsetMetadataRoot);
  }

  public void setOffsetAnnotationsPath(String offsetAnnotationsRoot) {
    indriJNI.IndexEnvironment_setOffsetAnnotationsPath(swigCPtr, offsetAnnotationsRoot);
  }

  public void addFileClass(String name, String iterator, String parser, String tokenizer, String startDocTag, String endDocTag, String endMetadataTag, String[] include, String[] exclude, String[] index, String[] metadata, Map conflations) {
    indriJNI.IndexEnvironment_addFileClass__SWIG_0(swigCPtr, name, iterator, parser, tokenizer, startDocTag, endDocTag, endMetadataTag, include, exclude, index, metadata, conflations);
  }

  public Specification getFileClassSpec(String name) {
  return indriJNI.IndexEnvironment_getFileClassSpec(swigCPtr, name);
}

  public void addFileClass(Specification spec) {
    indriJNI.IndexEnvironment_addFileClass__SWIG_1(swigCPtr, spec);
  }

  public void setIndexedFields(String[] fieldNames) {
    indriJNI.IndexEnvironment_setIndexedFields(swigCPtr, fieldNames);
  }

  public void setNumericField(String fieldName, boolean isNumeric) {
    indriJNI.IndexEnvironment_setNumericField(swigCPtr, fieldName, isNumeric);
  }

  public void setMetadataIndexedFields(String[] forward, String[] backward) {
    indriJNI.IndexEnvironment_setMetadataIndexedFields(swigCPtr, forward, backward);
  }

  public void setStopwords(String[] stopwords) {
    indriJNI.IndexEnvironment_setStopwords(swigCPtr, stopwords);
  }

  public void setStemmer(String stemmer) {
    indriJNI.IndexEnvironment_setStemmer(swigCPtr, stemmer);
  }

  public void setMemory(long memory) {
    indriJNI.IndexEnvironment_setMemory(swigCPtr, memory);
  }

  public void setNormalization(boolean normalize) {
    indriJNI.IndexEnvironment_setNormalization(swigCPtr, normalize);
  }

  public void create(String repositoryPath, IndexStatus callback) {
    indriJNI.IndexEnvironment_create__SWIG_0(swigCPtr, repositoryPath, IndexStatus.getCPtr(callback));
  }

  public void create(String repositoryPath) {
    indriJNI.IndexEnvironment_create__SWIG_1(swigCPtr, repositoryPath);
  }

  public void open(String repositoryPath, IndexStatus callback) {
    indriJNI.IndexEnvironment_open__SWIG_0(swigCPtr, repositoryPath, IndexStatus.getCPtr(callback));
  }

  public void open(String repositoryPath) {
    indriJNI.IndexEnvironment_open__SWIG_1(swigCPtr, repositoryPath);
  }

  public void close() {
    indriJNI.IndexEnvironment_close(swigCPtr);
  }

  public void addFile(String fileName) {
    indriJNI.IndexEnvironment_addFile__SWIG_0(swigCPtr, fileName);
  }

  public void addFile(String fileName, String fileClass) {
    indriJNI.IndexEnvironment_addFile__SWIG_1(swigCPtr, fileName, fileClass);
  }

  public int addString(String fileName, String fileClass, Map metadata) {
    return indriJNI.IndexEnvironment_addString(swigCPtr, fileName, fileClass, metadata);
  }

  public int addParsedDocument(ParsedDocument document) {
    return indriJNI.IndexEnvironment_addParsedDocument(swigCPtr, document);
  }

  public int documentsIndexed() {
    return indriJNI.IndexEnvironment_documentsIndexed(swigCPtr);
  }

  public int documentsSeen() {
    return indriJNI.IndexEnvironment_documentsSeen(swigCPtr);
  }

}
