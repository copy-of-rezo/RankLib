
//
// ParsedDocument.i
//
// 10 August 2004 -- tds
//

%typemap(jni) indri::api::ParsedDocument* "jobject"
%typemap(jtype) indri::api::ParsedDocument* "ParsedDocument"
%typemap(jstype) indri::api::ParsedDocument* "ParsedDocument"

%typemap(jni) std::vector<indri::api::ParsedDocument*> "jobjectArray"
%typemap(jtype) std::vector<indri::api::ParsedDocument*> "ParsedDocument[]"
%typemap(jstype) std::vector<indri::api::ParsedDocument*> "ParsedDocument[]"

%{

struct jni_parseddocument_info {
  jclass pdClazz;
  jmethodID pdConstructor;

  jclass stringClazz;
  jclass byteArrayClazz;

  jclass mapClazz;
  jmethodID mapConstructor;
  jmethodID putMethod;

  jclass teClazz;
  jmethodID teConstructor;

  jfieldID termsField;
  jfieldID textField;
  jfieldID positionsField;
  jfieldID metadataField;

  jfieldID beginField;
  jfieldID endField;
};

void parseddocument_init( JNIEnv* jenv, jni_parseddocument_info& info ) {
  info.pdClazz = jenv->FindClass("edu/umass/cs/indri/ParsedDocument");
  info.pdConstructor = jenv->GetMethodID(info.pdClazz, "<init>", "()V" );

  info.stringClazz = jenv->FindClass("java/lang/String" );
  info.byteArrayClazz = jenv->FindClass("[B" );

  info.mapClazz = jenv->FindClass("java/util/HashMap");
  info.mapConstructor = jenv->GetMethodID(info.mapClazz, "<init>", "()V" );

  info.putMethod = jenv->GetMethodID(info.mapClazz, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );
  info.teClazz = jenv->FindClass("edu/umass/cs/indri/ParsedDocument$TermExtent" );
  info.teConstructor = jenv->GetMethodID(info.teClazz, "<init>", "(II)V" );

  info.textField = jenv->GetFieldID(info.pdClazz, "text", "Ljava/lang/String;" );
  info.termsField = jenv->GetFieldID(info.pdClazz, "terms", "[Ljava/lang/String;" );
  info.positionsField = jenv->GetFieldID(info.pdClazz, "positions", "[Ledu/umass/cs/indri/ParsedDocument$TermExtent;" );
  info.metadataField = jenv->GetFieldID(info.pdClazz, "metadata", "Ljava/util/Map;" );

  info.beginField = jenv->GetFieldID(info.teClazz, "begin", "I");
  info.endField = jenv->GetFieldID(info.teClazz, "end", "I");
}

jobject parseddocument_copy( JNIEnv* jenv, jni_parseddocument_info& info, indri::api::ParsedDocument* doc ) {
  // make a parsed document
  jobject result = jenv->NewObject(info.pdClazz, info.pdConstructor);

  // make a metadata map to go in it
  jobject mapObject = jenv->NewObject(info.mapClazz, info.mapConstructor);

  // copy metadata information
  for( unsigned int i=0; i<doc->metadata.size(); i++ ) {
    indri::parse::MetadataPair& pair = doc->metadata[i];

    jstring key = jenv->NewStringUTF(pair.key);
    jbyteArray value = jenv->NewByteArray(pair.valueLength);

    jbyte* elements = jenv->GetByteArrayElements(value, 0);
    memcpy( elements, pair.value, pair.valueLength );
    jenv->ReleaseByteArrayElements(value, elements, 0);

    // put it in the map
    jenv->CallObjectMethod(mapObject, info.putMethod, key, value);
  }

  // make a terms string array
  jobjectArray termsArray = jenv->NewObjectArray(doc->terms.size(), info.stringClazz, NULL);

  // copy terms information
  for( unsigned int i=0; i<doc->terms.size(); i++ ) {
    jstring term = jenv->NewStringUTF(doc->terms[i]);
    jenv->SetObjectArrayElement(termsArray, i, term);
  }

  // make a positions array
  jobjectArray positionsArray = jenv->NewObjectArray(doc->positions.size(), info.teClazz, NULL);

  // copy positions information
  for( unsigned int i=0; i<doc->positions.size(); i++ ) {
    int begin = doc->positions[i].begin;
    int end = doc->positions[i].end;
    jobject position = jenv->NewObject(info.teClazz, info.teConstructor, begin, end);

    // add this object to the array
    jenv->SetObjectArrayElement(positionsArray, i, position);
  }

  // store field data
  jstring text = jenv->NewStringUTF(doc->text);

  jenv->SetObjectField(result, info.textField, text);
  jenv->SetObjectField(result, info.termsField, termsArray);
  jenv->SetObjectField(result, info.positionsField, positionsArray);
  jenv->SetObjectField(result, info.metadataField, mapObject);

  return result;
}

%}

