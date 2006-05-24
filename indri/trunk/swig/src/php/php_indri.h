/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.29
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */

/*
  +----------------------------------------------------------------------+
  | PHP version 4.0                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors:                                                             |
  |                                                                      |
  +----------------------------------------------------------------------+
 */


#ifndef PHP_INDRI_H
#define PHP_INDRI_H

extern zend_module_entry indri_module_entry;
#define phpext_indri_ptr &indri_module_entry

#ifdef PHP_WIN32
# define PHP_INDRI_API __declspec(dllexport)
#else
# define PHP_INDRI_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(indri);
PHP_MSHUTDOWN_FUNCTION(indri);
PHP_RINIT_FUNCTION(indri);
PHP_RSHUTDOWN_FUNCTION(indri);
PHP_MINFO_FUNCTION(indri);

ZEND_NAMED_FUNCTION(_wrap_new_ScoredExtentResult);
ZEND_NAMED_FUNCTION(_wrap_new_ScoredExtentResult);
ZEND_NAMED_FUNCTION(_wrap_new_TermExtent);
ZEND_NAMED_FUNCTION(_wrap_new_TermExtent);
ZEND_NAMED_FUNCTION(_wrap_ParsedDocument_getContent);
ZEND_NAMED_FUNCTION(_wrap_ParsedDocument_getContent);
ZEND_NAMED_FUNCTION(_wrap_new_ParsedDocument);
ZEND_NAMED_FUNCTION(_wrap_new_ParsedDocument);
ZEND_NAMED_FUNCTION(_wrap_new_QueryAnnotationNode);
ZEND_NAMED_FUNCTION(_wrap_new_QueryAnnotationNode);
ZEND_NAMED_FUNCTION(_wrap_QueryAnnotation_getQueryTree);
ZEND_NAMED_FUNCTION(_wrap_QueryAnnotation_getQueryTree);
ZEND_NAMED_FUNCTION(_wrap_QueryAnnotation_getAnnotations);
ZEND_NAMED_FUNCTION(_wrap_QueryAnnotation_getAnnotations);
ZEND_NAMED_FUNCTION(_wrap_QueryAnnotation_getResults);
ZEND_NAMED_FUNCTION(_wrap_QueryAnnotation_getResults);
ZEND_NAMED_FUNCTION(_wrap_new_QueryAnnotation);
ZEND_NAMED_FUNCTION(_wrap_new_QueryAnnotation);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_addServer);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_addServer);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_addIndex);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_addIndex);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_close);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_close);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_setMemory);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_setMemory);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_setScoringRules);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_setScoringRules);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_setStopwords);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_setStopwords);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runQuery);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runQuery);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runQuerydocset);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runQuerydocset);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runAnnotatedQuery);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runAnnotatedQuery);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runAnnotatedQuerydocset);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_runAnnotatedQuerydocset);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentsdocids);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentsdocids);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documents);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documents);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentMetadatadocids);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentMetadatadocids);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentMetadata);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentMetadata);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_termCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_termCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_onetermCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_onetermCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_stemCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_stemCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_termFieldCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_termFieldCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_stemFieldCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_stemFieldCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_fieldList);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_fieldList);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_documentCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_onedocumentCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_onedocumentCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_expressionCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_expressionCount);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_expressionList);
ZEND_NAMED_FUNCTION(_wrap_QueryEnvironment_expressionList);
ZEND_NAMED_FUNCTION(_wrap_new_QueryEnvironment);
ZEND_NAMED_FUNCTION(_wrap_new_QueryEnvironment);
#endif /* PHP_INDRI_H */
