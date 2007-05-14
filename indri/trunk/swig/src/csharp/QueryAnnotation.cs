/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.31
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace Indri {

using System;
using System.Runtime.InteropServices;

public class QueryAnnotation : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal QueryAnnotation(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(QueryAnnotation obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~QueryAnnotation() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        indri_csharpPINVOKE.delete_QueryAnnotation(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public QueryAnnotationNode getQueryTree() {
    IntPtr cPtr = indri_csharpPINVOKE.QueryAnnotation_getQueryTree(swigCPtr);
    QueryAnnotationNode ret = (cPtr == IntPtr.Zero) ? null : new QueryAnnotationNode(cPtr, false);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public MResults getAnnotations() {
    MResults ret = new MResults(indri_csharpPINVOKE.QueryAnnotation_getAnnotations(swigCPtr), false);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ScoredExtentResultVector getResults() {
    ScoredExtentResultVector ret = new ScoredExtentResultVector(indri_csharpPINVOKE.QueryAnnotation_getResults(swigCPtr), false);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public QueryAnnotation() : this(indri_csharpPINVOKE.new_QueryAnnotation(), true) {
  }

}

}
