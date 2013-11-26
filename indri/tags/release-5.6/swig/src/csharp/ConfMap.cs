/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace Indri {

using System;
using System.Runtime.InteropServices;

public class ConfMap : IDisposable 
#if !SWIG_DOTNET_1
    , System.Collections.Generic.IDictionary<ConflationPattern, string>
#endif
 {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal ConfMap(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(ConfMap obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ConfMap() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          indri_csharpPINVOKE.delete_ConfMap(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }


  public string this[ConflationPattern key] {
    get {
      return getitem(key);
    }

    set {
      setitem(key, value);
    }
  }

  public bool TryGetValue(ConflationPattern key, out string value) {
    if (this.ContainsKey(key)) {
      value = this[key];
      return true;
    }
    value = default(string);
    return false;
  }

  public int Count {
    get {
      return (int)size();
    }
  }

  public bool IsReadOnly {
    get { 
      return false; 
    }
  }

#if !SWIG_DOTNET_1

  public System.Collections.Generic.ICollection<ConflationPattern> Keys {
    get {
      System.Collections.Generic.ICollection<ConflationPattern> keys = new System.Collections.Generic.List<ConflationPattern>();
      IntPtr iter = create_iterator_begin();
      try {
        while (true) {
          keys.Add(get_next_key(iter));
        }
      } catch (ArgumentOutOfRangeException) {
      }
      return keys;
    }
  }

  public System.Collections.Generic.ICollection<string> Values {
    get {
      System.Collections.Generic.ICollection<string> vals = new System.Collections.Generic.List<string>();
      foreach (System.Collections.Generic.KeyValuePair<ConflationPattern, string> pair in this) {
        vals.Add(pair.Value);
      }
      return vals;
    }
  }
  
  public void Add(System.Collections.Generic.KeyValuePair<ConflationPattern, string> item) {
    Add(item.Key, item.Value);
  }

  public bool Remove(System.Collections.Generic.KeyValuePair<ConflationPattern, string> item) {
    if (Contains(item)) {
      return Remove(item.Key);
    } else {
      return false;
    }
  }

  public bool Contains(System.Collections.Generic.KeyValuePair<ConflationPattern, string> item) {
    if (this[item.Key] == item.Value) {
      return true;
    } else {
      return false;
    }
  }

  public void CopyTo(System.Collections.Generic.KeyValuePair<ConflationPattern, string>[] array) {
    CopyTo(array, 0);
  }

  public void CopyTo(System.Collections.Generic.KeyValuePair<ConflationPattern, string>[] array, int arrayIndex) {
    if (array == null)
      throw new ArgumentNullException("array");
    if (arrayIndex < 0)
      throw new ArgumentOutOfRangeException("arrayIndex", "Value is less than zero");
    if (array.Rank > 1)
      throw new ArgumentException("Multi dimensional array.", "array");
    if (arrayIndex+this.Count > array.Length)
      throw new ArgumentException("Number of elements to copy is too large.");

    System.Collections.Generic.IList<ConflationPattern> keyList = new System.Collections.Generic.List<ConflationPattern>(this.Keys);
    for (int i = 0; i < keyList.Count; i++) {
      ConflationPattern currentKey = keyList[i];
      array.SetValue(new System.Collections.Generic.KeyValuePair<ConflationPattern, string>(currentKey, this[currentKey]), arrayIndex+i);
    }
  }

  System.Collections.Generic.IEnumerator<System.Collections.Generic.KeyValuePair<ConflationPattern, string>> System.Collections.Generic.IEnumerable<System.Collections.Generic.KeyValuePair<ConflationPattern, string>>.GetEnumerator() {
    return new ConfMapEnumerator(this);
  }

  System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() {
    return new ConfMapEnumerator(this);
  }

  public ConfMapEnumerator GetEnumerator() {
    return new ConfMapEnumerator(this);
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class ConfMapEnumerator : System.Collections.IEnumerator, 
      System.Collections.Generic.IEnumerator<System.Collections.Generic.KeyValuePair<ConflationPattern, string>>
  {
    private ConfMap collectionRef;
    private System.Collections.Generic.IList<ConflationPattern> keyCollection;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public ConfMapEnumerator(ConfMap collection) {
      collectionRef = collection;
      keyCollection = new System.Collections.Generic.List<ConflationPattern>(collection.Keys);
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public System.Collections.Generic.KeyValuePair<ConflationPattern, string> Current {
      get {
        if (currentIndex == -1)
          throw new InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new InvalidOperationException("Collection modified.");
        return (System.Collections.Generic.KeyValuePair<ConflationPattern, string>)currentObject;
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
        ConflationPattern currentKey = keyCollection[currentIndex];
        currentObject = new System.Collections.Generic.KeyValuePair<ConflationPattern, string>(currentKey, collectionRef[currentKey]);
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

    public void Dispose() {
      currentIndex = -1;
      currentObject = null;
    }
  }
#endif
  

  public ConfMap() : this(indri_csharpPINVOKE.new_ConfMap__SWIG_0(), true) {
  }

  public ConfMap(ConfMap other) : this(indri_csharpPINVOKE.new_ConfMap__SWIG_1(ConfMap.getCPtr(other)), true) {
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  private uint size() {
    uint ret = indri_csharpPINVOKE.ConfMap_size(swigCPtr);
    return ret;
  }

  public bool empty() {
    bool ret = indri_csharpPINVOKE.ConfMap_empty(swigCPtr);
    return ret;
  }

  public void Clear() {
    indri_csharpPINVOKE.ConfMap_Clear(swigCPtr);
  }

  private string getitem(ConflationPattern key) {
    string ret = indri_csharpPINVOKE.ConfMap_getitem(swigCPtr, ConflationPattern.getCPtr(key));
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(ConflationPattern key, string x) {
    indri_csharpPINVOKE.ConfMap_setitem(swigCPtr, ConflationPattern.getCPtr(key), x);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool ContainsKey(ConflationPattern key) {
    bool ret = indri_csharpPINVOKE.ConfMap_ContainsKey(swigCPtr, ConflationPattern.getCPtr(key));
    return ret;
  }

  public void Add(ConflationPattern key, string val) {
    indri_csharpPINVOKE.ConfMap_Add(swigCPtr, ConflationPattern.getCPtr(key), val);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool Remove(ConflationPattern key) {
    bool ret = indri_csharpPINVOKE.ConfMap_Remove(swigCPtr, ConflationPattern.getCPtr(key));
    return ret;
  }

  private IntPtr create_iterator_begin() {
    IntPtr ret = indri_csharpPINVOKE.ConfMap_create_iterator_begin(swigCPtr);
    return ret;
  }

  private ConflationPattern get_next_key(IntPtr swigiterator) {
    IntPtr cPtr = indri_csharpPINVOKE.ConfMap_get_next_key(swigCPtr, swigiterator);
    ConflationPattern ret = (cPtr == IntPtr.Zero) ? null : new ConflationPattern(cPtr, false);
    if (indri_csharpPINVOKE.SWIGPendingException.Pending) throw indri_csharpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
