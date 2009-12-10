/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.36
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace Indri {

using System;
using System.Runtime.InteropServices;

public class MetadataPair : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal MetadataPair(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(MetadataPair obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~MetadataPair() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        indri_csharpPINVOKE.delete_MetadataPair(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public string key {
    set {
      indri_csharpPINVOKE.MetadataPair_key_set(swigCPtr, value);
    } 
    get {
      string ret = indri_csharpPINVOKE.MetadataPair_key_get(swigCPtr);
      return ret;
    } 
  }

  public string value {
    set {
      indri_csharpPINVOKE.MetadataPair_value_set(swigCPtr, value);
    } 
    get {
      string ret = indri_csharpPINVOKE.MetadataPair_value_get(swigCPtr);
      return ret;
    } 
  }

  public int valueLength {
    set {
      indri_csharpPINVOKE.MetadataPair_valueLength_set(swigCPtr, value);
    } 
    get {
      int ret = indri_csharpPINVOKE.MetadataPair_valueLength_get(swigCPtr);
      return ret;
    } 
  }

  public MetadataPair() : this(indri_csharpPINVOKE.new_MetadataPair(), true) {
  }

}

}
