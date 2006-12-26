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

public class TermExtentVector : IDisposable, System.Collections.IEnumerable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal TermExtentVector(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(TermExtentVector obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~TermExtentVector() {
    Dispose();
  }

  public virtual void Dispose() {
    if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
      swigCMemOwn = false;
      indri_csharpPINVOKE.delete_TermExtentVector(swigCPtr);
    }
    swigCPtr = new HandleRef(null, IntPtr.Zero);
    GC.SuppressFinalize(this);
  }

  public TermExtentVector(System.Collections.ICollection c) : this() {
    if (c == null)
      throw new ArgumentNullException("c");
    foreach (TermExtent element in c) {
      this.Add(element);
    }
  }

  public bool IsFixedSize {
    get {
      return false;
    }
  }

  public bool IsReadOnly {
    get {
      return false;
    }
  }

  public TermExtent this[int index]  {
    get {
      return getitem(index);
    }
    set {
      setitem(index, value);
    }
  }

  public int Capacity {
    get {
      return (int)capacity();
    }
    set {
      if (value < size())
        throw new ArgumentOutOfRangeException("Capacity");
      reserve((uint)value);
    }
  }

  public int Count {
    get {
      return (int)size();
    }
  }

  public bool IsSynchronized {
    get {
      return false;
    }
  }

  public void CopyTo(System.Array array) {
    CopyTo(0, array, 0, this.Count);
  }

  public void CopyTo(System.Array array, int arrayIndex) {
    CopyTo(0, array, arrayIndex, this.Count);
  }

  public void CopyTo(int index, System.Array array, int arrayIndex, int count) {
    if (array == null)
      throw new ArgumentNullException("array");
    if (index < 0)
      throw new ArgumentOutOfRangeException("index", "Value is less than zero");
    if (arrayIndex < 0)
      throw new ArgumentOutOfRangeException("arrayIndex", "Value is less than zero");
    if (count < 0)
      throw new ArgumentOutOfRangeException("count", "Value is less than zero");
    if (array.Rank > 1)
      throw new ArgumentException("Multi dimensional array.");
    if (index+count > this.Count || arrayIndex+count > array.Length)
      throw new ArgumentException("Number of elements to copy is too large.");
    for (int i=0; i<count; i++)
      array.SetValue(getitemcopy(index+i), arrayIndex+i);
  }

  // Type-safe version of IEnumerable.GetEnumerator
  System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() {
    return new TermExtentVectorEnumerator(this);
  }

  public TermExtentVectorEnumerator GetEnumerator() {
    return new TermExtentVectorEnumerator(this);
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class TermExtentVectorEnumerator : System.Collections.IEnumerator {
    private TermExtentVector collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public TermExtentVectorEnumerator(TermExtentVector collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public TermExtent Current {
      get {
        if (currentIndex == -1)
          throw new InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new InvalidOperationException("Collection modified.");
        return (TermExtent)currentObject;
      }
    }

    // Type-unsafe IEnumerator.Current
    object System.Collections.IEnumerator.Current {
      get {
        return Current;
      }
    }

    public bool MoveNext() {
      int size = collectionRef.Count;
      bool moveOkay = (currentIndex+1 < size) && (size == currentSize);
      if (moveOkay) {
        currentIndex++;
        currentObject = collectionRef[currentIndex];
      } else {
        currentObject = null;
      }
      return moveOkay;
    }

    public void Reset() {
      currentIndex = -1;
      currentObject = null;
      if (collectionRef.Count != currentSize) {
        throw new InvalidOperationException("Collection modified.");
      }
    }
  }

  public void Clear() {
    indri_csharpPINVOKE.TermExtentVector_Clear(swigCPtr);
  }

  public void Add(TermExtent value) {
    indri_csharpPINVOKE.TermExtentVector_Add(swigCPtr, TermExtent.getCPtr(value));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  private uint size() {
    uint ret = indri_csharpPINVOKE.TermExtentVector_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = indri_csharpPINVOKE.TermExtentVector_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    indri_csharpPINVOKE.TermExtentVector_reserve(swigCPtr, n);
  }

  public TermExtentVector() : this(indri_csharpPINVOKE.new_TermExtentVector__SWIG_0(), true) {
  }

  public TermExtentVector(int capacity) : this(indri_csharpPINVOKE.new_TermExtentVector__SWIG_1(capacity), true) {
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  private TermExtent getitemcopy(int index) {
    TermExtent ret = new TermExtent(indri_csharpPINVOKE.TermExtentVector_getitemcopy(swigCPtr, index), true);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private TermExtent getitem(int index) {
    TermExtent ret = new TermExtent(indri_csharpPINVOKE.TermExtentVector_getitem(swigCPtr, index), false);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, TermExtent value) {
    indri_csharpPINVOKE.TermExtentVector_setitem(swigCPtr, index, TermExtent.getCPtr(value));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(TermExtentVector values) {
    indri_csharpPINVOKE.TermExtentVector_AddRange(swigCPtr, TermExtentVector.getCPtr(values));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public TermExtentVector GetRange(int index, int count) {
    IntPtr cPtr = indri_csharpPINVOKE.TermExtentVector_GetRange(swigCPtr, index, count);
    TermExtentVector ret = (cPtr == IntPtr.Zero) ? null : new TermExtentVector(cPtr, true);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, TermExtent value) {
    indri_csharpPINVOKE.TermExtentVector_Insert(swigCPtr, index, TermExtent.getCPtr(value));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, TermExtentVector values) {
    indri_csharpPINVOKE.TermExtentVector_InsertRange(swigCPtr, index, TermExtentVector.getCPtr(values));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    indri_csharpPINVOKE.TermExtentVector_RemoveAt(swigCPtr, index);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    indri_csharpPINVOKE.TermExtentVector_RemoveRange(swigCPtr, index, count);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public static TermExtentVector Repeat(TermExtent value, int count) {
    IntPtr cPtr = indri_csharpPINVOKE.TermExtentVector_Repeat(TermExtent.getCPtr(value), count);
    TermExtentVector ret = (cPtr == IntPtr.Zero) ? null : new TermExtentVector(cPtr, true);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    indri_csharpPINVOKE.TermExtentVector_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    indri_csharpPINVOKE.TermExtentVector_Reverse__SWIG_1(swigCPtr, index, count);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, TermExtentVector values) {
    indri_csharpPINVOKE.TermExtentVector_SetRange(swigCPtr, index, TermExtentVector.getCPtr(values));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

}

}
