//
// ogrdatasource.cc
//
//
// Copyright (c) 2011, JF Gigand
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>
#include "php.h"
#include "php_gdal.h"
#include "ogrdatasource.h"
#include "ogrsfdriver.h"
#include "ogrlayer.h"
#include "ogrspatialreference.h"
#include "debug.h"

zend_class_entry *gdal_ogrdatasource_ce;
zend_object_handlers ogrdatasource_object_handlers;

void php_gdal_ogrdatasource_add_to_hash(php_ogrdatasource_object *obj)
{
  /*
  HashPosition pos;

  DEBUG_LOG_FUNCTION

  zend_hash_next_index_insert(&GDAL_G(ogrDataSources),
                              &(obj->datasource),
                              sizeof(obj->datasource), NULL);
  zend_hash_internal_pointer_end_ex(&GDAL_G(ogrDataSources), &pos);
  if (zend_hash_get_current_key_ex(&GDAL_G(ogrDataSources), NULL, NULL,
                                   &obj->hashIndex, 0, &pos)
      != HASH_KEY_IS_LONG) {
    php_log_err("php5-gdal: failed to get hash index for datasource");
  }

  DEBUG_LOG("php5-gdal: got OGR datasource hash index: %d",
          (int)obj->hashIndex);
  */
}

void php_gdal_ogrdatasource_release(php_ogrdatasource_object *obj)
{

  DEBUG_LOG_FUNCTION

  if (obj != NULL) {
    if (obj->datasource != NULL) {
      DEBUG_LOG("Attempting to Release OGRDataSource %x php_ogrdatasource_object PHP obj %x", obj->datasource, obj);
      DEBUG_LOG("OGRDataSource %x currently has %d references",
          obj->datasource, obj->datasource->GetRefCount());
      if (obj->datasource->Dereference() < 1) {
        DEBUG_LOG("Releasing OGRDataSource %x", obj->datasource);
        OGRDataSource::DestroyDataSource(obj->datasource);
        efree(obj);
      }
    }
    else {
      efree(obj);
    }
  }
}


void php_gdal_ogrdatasource_ptr_dtor(void **ptr)
{
  OGRDataSource *datasource = (OGRDataSource *)*ptr;

  DEBUG_LOG_FUNCTION

  if (datasource) {
    DEBUG_LOG("Releasing OGRDataSource %x in dtor", datasource);
    datasource->Release();
  }
}

//
// PHP stuff
//

void ogrdatasource_free_storage(void *object TSRMLS_DC)
{
  char *msg;
  int i, i2;

  DEBUG_LOG_FUNCTION

  php_ogrdatasource_object *obj = (php_ogrdatasource_object *)object;
  zend_hash_destroy(obj->std.properties);
  FREE_HASHTABLE(obj->std.properties);
  php_gdal_ogrdatasource_release(obj);
}

zend_object_value ogrdatasource_create_handler(zend_class_entry *type TSRMLS_DC)
{
  zval *tmp;
  zend_object_value retval;

  DEBUG_LOG_FUNCTION

  php_ogrdatasource_object *obj =
    (php_ogrdatasource_object *)emalloc(sizeof(php_ogrdatasource_object));
  memset(obj, 0, sizeof(php_ogrdatasource_object));
  obj->std.ce = type;
 
  DEBUG_LOG("Creating new php_ogrdatasource_object %x with OGRDatasource %x", obj, obj->datasource);

  ALLOC_HASHTABLE(obj->std.properties);
  zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID < 50399
  zend_hash_copy(obj->std.properties, &type->default_properties,
                 (copy_ctor_func_t)zval_add_ref,
                 (void *)&tmp, sizeof(zval *));
#else
  object_properties_init(&obj->std, type);
#endif

  retval.handle =
    zend_objects_store_put(obj, NULL,
                           ogrdatasource_free_storage, NULL TSRMLS_CC);
  retval.handlers = &ogrdatasource_object_handlers;

  return retval;

  //pdo_stmt_construct(stmt, return_value, dbstmt_ce, ctor_args TSRMLS_CC);

}

