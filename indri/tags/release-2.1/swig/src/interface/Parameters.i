
//
// Parameters.i
//
// 13 August 2004 -- tds
//

%typemap(jni) indri::api::Parameters "jobject"
%typemap(jtype) indri::api::Parameters "Map"
%typemap(jstype) indri::api::Parameters "Map"

%typemap(jni) indri::api::Parameters* "jobject"
%typemap(jtype) indri::api::Parameters* "Map"
%typemap(jstype) indri::api::Parameters* "Map"

%{

struct jni_parameters_info {
  jclass stringClazz;
  jclass mapClazz;
  jclass setClazz;
  jclass arrayOfMaps;
  jclass arrayOfString;
};

void java_parameters_map( JNIEnv* jenv, jni_parameters_info& info, indri::api::Parameters p, jobject obj );
void java_parameters_array_of_maps( JNIEnv* jenv, jni_parameters_info& info, indri::api::Parameters p, const std::string& key, jobjectArray array );

void java_parameters_init( JNIEnv* jenv, jni_parameters_info& info ) {
  info.stringClazz = jenv->FindClass("java/lang/String");
  info.mapClazz = jenv->FindClass("java/util/Map");
  info.setClazz = jenv->FindClass("java/util/Set");
  info.arrayOfMaps = jenv->FindClass("[Ljava/util/Map;");
  info.arrayOfString = jenv->FindClass("[Ljava/lang/String;");
}

void java_parameters_array_of_strings( JNIEnv* jenv, jni_parameters_info& info, indri::api::Parameters p, const std::string& key, jobjectArray array ) {
  // get the array size
  jsize arrayLength = jenv->GetArrayLength(array);

  for( int i=0; i<arrayLength; i++ ) {
    jstring s = (jstring) jenv->GetObjectArrayElement( array, i );
    
    const char* valueBytes = jenv->GetStringUTFChars(s, 0);
    std::string valueString = (const char*) valueBytes;
    jenv->ReleaseStringUTFChars(s, valueBytes);
    
    p.append(key).set(valueString);
  }
}

void java_parameters_array_of_maps( JNIEnv* jenv, jni_parameters_info& info, indri::api::Parameters p, const std::string& key, jobjectArray array ) {
  // get the array size
  jsize arrayLength = jenv->GetArrayLength(array);

  for( int i=0; i<arrayLength; i++ ) {
    jobject obj = jenv->GetObjectArrayElement( array, i );
    java_parameters_map( jenv, info, p, obj );
  }
}

void java_parameters_map( JNIEnv* jenv, jni_parameters_info& info, indri::api::Parameters p, jobject mapObj ) {
  // get map class and entrySet method pointer
  jclass mapClazz = jenv->GetObjectClass(mapObj);
  jmethodID mapEntrySet = jenv->GetMethodID(mapClazz, "entrySet", "()Ljava/util/Set;" );

  // call entry set function to set a Set of entries
  jobject entrySet = jenv->CallObjectMethod(mapObj, mapEntrySet);
  jclass setClazz = jenv->GetObjectClass(entrySet);
  jmethodID setToArray = jenv->GetMethodID(setClazz, "toArray", "()[Ljava/lang/Object;" );

  // turn that set into an array of objects (entries)
  jobjectArray entryArray = (jobjectArray) jenv->CallObjectMethod(entrySet, setToArray);

  // get the array size
  jsize entryArrayLength = jenv->GetArrayLength(entryArray);

  for( int i=0; i<entryArrayLength; i++ ) {
    // get the key string
    jobject entryObject = (jstring) jenv->GetObjectArrayElement( entryArray, i );
    jclass mapEntryClazz = jenv->GetObjectClass(entryObject);
    jmethodID mapEntryGetKey = jenv->GetMethodID(mapEntryClazz, "getKey", "()Ljava/lang/Object;" );
    jmethodID mapEntryGetValue = jenv->GetMethodID(mapEntryClazz, "getValue", "()Ljava/lang/Object;" );

    // get key string
    jstring key = (jstring) jenv->CallObjectMethod(entryObject, mapEntryGetKey);
    const char* bytes = jenv->GetStringUTFChars(key, 0);
    std::string keyString = (const char*) bytes;
    jenv->ReleaseStringUTFChars(key, bytes);
    
    // get value object
    jobject value = jenv->CallObjectMethod(entryObject, mapEntryGetValue);
    
    // figure out object type
    if( jenv->IsInstanceOf( value, info.stringClazz ) ) {
      const char* valueBytes = jenv->GetStringUTFChars( (jstring)value, 0);
      std::string valueString = (const char*) valueBytes;
      jenv->ReleaseStringUTFChars( (jstring)value, valueBytes);
      
      p.set( keyString, valueString );
    } else if( jenv->IsInstanceOf( value, info.mapClazz ) ) {
      indri::api::Parameters sub = p.append( keyString );
      java_parameters_map( jenv, info, p, value );
    } else if( jenv->IsInstanceOf( value, info.arrayOfMaps ) ) {
      java_parameters_array_of_maps( jenv, info, p, keyString, (jobjectArray) value );
    } else if( jenv->IsInstanceOf( value, info.arrayOfString ) ) {
      java_parameters_array_of_strings( jenv, info, p, keyString, (jobjectArray) value );
    } else {
      SWIG_exception(, SWIG_RuntimeError, "Found something in a Parameters parameter that wasn't a String, Map, String[] or Map[]." );
    }
  }
}

%}

%typemap(java,in) indri::api::Parameters {
  jni_parameters_info info;
  java_parameters_init( jenv, info );
  
  java_parameters_map( jenv, info, $1, $input );
}

%typemap(java,in) indri::api::Parameters* ( indri::api::Parameters p ) {
  if( $input != 0 ) {
    jni_parameters_info info;
    java_parameters_init( jenv, info );
  
    java_parameters_map( jenv, info, p, $input );
    $1 = &p;
  } else {
    $1 = 0;
  }
}
  
%typemap(javain) indri::api::Parameters "$javainput";
%typemap(javain) indri::api::Parameters* "$javainput";

%typemap(javaimports) indri::api::Parameters "import java.util.Map;";
%typemap(javaimports) indri::api::Parameters* "import java.util.Map;";