%typemap(java,in) const indri::api::ParsedDocument& ( indri::api::ParsedDocument pdoc, indri::utility::Buffer buf ) {
  jni_parseddocument_info info;

  parseddocument_init( jenv, info );
  $1 = &pdoc;

  jstring text = (jstring) jenv->GetObjectField($input, info.textField);
  jobjectArray terms = (jobjectArray) jenv->GetObjectField($input, info.termsField);
  jobjectArray positions = (jobjectArray) jenv->GetObjectField($input, info.positionsField);
  jobjectArray metadata = (jobjectArray) jenv->GetObjectField($input, info.metadataField);

  // store text
  const char* textString = jenv->GetStringUTFChars(text, 0);
  jsize textLength = jenv->GetStringUTFLength(text);
  strcpy( buf.write(textLength+1), textString );
  jenv->ReleaseStringUTFChars(text, textString, 0);

  // store terms
  std::vector<int> termPositions;
  jsize termCount = terms ? jenv->GetArrayLength(terms) : 0;
  for( int i=0; i<termCount; i++ ) {
    // get term string
    jstring term = jenv->GetObjectArrayElement(terms, i);

    if( term != 0 ) {
      termPositions.push_back( buf.position() );
      const char* termString = jenv->GetStringUTFChars(text, 0);
      jsize termStringLength = jenv->GetStringUTFLength(term);
      strcpy( buf.write(termStringLength+1), textString );
      jenv->ReleaseStringUTFChars(term, termString, 0);
    } else {
      termPositions.push_back(-1);
    }
  }

  // store positions (straight to structure, no buffer necessary)
  jsize positionsCount = positions ? jenv->GetArrayLength(positions) : 0;
  for( int i=0; i<positionsCount; i++ ) {
    jobject position = jenv->GetObjectArrayElement(positions, i);
    TagExtent extent;
    extent.begin = jenv->GetIntField(position, info.beginField);
    extent.end = jenv->GetIntField(position, info.endField);
    pdoc.positions.push_back(extent);
  }

  // store metadata
  jclass mapClazz = jenv->GetObjectClass(metadata);
  jmethodID mapEntrySet = jenv->GetMethodID(mapClazz, "entrySet", "()Ljava/util/Set;" );
  jobject entrySet = jenv->CallObjectMethod(metadata, mapEntrySet);
  jmethodID toArray = jenv->GetMethodID(entrySet, "toArray", "()[Ljava/lang/Object;" );
  jobjectArray metadataArray = (jobjectArray) jenv->CallObjectMethod(entrySet, toArray);

  jsize metadataCount = metadata ? jenv->GetArrayLength(metadata) : 0;
  std::vector<int> metadataKeyPositions;
  std::vector<int> metadataValuePositions;
  std::vector<int> metadataValueLengths;
  for( int i=0; i<metadataCount; i++ ) {
    jobject md = jenv->GetObjectArrayElement(metadata, i);
    jclass entryClazz = jenv->GetObjectClass(md);
    jmethodID mapEntryKey = jenv->GetMethodID(entryClazz, "getKey", "()Ljava/lang/Object;" );
    jmethodID mapEntryValue = jenv->GetMethodID(entryClazz, "getValue", "()Ljava/lang/Object;" );

    jstring key = (jstring) jenv->CallObjectMethod(md, mapEntryKey);
    jobject value = jenv->CallObjectMethod(md, mapEntryValue);

    metadataValuePositions.push_back( buf.position() );
    if( jenv->IsInstanceOf(value, info.byteArrayClazz) ) {
      // value is raw bytes
      const jbyte* bytes = jenv->GetByteArrayElements( (jbyteArray) value, 0 );
      jsize bytesLength = jenv->GetArrayLength((jbyteArray)value);
      memcpy( buf.write(bytesLength), bytes, bytesLength );
      jenv->ReleaseByteArrayElements( value, bytes, 0 );
      metadataValueLengths.push_back( buf.position() );
    } else if( jenv->IsInstanceOf(value, info.stringClazz) ) {
      const char* stringChars = jenv->GetStringUTFChars((jstring)value, 0);
      jsize valueStringLength = jenv->GetStringUTFLength((jstring)value);
      metadataValueLengths.push_back(valueStringLength+1);
      strcpy( buf.write(valueStringLength+1), stringChars, valueStringLength+1 );
      jenv->ReleaseStringUTFChars((jstring)value, stringChars, 0);
    }

    metadataKeyPositions.push_back(buf.position());
    const char* keyChars = jenv->GetStringUTFChars(key, 0);
    int keyLength = jenv->GetStringUTFLength(key, 0);
    strcpy( buf.write(keyLength+1), keyChars, keyLength+1 );
    jenv->ReleaseStringUTFChars((jstring)key, keyChars, 0);
  }

  // now, copy term pointers into arrays
  for( int i=0; i<termPositions.size(); i++ ) {
    pdoc.terms.push_back( buf.front() + termPositions[i] );
  }

  // copy metadata pointers
  for( int i=0; i<metadataKeyPositions.size(); i++ ) {
    const char* key = buf.front() + metadataKeyPositions[i];
    const char* value = buf.front() + metadataValuePositions[i];
    int valueLength = metadataValueLengths[i];
    pdoc.metadata.push_back( MetadataPair( key, value, valueLength ) );
  }

  // copy text
  pdoc.text = buf.front();
  pdoc.textLength = textLength;
}

%typemap(java,out) indri::api::ParsedDocument* {
  jni_parseddocument_info info;

  parseddocument_init( jenv, info );
  $result = parseddocument_copy( jenv, info, $1 );
  delete $1;
}

%typemap(java,out) std::vector<indri::api::ParsedDocument*> {
  jni_parseddocument_info info;
  parseddocument_init( jenv, info );
  
  $result = jenv->NewObjectArray($1.size(), info.pdClazz, NULL);

  for( unsigned int i=0; i<$1.size(); i++ ) {
    jobject document = parseddocument_copy( jenv, info, $1[i] );
    jenv->SetObjectArrayElement($result, i, document);
    delete $1[i];
  }
}

%typemap(javain) indri::api::ParsedDocument* "$javainput";

%typemap(javaout) indri::api::ParsedDocument* {
  return $jnicall;
}

%typemap(javaout) std::vector<indri::api::ParsedDocument*> {
  return $jnicall;
}

