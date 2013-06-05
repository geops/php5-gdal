//
// ogrgeometry.h
//

#ifndef PHP_OGRGEOMETRY_H
#define PHP_OGRGEOMETRY_H

#include <ogr_geometry.h>
#include "php.h"

extern zend_class_entry *gdal_ogrgeometry_ce;

struct php_ogrgeometry_object {
  zend_object std;
  OGRGeometry *geometry;
  bool is_reference;
};

void php_gdal_ogrgeometry_release(php_ogrgeometry_object *);
void php_gdal_ogrgeometry_startup(INIT_FUNC_ARGS);

#endif /* PHP_OGRGEOMETRY_H */
