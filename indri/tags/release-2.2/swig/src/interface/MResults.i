
//
// MResults.i
//
// Typemaps for EvaluatorNode::MResults&
//
// 10 August 2004 -- tds
//

%typemap(jni) const indri::infnet::EvaluatorNode::MResults& "jobject"
%typemap(jtype) const indri::infnet::EvaluatorNode::MResults& "java.util.Map"
%typemap(jstype) const indri::infnet::EvaluatorNode::MResults& "java.util.Map"

%typemap(java,out) const indri::infnet::EvaluatorNode::MResults&
{
  indri::infnet::EvaluatorNode::MResults::iterator iter;

  // make the map
  jclass mapClazz = jenv->FindClass("java/util/HashMap");
  jmethodID mapConstructor = jenv->GetMethodID(mapClazz, "<init>", "()V" );
  $result = jenv->NewObject(mapClazz, mapConstructor);
  jmethodID putMethod = jenv->GetMethodID(mapClazz, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );

  // look up information about ScoredExtentResults
  jclass seClazz = jenv->FindClass("edu/umass/cs/indri/ScoredExtentResult");
  jmethodID seConstructor = jenv->GetMethodID(seClazz, "<init>", "()V" );

  jfieldID scoreField = jenv->GetFieldID(seClazz, "score", "D" );
  jfieldID beginField = jenv->GetFieldID(seClazz, "begin", "I" );
  jfieldID endField = jenv->GetFieldID(seClazz, "end", "I" );
  jfieldID documentField = jenv->GetFieldID(seClazz, "document", "I" );

  for( iter = $1->begin(); iter != $1->end(); iter++ ) {
    std::vector<indri::api::ScoredExtentResult>& vec = iter->second;

    // make an array for this list of results
    jobjectArray array = jenv->NewObjectArray(vec.size(), seClazz, NULL);

    for( unsigned int i=0; i<vec.size(); i++ ) {
      // make a new scored extent result object
      jobject ser = jenv->NewObject(seClazz, seConstructor);

      // fill in the fields
      jenv->SetDoubleField(ser, scoreField, vec[i].score );
      jenv->SetIntField(ser, beginField, vec[i].begin );
      jenv->SetIntField(ser, endField, vec[i].end );
      jenv->SetIntField(ser, documentField, vec[i].document );

      // add this object to the array
      jenv->SetObjectArrayElement(array, i, ser);
    }

    // make a java string for this result list name
    jstring key = jenv->NewStringUTF(iter->first.c_str());
    // add the java array to the map
    jenv->CallObjectMethod($result, putMethod, key, array);
  }
}

%typemap(javaout) const indri::infnet::EvaluatorNode::MResults& {
  return $jnicall;
}