//
// CLASS METHODS
//

PHP_METHOD(OGRDataSource, GetName)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  const char *name = datasource->GetName();
  if (name) {
    RETURN_STRING((char *)name, 1);
  } else {
    RETURN_NULL();
  }
}

PHP_METHOD(OGRDataSource, GetLayerCount)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_LONG(datasource->GetLayerCount());
}

PHP_METHOD(OGRDataSource, GetLayer)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  OGRLayer *layer;
  php_ogrlayer_object *layer_obj;
  long index;

  DEBUG_LOG_FUNCTION

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"l",
                            &index) == FAILURE) {
    return;
  }
  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;

  layer = datasource->GetLayer(index);

  if (!layer) {
    RETURN_NULL();
  }

  if (object_init_ex(return_value, gdal_ogrlayer_ce) != SUCCESS) {
    RETURN_NULL();
  }
  layer_obj = (php_ogrlayer_object*)
    zend_object_store_get_object(return_value TSRMLS_CC);
  layer_obj->layer = layer;

  // increment the refcount on the datasource
  datasource->Reference();
  layer_obj->datasource_obj = obj;
}

PHP_METHOD(OGRDataSource, GetLayerByName)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  OGRLayer *layer;
  php_ogrlayer_object *layer_obj;
  char *name;
  int name_len;

  DEBUG_LOG_FUNCTION

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"s",
                            &name, &name_len) == FAILURE) {
    return;
  }
  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;

  int i = datasource->GetRefCount();
  layer = datasource->GetLayerByName(name);
  if (!layer) {
    RETURN_NULL();
  }

  if (object_init_ex(return_value, gdal_ogrlayer_ce) != SUCCESS) {
    RETURN_NULL();
  }
  layer_obj = (php_ogrlayer_object*)
    zend_object_store_get_object
    //zend_objects_get_address
    (return_value TSRMLS_CC);
  layer_obj->layer = layer;

  // increment the refcount on the datasource
  datasource->Reference();
  layer_obj->datasource_obj = obj;
}

PHP_METHOD(OGRDataSource, DeleteLayer)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  long index;

  DEBUG_LOG_FUNCTION

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"l",
                            &index) == FAILURE) {
    return;
  }
  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;

  RETURN_LONG(datasource->DeleteLayer(index));
}

PHP_METHOD(OGRDataSource, TestCapability)
{
  // Available capabilities as of 1.7.3 (ogr_core.h):
  //    "CreateLayer", "DeleteLayer"
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  char *strcapability = NULL;
  int strcapability_len;

  DEBUG_LOG_FUNCTION

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"s",
                            &strcapability, &strcapability_len) == FAILURE) {
    return;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_BOOL(datasource->TestCapability(strcapability));
}

PHP_METHOD(OGRDataSource, CreateLayer)
{
  // Available capabilities as of 1.7.3 (ogr_core.h):
  //    "CreateLayer", "DeleteLayer"
  char *name = NULL;
  int name_len;
  OGRwkbGeometryType gtype = wkbUnknown;
  zval *spatialrefp = NULL;
  php_ogrspatialreference_object *spatialref_obj;
  OGRSpatialReference *spatialref = NULL;
  php_ogrdatasource_object *obj;
  OGRDataSource *datasource;
  OGRLayer *layer;
  php_ogrlayer_object *layer_obj;

  DEBUG_LOG_FUNCTION

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"s|O!l",
                            &name, &name_len, &spatialrefp,
                            gdal_ogrspatialreference_ce, &gtype) == FAILURE) {
    return;
  }

  if (spatialrefp) {
    spatialref_obj = (php_ogrspatialreference_object *)
      zend_object_store_get_object(spatialrefp);
    spatialref = spatialref_obj->spatialreference;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;

  layer = datasource->CreateLayer(name, spatialref, gtype, NULL /* TODO: pass options */);
  if (!layer) {
    RETURN_NULL();
  }
  if (object_init_ex(return_value, gdal_ogrlayer_ce) != SUCCESS) {
    RETURN_NULL();
  }
  layer_obj = (php_ogrlayer_object*)
    zend_object_store_get_object(return_value TSRMLS_CC);
  layer_obj->layer = layer;

  // increment the refcount on the datasource
  datasource->Reference();
  layer_obj->datasource_obj = obj;

}

