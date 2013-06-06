--TEST--
Test: Version number
--FILE--
<?php
if (function_exists("gdalextversioninfo")) {
    echo "gdalextversioninfo found\n";
    $version = gdalextversioninfo();
    if (!is_null($version)) {
        echo "gdalextversioninfo: version string returned";
    }
    else {
        echo "gdalextversioninfo: no version returned";
    }
}
?>
--EXPECT--
gdalextversioninfo found
gdalextversioninfo: version string returned
