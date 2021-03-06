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

public class ScoredExtentResultVector : IDisposable, System.Collections.IEnumerable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal ScoredExtentResultVector(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(ScoredExtentResultVector obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ScoredExtentResultVector() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        indri_csharpPINVOKE.delete_ScoredExtentResultVector(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public ScoredExtentResultVector(System.Collections.ICollection c) : this() {
    if (c == null)
      throw new ArgumentNullException("c");
    foreach (ScoredExtentResult element in c) {
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

  public ScoredExtentResult this[int index]  {
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
    return new ScoredExtentResultVectorEnumerator(this);
  }

  public ScoredExtentResultVectorEnumerator GetEnumerator() {
    return new ScoredExtentResultVectorEnumerator(this);
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class ScoredExtentResultVectorEnumerator : System.Collections.IEnumerator {
    private ScoredExtentResultVector collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public ScoredExtentResultVectorEnumerator(ScoredExtentResultVector collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public ScoredExtentResult Current {
      get {
        if (currentIndex == -1)
          throw new InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new InvalidOperationException("Collection modified.");
        return (ScoredExtentResult)currentObject;
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
    indri_csharpPINVOKE.ScoredExtentResultVector_Clear(swigCPtr);
  }

  public void Add(ScoredExtentResult x) {
    indri_csharpPINVOKE.ScoredExtentResultVector_Add(swigCPtr, ScoredExtentResult.getCPtr(x));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  private uint size() {
    uint ret = indri_csharpPINVOKE.ScoredExtentResultVector_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = indri_csharpPINVOKE.ScoredExtentResultVector_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    indri_csharpPINVOKE.ScoredExtentResultVector_reserve(swigCPtr, n);
  }

  public ScoredExtentResultVector() : this(indri_csharpPINVOKE.new_ScoredExtentResultVector__SWIG_0(), true) {
  }

  public ScoredExtentResultVector(int capacity) : this(indri_csharpPINVOKE.new_ScoredExtentResultVector__SWIG_1(capacity), true) {
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  private ScoredExtentResult getitemcopy(int index) {
    ScoredExtentResult ret = new ScoredExtentResult(indri_csharpPINVOKE.ScoredExtentResultVector_getitemcopy(swigCPtr, index), true);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private ScoredExtentResult getitem(int index) {
    ScoredExtentResult ret = new ScoredExtentResult(indri_csharpPINVOKE.ScoredExtentResultVector_getitem(swigCPtr, index), false);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, ScoredExtentResult val) {
    indri_csharpPINVOKE.ScoredExtentResultVector_setitem(swigCPtr, index, ScoredExtentResult.getCPtr(val));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(ScoredExtentResultVector values) {
    indri_csharpPINVOKE.ScoredExtentResultVector_AddRange(swigCPtr, ScoredExtentResultVector.getCPtr(values));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public ScoredExtentResultVector GetRange(int index, int count) {
    IntPtr cPtr = indri_csharpPINVOKE.ScoredExtentResultVector_GetRange(swigCPtr, index, count);
    ScoredExtentResultVector ret = (cPtr == IntPtr.Zero) ? null : new ScoredExtentResultVector(cPtr, true);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, ScoredExtentResult x) {
    indri_csharpPINVOKE.ScoredExtentResultVector_Insert(swigCPtr, index, ScoredExtentResult.getCPtr(x));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, ScoredExtentResultVector values) {
    indri_csharpPINVOKE.ScoredExtentResultVector_InsertRange(swigCPtr, index, ScoredExtentResultVector.getCPtr(values));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    indri_csharpPINVOKE.ScoredExtentResultVector_RemoveAt(swigCPtr, index);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    indri_csharpPINVOKE.ScoredExtentResultVector_RemoveRange(swigCPtr, index, count);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public static ScoredExtentResultVector Repeat(ScoredExtentResult value, int count) {
    IntPtr cPtr = indri_csharpPINVOKE.ScoredExtentResultVector_Repeat(ScoredExtentResult.getCPtr(value), count);
    ScoredExtentResultVector ret = (cPtr == IntPtr.Zero) ? null : new ScoredExtentResultVector(cPtr, true);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    indri_csharpPINVOKE.ScoredExtentResultVector_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    indri_csharpPINVOKE.ScoredExtentResultVector_Reverse__SWIG_1(swigCPtr, index, count);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, ScoredExtentResultVector values) {
    indri_csharpPINVOKE.ScoredExtentResultVector_SetRange(swigCPtr, index, ScoredExtentResultVector.getCPtr(values));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

}

}
