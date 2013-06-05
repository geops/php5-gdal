//
// ogrdatasource.h
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

#ifndef PHP_OGRDATASOURCE_H
#define PHP_OGRDATASOURCE_H

#include <ogrsf_frmts.h>
#include "php.h"

extern zend_class_entry *gdal_ogrdatasource_ce;

struct php_ogrdatasource_object {
  zend_object std;
  OGRDataSource *datasource;
  ulong hashIndex;
};

void php_gdal_ogrdatasource_release(php_ogrdatasource_object *);
void php_gdal_ogrdatasource_startup(INIT_FUNC_ARGS);
void php_gdal_ogrdatasource_add_to_hash(php_ogrdatasource_object *obj);
void php_gdal_ogrdatasource_ptr_dtor(void **ptr);
//void ogrdatasource_destroy_all();

#endif /* PHP_OGRDATASOURCE_H */