PHP_METHOD(OGRDataSource, SyncToDisk)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;
  OGRErr error;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  error = datasource->SyncToDisk();
  RETURN_LONG(error);
}

PHP_METHOD(OGRDataSource, GetDriver)
{
  OGRDataSource *datasource;
  OGRSFDriver *driver;
  php_ogrdatasource_object *obj;
  driver_object *driver_obj;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  driver = datasource->GetDriver();
  if (!driver) {
    RETURN_NULL();
  }
  if (object_init_ex(return_value, gdal_ogrsfdriver_ce) != SUCCESS) {
    RETURN_NULL();
  }
  driver_obj = (driver_object*)
    zend_object_store_get_object(return_value TSRMLS_CC);
  driver_obj->driver = driver;
}

PHP_METHOD(OGRDataSource, Reference)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_LONG(datasource->Reference());
}

PHP_METHOD(OGRDataSource, Dereference)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_LONG(datasource->Dereference());
}

PHP_METHOD(OGRDataSource, GetRefCount)
{
  OGRDataSource *datasource;
  php_ogrdatasource_object *obj;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);
  datasource = obj->datasource;
  RETURN_LONG(datasource->GetRefCount());
}

PHP_METHOD(OGRDataSource, Close)
{
  php_ogrdatasource_object *obj;
  zval *p;

  DEBUG_LOG_FUNCTION

  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  obj = (php_ogrdatasource_object *)
    zend_object_store_get_object(getThis() TSRMLS_CC);

  php_gdal_ogrdatasource_release(obj);
}

PHP_METHOD(OGRDataSource, DestroyDataSource)
{
  php_ogrdatasource_object *obj;
  zval *p;

  DEBUG_LOG_FUNCTION

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char*)"O",
                            &p, gdal_ogrdatasource_ce) == FAILURE) {
    return;
  }

  obj = (php_ogrdatasource_object *)zend_object_store_get_object(p);
  php_gdal_ogrdatasource_release(obj);
}


//
// PHP stuff
//

zend_function_entry ogrdatasource_methods[] = {
  PHP_ME(OGRDataSource, GetName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetLayerCount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetLayer, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetLayerByName, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, DeleteLayer, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, TestCapability, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, CreateLayer, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, CopyLayer, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, GetStyleTable, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, SetStyleTableDirectly, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, SetStyleTable, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, ExecuteSQL, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(OGRDataSource, ReleaseResultSet, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, SyncToDisk, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetDriver, NULL, ZEND_ACC_PUBLIC)
  //PHP_ME(OGRDataSource, SetDriver, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, Reference, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, Dereference, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, GetRefCount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(OGRDataSource, Close, NULL, ZEND_ACC_PUBLIC) // extra
  PHP_ME(OGRDataSource, DestroyDataSource, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
  {NULL, NULL, NULL}
};

void php_gdal_ogrdatasource_startup(INIT_FUNC_ARGS)
{
  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "OGRDataSource", ogrdatasource_methods);
  gdal_ogrdatasource_ce = zend_register_internal_class(&ce TSRMLS_CC);
  gdal_ogrdatasource_ce->create_object = ogrdatasource_create_handler;
  memcpy(&ogrdatasource_object_handlers,
         zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  ogrdatasource_object_handlers.clone_obj = NULL;
}
