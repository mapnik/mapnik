# Mapnik Changelog

A simple log of core changes affecting Mapnik usage.

Developers: Please commit along with changes.

For a complete change history, see the git log.

## 2.3.0

Released ...

(Packaged from ...)

Summary: TODO

- GDAL plugin: Added back support for user driven `nodata` on rgb(a) images (#2023)

- GDAL plugin: Allowed nodata to override alpha band if set on rgba images (#2023)

- GDAL plugin: Added `nodata_tolerance` option to set nearby pixels transparent (has similar effect to the `nearblack` program) (#2023)

- Added support for web fonts: .woff format (#2113)

- Added missing support for `geometry-transform` in `line-pattern` and `polygon-pattern` symbolizers (#2065)

- Dropped support for Sun compiler

- Upgraded unifont to `unifont-6.3.20131020`

- CSV Plugin: added the ability to pass an `extent` in options

- Fixed crash when rendering to cairo context from python (#2031)

- Moved `label-position-tolerance` from unsigned type to double

- Default PNG encoding method when `png` is supplied is now `png8:m=h`, so paletted png using hextree color quantization (#2028)
  Use `png32` now for full color png. More details at https://github.com/mapnik/mapnik/wiki/Image-IO.

- Added support for more seamless blurring by rendering to a larger internal image to avoid edge effects (#1478)

- Fixed rendering of large shapes at high zoom levels, which might dissapear due to integer overflow. This
  bug was previously fixable when geometries were clipped, but would, until now, re-appear if clipping was turned
  off for a symbolizer (#2000)

- Added single color argument support to `colorize-alpha` to allow colorizing alpha with one color.

- Added `color-to-alpha` `image-filter` to allow for applying alpha in proportion to color similiarity (#2023)

- Added Async support to PostGIS plugin - https://github.com/mapnik/mapnik/wiki/PostGIS-Async

- Fixed alpha handling bug with `comp-op:dst-over` (#1995)

- Fixed alpha handling bug with building-fill-opacity (#2011)

- Optimized mapnik.Path.to_wkb

- Python: added `__geo_interface__` to mapnik.Feature and mapnik.Path (#2009)

- Python: Exposed optimized WKTReader for parsing WKT into geometry paths (6bfbb53)

- Optimized expression evaluation of text by avoiding extra copy (1dd1275)

- Added Map level `background-image-comp-op` to control the compositing operation used to blend the 
`background-image` onto the `background-color`. Has no meaning if `background-color` or `background-image`
are not set. (#1966)

- Added Map level `background-image-opacity` to dynamically set the opacity of the `background-image` (#1966)

- Removed `RENDERING_STATS` compile option since it should be replaced with a better solution (#1956)

- Added support to experimental `svg_renderer` for grouping layers for inkscape and illustrator (#1917)

- Fixed compile of python bindings against Python 3.x

- Optimized SVG loading by improving color parsing speed (#1918)

- Fixed startup problem when fonts cannot be read due to lacking permissions (#1919)

- Fixed bad behavior when negative image dimensions are requested (#1927)

- Fixed handling of `marker-ignore-placement:true` when `marker-placement:line` (#1931)

- Fixed handling of svg `opacity` in Cairo renderer (#1943)

- Fixed handling of SVG files which contain empty `<g>` (#1944)

- Fixed various 32bit test failures

- Fixed compile against icu when by using `U_NAMESPACE_QUALIFIER`

- Fixed missing support for using PathExpression in `marker-file` (#1952)

- Added support for `line-pattern-offset` (#1991)

- Added support for building on Android (tested with `android-ndk-r9`)

- Added support for compiling with both -ansi (aka -std=c++98) and -std=c++11

- Added support for compiling and linking on OS X against libc++

- Fixed regression in handling `F` type dbf fields, introduced in v2.2.0.

- Added the ability to create a mapnik Feature from a geojson feature with `mapnik.Feature.from_geojson` in python.

- Added to python bindings: `has_tiff`, `has_png`, `has_webp`, `has_proj4`, `has_svg_renderer`, and `has_grid_renderer`

- Made it possible to disable compilation of `grid_renderer` with `./configure GRID_RENDERER=False` (#1962)

- Added `premultiplied` property on mapnik::image_32 / mapnik.Image to enable knowledge of premultiplied status of image buffer.

- Added `webp` image encoding and decoding support (#1955)

- Added `scale-hsla` image-filter that allows scaling colors in HSL color space. RGB is converted to HSL (hue-saturation-lightness) and then each value (and the original alpha value) is stretched based on the specified scaling values. An example syntax is `scale-hsla(0,1,0,1,0,1,0,1)` which means no change because the full range will be kept (0 for lowest, 1 for highest). Other examples are: 1) `scale-hsla(0,0,0,1,0,1,0,1)` which would force all colors to be red in hue in the same way `scale-hsla(1,1,0,1,0,1,0,1)` would, 2) `scale-hsla(0,1,1,1,0,1,0,1)` which would cause all colors to become fully saturated, 3) `scale-hsla(0,1,1,1,0,1,.5,1)` which would force no colors to be any more transparent than half, and 4) `scale-hsla(0,1,1,1,0,1,0,.5)` which would force all colors to be at least half transparent. (#1954)

## 2.2.0

Released June 3rd, 2013

(Packaged from 9231205)

Summary: The 2.2.0 release is primarily a performance and stability release. The code line represents development in the master branch since the release of 2.1.0 in Aug 2012 and therefore includes nearly a year of bug-fixes and optimizations. Nearly 500 new tests have been added bring the total coverage to 925. Shapefile and PostGIS datasources have benefited from numerous stability fixes, 64 bit integer support has been added to support OSM data in the grid renderer and in attribute filtering, and many fixes have landed for higher quality output when using a custom `scale_factor` during rendering. Critical code paths have been optimized include raster rendering, xml map loading, string to number conversion, vector reprojection when using `epsg:4326` and `epsg:3857`, `hextree` encoding, halo rendering, and rendering when using a custom `gamma`. Mapnik 2.2 also compiles faster than previous releases in the 2.x series and drops several unneeded and hard to install dependencies making builds on OS X and Windows easier than any previous release.

- Removed 3 depedencies without loosing any functionality: `ltdl`, `cairomm` and `libsigc++` (#1804,#806,#1681)

- Added 64 bit integer support in expressions, feature ids, and the grid_renderer (#1661,#1662,#1662)

- Added the ability to disable the need for various dependencies: `proj4`, `libpng`, `libtiff`, `libjpeg`

- Added faster reprojection support between `epsg:3857` and `epsg:4326` (#1705,#1703,#1579)

- Added `colorize-alpha` image filter that applies user provided color gradients based on level of alpha.
  Accepts one or more colors separated by commas. Each color can be paired with an `offset` value separated
  by a space that is either `0-100%` or `0.0-1.0`. An `offset` of `0` is implied and the default. For background
  on where this design came from see http://www.w3.org/TR/SVG/pservers.html#GradientStops. A simple example
  of colorizing alpha into a "rainbow" is `colorize-alpha(blue,cyan,lightgreen, yellow, orange, red)`. An example of
  using offsets and the variety of supported color encodings is to produce a ramp which sharp contrast between `blue`
  and `cyan` is `colorize-alpha(blue 30%, cyan, yellow 0.7 , rgb(0%,80%,0%) 90%)` (#1371).

- Fixed concurrency problem when using cursors in postgis plugin (#1823,#1588)

- Fixed postgres connection pool leaks when using `persist_connection=false` (#1764)

- Fixed postgres connection key to respect highest value of `max_size` and `initial_size` for any layer in map (#1599)

- Fixed potential crash in wkb parsing when postgis returns null geometry (#1843)

- Fixed blurry rendering of image and SVG icons (#1316)

- Added detection of invalid srs values when loading xml (#646)

- Added support for specifying a base_path as a third, optional argument to load_xml

- Removed muffling of projection errors while rendering (#646)

- Improved logging system (https://github.com/mapnik/mapnik/wiki/Logging)

- Added support for reading images from in memory streams (#1805)

- Optimized halo rendering. When halo radius is < 1 new method will be used automatically (#1781)

- Added `text-halo-rasterizer` property. Set to `fast` for lower quality but faster
  halo rendering (#1298) which matched new default method when radius is < 1.

- Added support in `shape`, `sqlite`, `geojson`, and `csv` plugin for handling non-latin characters in the paths to file-based resources (#1177)

- Fixed rendering of markers when their size is greater than the specified `spacing` value (#1487)

- Fixed handling of alpha premultiplication in image scaling (#1489)

- Optimized rendering when a style with no symbolizers is encountered (#1517)

- Optimized string handling and type conversion by removing `boost::to_lower`, `boost::trim`, and `boost::lexical_cast` usage (#1687,#1687,#1633)

- Optimized alpha preserving `hextree` method for quantization of png images (#1629)

- Faster rendering of rasters by reducing memory allocation of temporary buffers (#1516)

- Fixed some raster reprojection artifacts (#1501)

- Fixed raster alignment when width != height and raster is being scaled (#1748,#1622)

- Added support for caching rasters for re-use during rendering when styling more than once per layer (#1543)

- Improved compile speeds of the code - in some cases by up to 2x and removed need for freetype dependency when building code against mapnik (#1688, #1756)

- Removed internal rule cache on `mapnik::Map` c++ object (#1723)

- Improved the scaled rendering of various map features when using `scale_factor` > 1 (#1280,#1100,#1273,#1792,#1291,#1344,#1279,#1624,#1767,#1766)

- Added C++ api for overriding scale_denominator to enable rendering at fixed scale (#1582)

- Added Layer `buffer-size` that can be used to override Map `buffer-size` to avoid
  over-fetching of data that does not need to be buffered as much as other layers.
  Map level `buffer-size` will be default if layers do not set the option. Renamed a
  previously undocumented parameter by the same name that impacted clipping extent and
  was not needed (clipping padding should likely be a symbolizer level option) (#1566)

- Fixed potential file descriptor leaks in image readers when invalid images were encountered (#1783)

- Fixed alpha handling in the `blur` and `invert` image filters (#1541)

- Fixed error reporting in the python plugin (#1422)

- Added the ability to run tests without installing with `make test-local`

- Reduced library binary size by adding support for `-fvisibility-inlines-hidden` and `-fvisibility=hidden` (#1826,#1832)

- Added `mapnik::map_request` class, a special object to allow passing mutable map objects to renderer (#1737)

- Added the ability to use `boost::hash` on `mapnik::value` types (#1729)

- Removed obsolete `geos` plugin (functionality replaced by `csv` plugin) and unmaintained `kismet` plugin (#1809,#1833)

- Added new `mapnik-config` flags: `--all-flags`, `--defines`, `--git-describe`, `--includes`, `--dep-includes`, `--cxxflags`, `--cxx` (#1443)

- Added support for unicode strings as arguments in python bindings (#163)

- Added DebugSymbolizer which is able to render the otherwise invisible collision boxes (#1366)

- Optimized rendering by reducing overhead of using `gamma` property (#1174)

- Fixed rendering artifacts when using `polygon-gamma` or `line-gamma` equal to 0 (#761,#1763)

- Fixed and optimized the display of excessive precision of some float data in labels (#430,#1697)

- Removed the `bind` option for datasources (#1654)

- Added ability to access style list from map by (name,obj) in python (#1725)

- Added `is_solid` method to python mapnik.Image and mapnik.ImageView classes (#1728)

- Changed scale_denominator C++ interface to take scale as first argument rather than map.

- Added support for `background-image` in cairo_renderer (#1724)

- Fixed building symbolizer rendering to be fully sensitive to alpha (8b66128c892 / bc8ea1c5a7a)

- `<Filter>[attr]</Filter>` now returns false if attr is an empty string (#1665)

- `<Filter>[attr]!=null</Filter>` now returns true if attr is not null (#1642)

- Added support for DBF `Logical` type: #1614

- Added serialization of `line-offset` to save_map (#1562)

- Enabled default input plugin directory and fonts path to be set inherited from environment settings in
  python bindings to make it easier to run tests locally (#1594). New environment settings are:
    - MAPNIK_INPUT_PLUGINS_DIRECTORY
    - MAPNIK_FONT_DIRECTORY

- Added support for controlling rendering behavior of markers on multi-geometries `marker-multi-policy` (#1555,#1573)

- Added alternative PNG/ZLIB implementation (`miniz`) that can be enabled with `e=miniz` (#1554)

- Added support for setting zlib `Z_FIXED` strategy with format string: `png:z=fixed`

- Fixed handling of transparency level option in `octree` png encoding (#1556)

- Added ability to pass a pre-created collision detector to the cairo renderer (#1444)

- Tolerance parameter is now supported for querying datasources at a given point (#503/#1499)

- Improved detection of newlines in CSV files - now more robust in the face of mixed newline types (#1497)

- Allow style level compositing operations to work outside of featureset extents across tiled requests (#1477)

- Support for encoding `literal` postgres types as strings 69fb17cd3/#1466

- Fixed zoom_all behavior when Map maximum-extent is provided. Previously maximum-extent was used outright but
  now the combined layer extents will be again respected: they will be clipped to the maximum-extent if possible
  and only when back-projecting fails for all layers will the maximum-extent be used as a fallback (#1473)

- Compile time flag called `PLUGIN_LINKING` to allow input datasource plugins to be statically linked with the mapnik library (#249)

- Fixed `dasharray` rendering in cairo backend (#1740)

- Fixed handling of `opacity` in svg rendering (#1744)

- Fixed uneven rendering of markers along lines (#1693)

- Fixed handling of extra bytes in some shapefile fields (#1605)

- Fixed handling (finally) of null shapes and partially corrupt shapefiles (#1630,#1621)

- Added ability to re-use `mapnik::image_32` and `mapnik::grid` by exposing a `clear` method (#1571)

- Added support for writing RGB (no A) png images by using the format string of `png:t=0` (#1559)

- Added experimental support for geometry simplification at symbolizer level (#1385)

## Mapnik 2.1.0

Released Aug 23, 2012

(Packaged from a25aac8)

- Feature-level compositing (comp-op) for all symbolizers (except building) in AGG and Cairo renderers (#1409)

- Style-level compositing (comp-op) (#1409) and style-level opacity for AGG renderer (#314)

- New experimental framework for image manipulation called `image-filters` to allow things to be done across entire layer canvas like burring (#1412)

- Support for recoloring stroke, fill, and opacity of SVG files (#1410 / #659)

- Support for data-driven transform expressions (#664)

- New support for offsetting geometries / parallel lines in line_symbolizer (#927/#1269)

- New support for clipping geometries - now default enabled on all symbolizers (#1116)

- Framework for chainable geometry transformations (called `vertex_converters`) so that you can do things like clip, smooth, and offset at the same time (#927)

- WKT parsing now is more robust and supports multi-geometries (#745)

- New support for outputting WKT/WKB/GeoJSON/SVG from mapnik.Geometry objects (#1411)

- New experimental python datasource plugin (#1337)

- New experimental geojson datasource plugin using in-memory rtree indexing (#1413)

- Cairo rendering is now much more similiar to AGG rendering as cairo backend now supports `scale_factor` (#1280) and other fixed have landed (#1343, #1233, #1344, #1242, #687, #737, #1006, #1071)

- mapnik::Feature objects and datasource plugins now use a `Context` to store attribute schemas to reduce the memory footprint of features (#834)

- Added Stroke `miterlimit` (#786)

- Python: exposed Map `background_image` (and aliased `background` to `background_color`)

- Python: exposed BuildingSymbolizer

- Support in the CSV plugin for reading JSON encoded geometries (#1392)

- Increased grid encoding performance (#1315)

- Added support for setting opacity dynamically on images in polygon pattern and markers symbolizers

- Added support for filtering on a features geometry type, either `point`, `linestring`, `polygon`,
  or `collection` using the expression keyword of `[mapnik::geometry_type]` (#546)

- MarkersSymbolizer width and height moved to expressions (#1102)

- PostGIS: Added `simplify_geometries` option - will trigger ST_Simplify on geometries before returning to Mapnik (#1179)

- Improved error feedback for invalid values passed to map.query_point

- Fixed rendering of thin svg lines (#1129)

- Improved logging/debugging system with release logs and file redirection (https://github.com/mapnik/mapnik/wiki/Runtime-Logging) (#937 and partially #986, #467)

- GDAL: allow setting `nodata` value on the fly (will override value if `nodata` is set in data) (#1161)

- GDAL: respect `nodata` for paletted/colormapped images (#1160)

- PostGIS: Added a new option called `autodetect_key_field` (by default false) that if true will
  trigger autodetection of the table primary key allowing for feature.id() to represent
  globally unique ids. This option has no effect if the user has not manually supplied the `key_field` option. (#804)

- Cairo: Add full rendering support for markers to match AGG renderer functionality (#1071)

- Fix Markers rendering so that ellipse height/width units are pixels (previously were unintentionally radii) (#1134)

- Added `ignore-placement` attribute to markers-symbolizer (#1135)

- Removed PointDatasource - use more robust MemoryDatasource instead (#1032)

- SQLite - Added support for !intersects! token in sql subselects (#809) allow custom positioning of rtree spatial filter.

- New CSV plugin - reads tabular files - autodetecting geo columns, newlines, and delimiters. Uses in-memory featureset for fast rendering and is not designed for large files (#902)

- Fixed bug in shield line placement when dx/dy are used to shift the label relative to the placement point (Matt Amos) (#908)

- Added <layer_by_sql> parameter in OGR plugin to select a layer by SQL query (besides name or index): see http://www.gdal.org/ogr/ogr_sql.html for specifications (kunitoki) (#472)

- Added support for output maps as tiff files (addresses #967 partially)

- Added support for justify-alignment=auto. This is the new default. (#1125)

- Added support for grouped rendering using the `group-by` layer option: https://github.com/mapnik/mapnik/wiki/Grouped-rendering


## Mapnik 2.0.2

Released Aug 3, 2012

(Packaged from adb2ec741)

- Fixed handling of empty WKB geometries (#1334)

- Fixed naming of `stroke-dashoffset` in save_map (cc3cd5f63f28)

- Fixed support for boost 1.50 (8dea5a5fe239233)

- Fixed TextSymbolizer placement in Cairo backend so it respects avoid-edges and minimum-padding across all renderers (#1242)

- Fixed ShieldSymbolizer placement so it respects avoid-edges and minimum-padding across all renderers (#1242)

- Rolled back change made in 2.0.1 to marker width/height meaning that Mapnik > 2.0.2 will stick to assuming width/heigh are radii for back compatibility with 2.0.0. The reverted change is seen below as "Fix Markers rendering so that ellipse height/width units are pixels (previously were unintentionally radii)". Issue tracking this is #1163

- XML: Fixed to avoid throwing if a `<Parameters>` element is encountered (which is supported in >= 2.1.x)

- Support for PostGIS 2.0 in the pgsql2sqlite command (e69c44e/47e5b3c)

- Fixed reference counting of Py_None when returning null attributes from Postgres during UTFGrid encoding, which could cause a Fatal Python error: deallocating None (#1221)

- Fixed possible breakage registering plugins via python if a custom PREFIX or DESTDIR was used (e.g. macports/homebrew) (#1171)

- Fixed memory leak in the case of proj >= 4.8 and a projection initialization error (#1173)


## Mapnik 2.0.1

Released April 10, 2012

(Packaged from 57347e9106)

- Support for PostGIS 2.0 (#956,#1083)

- Switched back to "libmapnik" and "import mapnik" rather than "mapnik2" (mapnik2 will still work from python) (#941)

- Restored Python 2.5 compatibility (#904)

- Fixed `mapnik-config --version` (#903)

- Cairo: Add full rendering support for markers to match AGG renderer functionality (#1071)

- Fix Markers rendering so that ellipse height/width units are pixels (previously were unintentially radii) (#1134)

- Added `ignore-placement` attribute to markers-symbolizer (#1135)

- Removed svn_revision info from mapnik-config and python bindings as git is now used

- Removed OGCServer from core - now at https://github.com/mapnik/OGCServer (e7f6267)

- Fixed SQLite open stability across platforms/versions (#854)

- Workaround for boost interprocess compile error with recent gcc versions (#950,#1001,#1082)

- Fix possible memory corruption when using `hextree` mode for png color reduction (#1087)

- Fixed bug in shield line placement when dx/dy are used to shift the label relative to the placement point (Matt Amos) (#908)

- Fix to avoid modifying a feature if an attribute is requested that does not exist (0f5ab18ed)

- Fixed ability to save to jpeg format from python (7387afd9) (#896)


## Mapnik 2.0.0

Released September 26, 2011

(Packaged from 5b4c20eab3)

- Add minimum-path-length property to text_symbolizer to allow labels to be placed only on lines of a certain length (#865)

- Add support for png quantization using fixed palettes (#843)

- Add AlsoFilter functionality - https://github.com/mapnik/mapnik/wiki/AlsoFilter

- SQLite Plugin: optimize i/o using shared cache and no mutexes (#797)

- Directly link input plugins to libmapnik to avoid having to set dlopen flags from binding languages (#790)

- Throw an error during registration for fonts which Freetype2 does not report a family or style name (r2985).

- Fixed quoting syntax for "table"."attribute" in PostGIS plugin (previously if table aliases were used quoting like "table.attribute" would cause query failure) (r2979).

- Added the ability to control the PostGIS feature id by suppling a key_field to reference and integer attribute name (r2979).

- Added alternative, more robust proj_transform functions to project a bbox using more points than just the four
  corners to ensure an optimally sized bbox despite proj4 out of bounds conditions. (olt)

- Added map.base parameter that can be set to control where files with relative paths should be interpreted
  from when a map is loaded from a string or saved to a string. It defaults to an empty string which means
  that the base path will be the current working directory of the mapnik process. When a stylesheet is read
  from a file that files directory is used. And a custom value can still be passed as an argument to
  load_map_from_string().

- Added python function `render_grid` to allow conversion of grid buffer to python object containing list of grid
  pixels, list of keys, and a and dictionary of feature attributes.

- Added new rendering backend, grid_renderer, that collects the attributes of rendered features and
  burns their ids into a grid buffer.

- Added optional `maximum-extent` parameter to map object. If set will be used, instead of combined
  layer extents, for return value of map.zoom_all(). Useful in cases where the combined layer extents
  cannot possibly be projected into the map srs or the user wishes to control map bounds without
  modifying the extents of each layer.

- Support for `nodata` values with grey and rgb images in GDAL plugin (#727)

- Print warning if invalid XML property names are used (#110)

- Made XML property names use consistent dashes, never underscores (#644)

- Added support for drawing only first matching rule using filter-mode="first" in Style (#706)

- Added support to PointSymbolizer (`ignore_placement`) for skipping adding placed points to collision detector (#564)

- Added ability to register fonts within XML using Map level `font-directory` parameter (#168)

- TextSymbolizer: Change text_convert to text_transform to better match css naming (r2211)

- Shapefile Plugin: Throw error if attribute name is requested that does not exist (#604)

- Upgraded to the latest proj4 string literal for EPSG:4326 (WGS84) as global default projection (#333)

- Added `mapnik_version_from_string()` function in python bindings to easily convert string representation
  of version number to the integer format used in `mapnik/version.hpp`. e.g. `0.7.1` --> `701`.

- Added xinclude (http://www.w3.org/TR/xinclude/) support to libxml2-based xml parser (oldtopos) (#567)

- Optimized rendering speeds by avoiding locking in the projection code (r2063) (r2713)

- Added support for setting global alignment of polygon pattern fills (#203)

- Added support for choosing OGR layer by index number using `layer_by_index` parameter (r1904)

- Added support for fractional halo widths (using FT Stroker) (#93)

- Added support for reading jpeg images (in addition to png/tiff) for image symbolizers (#518)

- Made libjpeg dependency optional at compile time and added mapnik2.has_jpeg() method to check for support in python (#545).

- Fixed reading of PostGIS data on Big Endian systems (#515)

- PostGIS: Added better support for alternative schemas (#500)

- AGG Renderer - Enforced default gamma function on all symbolizers to ensure proper antialiasing
  even when gamma is modified on the PolygonSymbolizer. (#512)

- Added ability to read pre 2.0.0 stylesheets, but prints a warning for deprecated syntax (r1592, #501)

- Rasterlite Plugin: Experimental support for Rasterlite, to practically use sqlite database with wavelet compressed rasters (#469)

- PNG: fixed png256 for large images and some improvements to reduce color corruptions (#522)

- Implement MarkersSymbolizer in Cairo render and improve the markers placement finder. (#553)


# Mapnik 0.7.2

Released Oct 18, 2011

(Packaged from bc5cabeb6a)

- Added forward compatibility for Mapnik 2.0 XML syntax (https://github.com/mapnik/mapnik/wiki/Mapnik2/Changes)

- Build fixes to ensure boost_threads are not used unless THREADING=multi build option is used

- Fixes for the clang compiler

- Support for latest libpng (>= 1.5.x) (r2999)

- Fixes to the postgres pool

- Fix for correct transparency levels in png256/png8 output (#540)

- Various build system fixes, especially for gcc compiler on open solaris.

- When plugins are not found, report the searched directories (#568)

- Improved font loading support (#559)

- Fix to shapeindex for allowing indexing of directory of shapefiles like `shapeindex dir/*shp`

- Fixed handling of null and multipatch shapes in shapefile driver - avoiding inf loop (#573)

- Fixed raster alpha blending (#589,#674)

- Enhanced support for faster reprojection if proj >= 4.8 is used (#575)

- Allow for late-binding of datasources (#622)

- Fix to OSM plugin to avoid over-caching of data (#542)

- Various fixes to sqlite, ogr, and occi driver backported from trunk.

- Ensured that `\n` triggers linebreaks in text rendering (#584)

- Support for boost filesystem v3

- Fixes to cairo renderer to avoid missing images (r2526)

- Fixed reading of label_position_tolerance on text_symbolizer and height for building_symbolizer


# Mapnik 0.7.1

Released March 23, 2010

(Packaged from r1745/db89f1ca75)

- Rasters: Various fixes and improvements to 8bit png output ([#522](https://github.com/mapnik/mapnik/issues/522),[#475](https://github.com/mapnik/mapnik/issues/475))

- XML: Save map buffer_size when serializing map.

- SCons: Added new build options `PRIORITIZE_LINKING` and `LINK_PRIORITY`. The first is a boolean (default True)
  of whether to use the new sorting implementation that gives explcit preference to custom or local paths
  during compile and linking that will affect builds when duplicate libraries and include directories are on the
  system. LINK_PRIORITY defaults to prioritizing internal sources of the mapnik source folder, then local/user
  installed libraries over system libraries, but the option can be customized. Sorting not only ensures that
  compiling and linking will more likely match the desired libraries but also gives more likelyhood to avoid
  the scenario where libraries are linked that don`t match the includes libmapnik compiled against.

- XML: Fixed behavior of PolygonPatternSymbolizer and LinePatternSymbolizer whereby width, height,
  and type of images is actually allowed to be optionally ommitted ([#508](https://github.com/mapnik/mapnik/issues/508)). This was added in r1543 but
  only worked correctly for PointSymbolizer and ShieldSymbolizer.

- Fixed reading of PostGIS data on Big Endian systems ([#515](https://github.com/mapnik/mapnik/issues/515))

- PostGIS: Added better support for alterative schemas ([#500](https://github.com/mapnik/mapnik/issues/500))

- AGG Renderer - Enforced default gamma function on all symbolizers to ensure proper antialiasing
  even when gamma is modified on the PolygonSymbolizer. ([#512](https://github.com/mapnik/mapnik/issues/512))

- PNG: fixed png256 for large images and some improvements to reduce color corruptions ([#522](https://github.com/mapnik/mapnik/issues/522))

- PNG: Added new quantization method for indexed png format using `hextree` with full support for alpha
  channel. Also new method has some optimizations for color gradients common when using elevation based
  rasters. By default old method using `octree` is used. (r1680, r1683, [#477](https://github.com/mapnik/mapnik/issues/477))

- PNG: Added initial support for passing options to png writter like number of colors, transparency
  support, quantization method and possibly other in future using type parameter. For example
  "png8:c=128:t=1:m=h" limits palette to 128 colors, uses only binary transparency (0 - none,
  1 - binary, 2 - full), and new method of quantization using `hextree` (h - `hextree`, o - `octree`).
  Existing type "png256" can be also written using "png8:c=256:m=o:t=2"  (r1680, r1683, [#477](https://github.com/mapnik/mapnik/issues/477))


# Mapnik 0.7.0

Released January, 19 2010

(Packaged from r1574/a0da946be9)

- Core: Fixed linking to external libagg (r1297,r1299)

- Core: Completed full support for PPC (Big endian) architectures (r1352 -> r1357)

- Gdal Plugin: Added support for Gdal overviews, enabling fast loading of > 1GB rasters (#54)

    * Use the gdaladdo utility to add overviews to existing GDAL datasets

- PostGIS: Added an optional `geometry_table` parameter. The `geometry_table` used by Mapnik to look up
  metadata in the geometry_columns and calculate extents (when the `geometry_field` and `srid` parameters
  are not supplied). If `geometry_table` is not specified Mapnik will attempt to determine the name of the
  table to query based on parsing the `table` parameter, which may fail for complex queries with more than
  one `from` keyword. Using this parameter should allow for existing metadata and table indexes to be used
  while opening the door to much more complicated subqueries being passed to the `table` parameter without
  failing (#260, #426).

- PostGIS Plugin: Added optional `geometry_field` and `srid` parameters. If specified these will allow
  Mapnik to skip several queries to try to determine these values dynamically, and can be helpful to avoid
  possible query failures during metadata lookup with complex subqueries as discussed in #260 and #436, but
  also solvable by specifying the `geometry_table` parameter. (r1300,#376)

- PostGIS: Added an optional `extent_from_subquery` parameter that when true (while the `extent` parameter is
  not provided and `estimate_extent` is false) will direct Mapnik to calculate the extent upon the exact table
  or sql provided in the `table` parameter. If a sub-select is used for the table parameter then this will,
  in cases where the subquery limits results, provide a faster and more accurate layer extent. It will have
  no effect if the `table` parameter is simply an existing table. This parameter is false by default. (#456)

- PostGIS Plugin: Added `!bbox!` token substitution ability in sql query string. This opens the door for various
  complex queries that may aggregate geometries to be kept fast by allowing proper placement of the bbox
  query to be used by indexes. (#415)

    * Pass the bbox token inside a subquery like: !bbox!

    * Valid Usages include:

        <Parameter name="table">
          (Select ST_Union(geom) as geom from table where ST_Intersects(geometry,!bbox!)) as map
        </Parameter>

        <Parameter name="table">
          (Select * from table where geom &amp;&amp; !bbox!) as map
        </Parameter>

- PostGIS Plugin: Added `scale_denominator` substitution ability in sql query string (#415/#465)

    * Pass the scale_denominator token inside a subquery like: !scale_denominator!

    * e.g. (Select * from table where field_value > !scale_denominator!) as map

- PostGIS Plugin: Added support for quoted table names (r1454) (#393)

- PostGIS: Add a `persist_connection` option (default true), that when false will release
  the idle psql connection after datasource goes out of scope (r1337) (#433,#434)

- PostGIS: Added support for BigInt (int8) postgres type (384)

- PostGIS Plugin: Throw and report errors if SQL execution fails (r1291) (#363, #242)

- PostGIS Plugin: Fixed problem in conversion of long numbers to strings (r1302,1303)

- PostGIS Plugin: Added missing support for BigInt(int8) postgres datatypes (r1250) (#384)

- OGR Plugin: Added support for reading multipoint features (#458)

- Shape Plugin: Fixed bug in file extension stripping (#413)

- Shape Plugin: Fixed missing compiler flags that causes crashing on newer g++ versions (#436)

- PNG: Fixed problem with garbled/striped png256 output along sharp edges(#416,#445,#447,#202)

- PNG: Added support for semi-transparency in png256 output (#477,#202)

- PolygonSymbolizer: Added `gamma` attribute to allow for dilation of polygon edges - a solution
  to gap artifacts or "ghost lines" between adjacent polygons and allows for slight sharpening of
  the edges of non overlapping polygons. Accepts any values but 0-1 is the recommended range.

- TextSymbolizer: Large set of new attributes: `text_transform`, `line_spacing`, `character_spacing`,
  `wrap_character`, `wrap_before`, `horizontal_alignment`, `justify_alignment`, and `opacity`.

    * More details at changesets: r1254 and r1341

- SheildSymbolizer: Added special new attributes: `unlock_image`, `VERTEX` placement, `no_text` and many
  attributes previously only supported in the TextSymbolizer: `allow_overlap`, `vertical_alignment`,
  `horizontal_alignment`, `justify_alignment`, `wrap_width`, `wrap_character`, `wrap_before`, `text_transform`,
  `line_spacing`, `character_spacing`, and `opacity`.

    * More details at changeset r1341

- XML: Added support for using CDATA with libxml2 parser (r1364)

- XML: Fixed memory leak in libxml2 implementation (#473)

- XML: Added function to serialize map to string, called `mapnik.save_map_to_string()` (#396)

- XML: Added parameter to <Map> called `minimum_version` to allow for enforcing the minimum Mapnik version
  needed for XML features used in the mapfiles. Uses Major.Minor.Point syntax, for example
  <Map minimum_version="0.6.1"> would throw an error if the user is running Mapnik less than 0.6.1.

- XML: Added support for relative paths when using entities and `mapnik.load_map_from_string()` (#440)

- XML: Made width and height optional for symbolizers using images (r1543)

- XML: Ensured that default values for layers are not serialized in save_map() (r1366)

- XML: Added missing serialization of PointSymbolizer `opacity` and `allow_overlap` attributes (r1358)

- XML: Default text vertical_alignment now dependent on dy (#485, r1527)

- Python: Exposed ability to write to Cairo formats using `mapnik.render_to_file()` and without pycairo (#381)

- Python: Fixed potential crash if pycairo support is enabled but python-cairo module is missing (#392)

- Python: Added `mapnik.has_pycairo()` function to test for pycairo support (r1278) (#284)

- Python: Added `mapnik.register_plugins()` and `mapnik.register_fonts()` functions (r1256)

- Python: Pickling support for point_symbolizer (r1295) (#345)

- Python: Ensured mapnik::config_errors now throw RuntimeError exception instead of UserWarning exception (#442)

- Filters: Added support for `!=` as an alias to `<>` for not-equals filters (avoids &lt;&gt;) (r1326) (#427)

- SCons: Improved boost auto-detection (r1255,r1279)

- SCons: Fixed support for JOBS=N and FAST=True to enable faster compiling (r1440)

- SCons: Ensured that -h or --help will properly print help on custom Mapnik options before a user
  has been able to properly run `configure`. (r1514)

- SCons: Added ability to link to custom icu library name using ICU_LIB_NAME (r1414)

- SCons: Improved reliability of python linking on OSX (#380)

- Fonts: Added unifont to auto-installed fonts, which is used by the OSM styles as a fallback font (r1328)


# Mapnik 0.6.1

Released July 14, 2009

(Packaged from r1247/353ff576c7)

- Plugins: expose list of registered plugins as a `plugin_names()` method of DatasourceCache (r1180)

- XML: Fixed serialization and parsing bugs related to handling of integers and Enums (#328,#353)

- SCons: Added the ability to set the PKG_CONFIG_PATH env setting (#217)

- SCons: Improved linking to only required libraries for libmapnik (#371)

- Shape Plugin: Added compile time flag to allow disabling the use of memory mapped files (r1213) (#342)

- Core: Improved support for PPC (Big endian) architectures (r1198 -> r1213)

- Scons: Improved auto-detection of boost libs/headers (r1200) (#297)

- Plugins: Exposed list of available/registered plugins (r1180) (#246)

- SCons: Improve build support for SunCC (patches from River Tarnell) (r1168, r1169)

- Python: Pickling support for text_symbolizer (r1164) (#345)

- Python: Pickling support for proj_transform and view/coord_transform (r1163) (#345)

- Python: Pickling support for parameters (r1162) (#345)

- Python: Pickling support for stroke objects (r1161) (#345)

- Python: Pickling support for line_symbolizer (r1160) (#345)

- Python: Pickling support for projection objects (r1159) (#345)

- Python: Pickling support for shield_symbolizer (r1158) (#345)

- Python: Pickling support for polygon_symbolizer (r1157) (#345)

- Python: Pickling support for query objects (r1156) (#345)

- Python: Pickling support for pattern symbolizers (r1155) (#345)

- Python: Pickling support for raster_symbolizer (r1154) (#345)

- Python: Added `mapnik.has_cairo()` function to test for cairo support (r1152) (#284)

- Python: Exposed dash_array get method (r1151) (#317)

- Python: Pickling support for Coord objects (#345)

- GDAL Plugin: Added an experimental option to open files in `shared mode` (r1143)

- Python: Exposed RasterSymbolizer options in Python (r1139)

- Plugins: Fixed support for non-file based sources in GDAL and OGR plugins (#336,#337)

- Plugins: Formal inclusion of new plugin for Kismet server (r1127) (#293)

- Python: Made access to features and featuresets more Pythonic (r1121) (#171,#280,#283)

- XML: Ensured relative paths in XML are interpreted relative to XML file location (r1124) (#326)

- XML: Added ability to serialize all default symbolizer values by passing third argument to save_map(m,`file.xml`,True)(r1117) (#327)

- Core: Added support for alpha transparency when writing to png256 (patch from Marcin Rudowski) (#202)

- SCons: Ensured ABI compatibility information is embedded in libmapnik.dylib on Mac OS X (#322)

- SCons: Ensured that the full `install_name` path would be added to libmapnik.dylib on Mac OS X (#374)

- Tests: Added testing framework in Python using nose (r1101-r1105)

- Raster Plugin: Added a tile/bbox-based read policy for large (rasters width * height > 1024*1024 will be loaded in chunks) (r1089)

- OGCServer: Made lxml dependency optional (r1085) (#303)

- Rasters: Handle rounding to allow better alignment of raster layers (r1079) (#295)

- AGG Renderer: Added option to control output JPEG quality (r1078) (#198)

- Plugins: Fixed segfault in OGR Plugin with empty geometries (r1074) (#292)


# Mapnik 0.6.0

Released April 1, 2009

(Packaged from r1066/c88e03436f)

- Python: Added support for aspect_fix_mode (r1013)

- OGCServer Fixed axis-ordering for WMS 1.3.0 request (r1051) (#241)

- Plugins: Added option to all plugins to support using a `base` path argument (r1042)

- Symbolizers: RasterSymbolizer now support composing modes for hillshading (r1027)

- SCons: Added options to build the rundemo and pgsql2sqlite tools (r989)

- OGCServer: Added content-length output (r986)

- SCons: Replaced LIBS/INCLUDES options for postgres and gdal with pg_config and gdal-config (r977)

- SCons: Created an optional configure stage (r973)

- Python: Added further pickling/copy support to Map, Layers, Datasources, Styles,and Rules (r907,r913,r921)

- Plugins: Added Sqlite driver for reading sqlite databases (r881)

- Python: Exposed a number of properties for the Text Symbolizer (r869)

- Plugins: PostGIS plugin now accepts multi-line queries (r862)

- Filter parsing: Allow numbers in the filter field name.
  This allows for shapefiles with columns like `1970`.

- Plugins: Added OGR driver for reading all OGR supported formats (kunitoki) (r836) (#170)

- XML: Added serialization of Fontsets (r807)

- XML: Added support for reading xml from a string (r806)

- C++: renamed mapnik::Color to mapnik::color (r796)

- Python: Made available the scale_denominator property from the map in c++ and python (r794)

- Python: Added ability to resize map and clear all layers and styles from python (r793)

- Python: Exposed Proj to/from transformation for projected coordinate systems (r792,r822) (#117)

- Memory Datasource: Added support for dynamically adding Points to map using Point Datasource (r790)

- XML: Added xml serialization for abstract, title, minzoom, maxzoom, and queryable attributes (r787)

- Core: Transformation is now skipped if srs values match exactly (r777)

- Symbolizers: `min_distance` now honored for POINT placement using Text Symbolizer (r771)

- Plugins: PostGIS plugin now accepts a geometry_field,record_limit, cursor_size options (r769,r872)

- Python: Added ability to transform as a method on Coord and Envelope objects (r764)

- Python: Added docstrings to the Layer object (r763)

- Plugins: Loosened the type checking in Shapefile Plugin dbf reader (r762)

- Fonts: Added support for Right-to-left Hebrew text (r749)

- Core: Added a Map buffer parameter - helps to avoid cut labels at tile edges (r744)

- Symbolizers: Added opacity support to Point Symbolizer (r743)

- Symbolizers: Added support of using Points with Shield Symbolizer (r741)

- Plugins: PostGIS plugin now accepts alternate schemas (r773)

- Core: Added a Map aspect_fix_mode to ensure proper syncing of map dimensions and bbox (r705)

- Fonts: Added support for fallback fonts (r704)

- Cairo: Cairo support exposed in Python (r666)

- Plugins: Added OSM plugin for reading directly from OSM data (r663)

- Filters: Added support for boolean expressions (r660)

- Python: Added ability to open Image32 files (r652)

- Cairo: Cairo rendering support added (r656)

- Core: Added unicode support based on ICU (r650)

- Core: Added support for single and multi threaded variants of Mapnik (r632,r634)

- Plugins: Use memory mapped files for reading shape file (r628)

- Core: Use streams to write images (i/o refactor) (r628) (#15)

# Mapnik 0.5.1

Released April 15, 2008

(Packaged from c29cb7386d)

# Mapnik 0.5.0

Released April 15, 2008

(Packaged from 0464a3563c)

# Mapnik 0.4.0

Released February 26, 2007

(Packaged from 8d73e3a8dc)

# Mapnik 0.3.0

Released May 22, 2006

(Packaged from 3ae046ebe2)
