
//
// DocumentVector.i
//
// 16 August 2004 -- tds
//

%typemap(jni) indri::api::DocumentVector* "jobject"
%typemap(jtype) indri::api::DocumentVector* "DocumentVector"
%typemap(jstype) indri::api::DocumentVector* "DocumentVector"

%typemap(jni) std::vector<indri::api::DocumentVector*> "jobjectArray"
%typemap(jtype) std::vector<indri::api::DocumentVector*> "DocumentVector[]"
%typemap(jstype) std::vector<indri::api::DocumentVector*> "DocumentVector[]"

%{

jobject documentvector_copy( JNIEnv* jenv, indri::api::DocumentVector* vec ) {
  jobject result;

  jclass stringClazz = jenv->FindClass( "java/lang/String" );
  jclass docVectorClazz = jenv->FindClass( "edu/umass/cs/indri/DocumentVector" );
  jclass fieldClazz = jenv->FindClass( "edu/umass/cs/indri/DocumentVector$Field" );

  jfieldID beginField = jenv->GetFieldID( fieldClazz, "begin", "I" );
  jfieldID endField = jenv->GetFieldID( fieldClazz, "end", "I" );
  jfieldID numberField = jenv->GetFieldID( fieldClazz, "number", "J" );
  jfieldID nameField = jenv->GetFieldID( fieldClazz, "name", "Ljava/lang/String;" );

  jfieldID stemsField = jenv->GetFieldID( docVectorClazz, "stems", "[Ljava/lang/String;" );
  jfieldID positionsField = jenv->GetFieldID( docVectorClazz, "positions", "[I" );
  jfieldID fieldsField = jenv->GetFieldID( docVectorClazz, "fields", "[Ledu/umass/cs/indri/DocumentVector$Field;" );
  
  jmethodID fieldConstructor = jenv->GetMethodID( fieldClazz, "<init>", "()V" );
  jmethodID docVecConstructor = jenv->GetMethodID( docVectorClazz, "<init>", "()V" );

  int stemsCount = vec->stems().size();
  int positionsCount = vec->positions().size();
  int fieldsCount = vec->fields().size();
  
  // store positions
  jintArray posArray = jenv->NewIntArray( positionsCount );
  jint* posElements = jenv->GetIntArrayElements( posArray, 0 );

  for( int i=0; i<positionsCount; i++ ) {
    posElements[i] = vec->positions()[i];
  }

  jenv->ReleaseIntArrayElements( posArray, posElements, 0 );

  // store stems
  jobjectArray stemArray = jenv->NewObjectArray( stemsCount, stringClazz, 0 );

  for( int i=0; i<stemsCount; i++ ) {
    jstring s = jenv->NewStringUTF( vec->stems()[i].c_str() );
    jenv->SetObjectArrayElement( stemArray, i, s );
  }
  
  // store fields
  jobjectArray fieldsArray = jenv->NewObjectArray( fieldsCount, fieldClazz, 0 );
  
  for( int i=0; i<fieldsCount; i++ ) {
    // make a field object
    jobject f = jenv->NewObject( fieldClazz, fieldConstructor );
    jstring name = jenv->NewStringUTF( vec->fields()[i].name.c_str() );
    
    jenv->SetIntField( f, beginField, vec->fields()[i].begin );
    jenv->SetIntField( f, endField, vec->fields()[i].end );
    jenv->SetLongField( f, numberField, vec->fields()[i].number );
    jenv->SetObjectField( f, nameField, name );
  
    // put it in the array
    jenv->SetObjectArrayElement( fieldsArray, i, f );
  }
// don't delete this twice.
//  delete vec;
  
  // build the document vector object
  result = jenv->NewObject( docVectorClazz, docVecConstructor );

  // store fields
  jenv->SetObjectField( result, stemsField, stemArray );
  jenv->SetObjectField( result, positionsField, posArray );
  jenv->SetObjectField( result, fieldsField, fieldsArray );

  return result;
}

%}

%typemap(java,out) indri::api::DocumentVector* {
// this typemap is currently unused, but should delete
// the input to be consistent with the vector version
  jobject vec = documentvector_copy( jenv, $1 );
  delete $1;
  $result = vec;
}

%typemap(javaout) DocumentVector* {
  return $jnicall;
}

%typemap(java,out) std::vector<indri::api::DocumentVector*> {
  jclass docVecClazz = jenv->FindClass( "edu/umass/cs/indri/DocumentVector" );
  $result = jenv->NewObjectArray($1.size(), docVecClazz, NULL);

  for( unsigned int i=0; i<$1.size(); i++ ) {
    jobject vec = documentvector_copy( jenv, $1[i] );
    jenv->SetObjectArrayElement($result, i, vec);
    delete $1[i];
  }
}

%typemap(javaout) std::vector<indri::api::DocumentVector*> {
  return $jnicall;
}

