/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.29
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace Indri {

using System;
using System.Runtime.InteropServices;

public class RMExpander : QueryExpander {
  private HandleRef swigCPtr;

  internal RMExpander(IntPtr cPtr, bool cMemoryOwn) : base(indriPINVOKE.RMExpanderUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(RMExpander obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~RMExpander() {
    Dispose();
  }

  public override void Dispose() {
    if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
      swigCMemOwn = false;
      indriPINVOKE.delete_RMExpander(swigCPtr);
    }
    swigCPtr = new HandleRef(null, IntPtr.Zero);
    GC.SuppressFinalize(this);
    base.Dispose();
  }

  public RMExpander(QueryEnvironment env, Parameters param) : this(indriPINVOKE.new_RMExpander(QueryEnvironment.getCPtr(env), Parameters.getCPtr(param)), true) {
    if (indriPINVOKE.SWIGPendingException.Pending) throw indriPINVOKE.SWIGPendingException.Retrieve();
  }

  public override string expand(string originalQuery, ScoredExtentResultVector results) {
    string ret = indriPINVOKE.RMExpander_expand(swigCPtr, originalQuery, ScoredExtentResultVector.getCPtr(results));
    if (indriPINVOKE.SWIGPendingException.Pending) throw indriPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
