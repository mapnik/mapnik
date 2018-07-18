# Mapnik Changelog

A simple log of core changes affecting Mapnik usage.

Developers: Please commit along with changes.

For a complete change history, see the git log.


## UNRELEASED

#### Notice

- Mapnik now requires C++14 compliant compiler (`-std=c++14`)

#### Core

- Rewrote parser grammars using Boost.Spirit X3 (replacing Boost.Spirit Qi)
- Fixed bbox reprojection ([#3935](https://github.com/mapnik/mapnik/issues/3935))
- Fixed missing MarkersSymbolizer placement on zero-length lines ([#3899](https://github.com/mapnik/mapnik/issues/3899))
- Fixed segfault when ShieldSymbolizer has invalid placements ([5464ae9](https://github.com/mapnik/mapnik/commit/5464ae9cdfd32983883463bcc2396dc0e51d885f), [#3604](https://github.com/mapnik/mapnik/issues/3604#issuecomment-281542148))
- Fixed `apply_color_blind_filter` ([#3862](https://github.com/mapnik/mapnik/issues/3862))
- Fixed GroupSymbolizer crash, `svg::path_attributes` are now stored in `std::deque` ([#3455](https://github.com/mapnik/mapnik/issues/3455))
- Duplicate Style name in map XML now throws `config_error` in strict mode ([#3770](https://github.com/mapnik/mapnik/issues/3770), [#3917](https://github.com/mapnik/mapnik/issues/3917))
- Improved padding calculation for polygon/line clipping ([#3909](https://github.com/mapnik/mapnik/issues/3909))
- Slightly improved `sql_utils::table_from_sql` ([2587bb3](https://github.com/mapnik/mapnik/commit/2587bb3a1d8db397acfa8dcc2d332da3a8a9399f))
- Added wrappers for proper quoting in SQL query construction: `sql_utils::identifier`, `sql_utils::literal` ([7b21713](https://github.com/mapnik/mapnik/commit/7b217133e2749b82c2638551045c4edbece15086))
- Added two-argument `sql_utils::unquote`, `sql_utils::unquote_copy` that also collapse inner quotes ([a4e8ea2](https://github.com/mapnik/mapnik/commit/a4e8ea21be297d89bbf36ba594d6c661a7a9ac81))

#### Plugins

- PostGIS: changed syntax for user `@variable` interpolation to `!@variable!` ([#3618](https://github.com/mapnik/mapnik/issues/3618))
- PostGIS: using parameter `estimate_extent` now requires PostGIS >= 2.1.0 ([#3624](https://github.com/mapnik/mapnik/issues/3624))
- PGraster: added variable interpolation like in PostGIS plugin ([#3618](https://github.com/mapnik/mapnik/issues/3618))
- PGraster: using parameter `estimate_extent` now requires PostGIS >= 2.1.0 ([#3624](https://github.com/mapnik/mapnik/issues/3624))


## 3.0.20

Released: April 12, 2018

(Packaged from [f02c7bc](https://github.com/mapnik/mapnik/commit/f02c7bcdb))

- Make `max_image_area` a datasource parameter for GDAL.
- GDAL Driver Overview Fix and Memory Reduction ([#3872](https://github.com/mapnik/mapnik/issues/3872))
- Raster colorizer: check image bounds ([#3879](https://github.com/mapnik/mapnik/issues/3879))
- Removed usage of `typename` in template template declarations (available in C++17) ([#3882](https://github.com/mapnik/mapnik/issues/3882))


## 3.0.19

Released: March 06, 2018

(Packaged from [d50562d](https://github.com/mapnik/mapnik/commit/d50562d54))

- Backported scaling of precision by polygon size  ([#3844](https://github.com/mapnik/mapnik/issues/3844))
- Backported GRID placement ([#3847](https://github.com/mapnik/mapnik/issues/3847), [#3854](https://github.com/mapnik/mapnik/issues/3854), [#3855](https://github.com/mapnik/mapnik/issues/3855))
- Added missing `MAPNIK_DECL` to all `text_placement_` types ([7ce142a](https://github.com/mapnik/mapnik/commit/7ce142a5aa8e9da5ddd11266a054c1e69052230d))
- Fixed invalid memory access if input buffer size is zero ([a602c65](https://github.com/mapnik/mapnik/commit/a602c65354a4b595821d2300f38ebc107d07e2a9))
- Fixed handling of an empty polygon in `grid_vertex_converter` ([2f2dcf1](https://github.com/mapnik/mapnik/commit/2f2dcf1eeae71aaa7878f4bc9a39741321f07e68))
- Fixed `PROJ_LIB` detection logic ([44f1ae3](https://github.com/mapnik/mapnik/commit/44f1ae3a6e9e9979d1a93343f40db6cd7dbf51d5))
- Default to `icu-config` for obtaining `ICU_DATA` if `u_getDataDirectory` fails ([2cef98d](https://github.com/mapnik/mapnik/commit/2cef98d7f76cdd302afcf15f1c585379537e8f1d))


## 3.0.18

Released: January 26, 2018

(Packaged from [44ef46c](https://github.com/mapnik/mapnik/commit/44ef46c81))

- SVG parser - fixed logic for calculating dimensions when `width` and `height` expressed in
  percentage units ([#3812](https://github.com/mapnik/mapnik/issues/3812))
- New improved `interior` placement algorithm ([#3839](https://github.com/mapnik/mapnik/issues/3839))
- Fixed handling of an empty interior rings in `polygon_vertex_processor` ([#3833](https://github.com/mapnik/mapnik/issues/3833))
- Fixed handling of an empty interior rings in `vertex_adapter` ([#3842](https://github.com/mapnik/mapnik/issues/3842))([#3838](https://github.com/mapnik/mapnik/issues/3838))


## 3.0.17

Released: November 29, 2017

(Packaged from [ebdd96c](https://github.com/mapnik/mapnik/commit/ebdd96c61))

- Use `Scons 3` as an internal build sytsem + support both Python 2 and 3.
- Added glibcxx workaround to support libstdc++-4.8


## 3.0.16

Released: November 16, 2017

(Packaged from [8d7b75e](https://github.com/mapnik/mapnik/commit/8d7b75e))

- Added "strict" SVG parsing mode with consistent error handling  and disabled processing of unsupported attributes.
- Added support for `<use>` element.
- Implemented compile time string literal to integer conversion, to be able to convert large `if/else if/else` statements to `switch`.
- WKB reader - pre-allocate optimisations in `multi_polygon` and `geometry_collection`.
- Set alpha values in RGBA TIFFs even when `NODATA` value is present. ([#3714](https://github.com/mapnik/mapnik/issues/3714))
- Support building with ICU >= 59.
- SCons - added `ICU_DATA`, `PROJ_LIB` and `GDAL_DATA` settings, available via `mapnik-config`
- Fixed centroid and interior text placement algorithms ([#3771](https://github.com/mapnik/mapnik/issues/3771))
- Fixed memory leak ([#3775](https://github.com/mapnik/mapnik/issues/3775))
- SVG parser - fixed default gradient vector in linear gradient.
- Fixed bounding box collection logic ([#3709](https://github.com/mapnik/mapnik/issues/3709))


## 3.0.15

Released: June 16, 2017

(Packaged from [6e6cf84](https://github.com/mapnik/mapnik/commit/6e6cf84))

- Restored `filter_factor` logic in `gdal.input` and added to `raster.input` ([#3699](https://github.com/mapnik/mapnik/issues/3699))
   (updated tests [mapnik/test-data-visual@fd518f1](https://github.com/mapnik/test-data-visual/commit/fd518f1f512b8aea4ac740c2ce12c249616a291c))
- Fixed bug related to rows swapping implementation in `tiff_reader` ref [#3679](https://github.com/mapnik/mapnik/issues/3679)
   (updated visual tests to catch this regression in the future
      [mapnik/test-data-visual@be0ba96](https://github.com/mapnik/test-data-visual/commit/be0ba965cd2240576a8edfca84801cbb7a4832d6))
- TIFF I/O - port memory mapped based I/O from master


## 3.0.14

Released: June 5, 2017

(Packaged from [2b42e17](https://github.com/mapnik/mapnik/commit/2b42e17))

- Fixed problems with high levels of overzooming in the GDAL and raster plugin where data would be slightly offset
- High levels of overzooming on raster data no longer results in the return of a transparent image.
- Fixed bug in `mapnik::util::file::data()` ([a220bda](https://github.com/mapnik/mapnik/commit/a220bda05d2aa1))
- TIFF I/O - added support for grey scale multiband images + fixed and made generic `read_stripped` and `read_generic`.
- shapeindex - return error code when no features can read from shapefile ([#3198](https://github.com/mapnik/mapnik/issues/3198))
- Upgrade Scons to `2.5.1`
- Fixed bug (typo) in `raster_featureset.cpp` ([#3696](https://github.com/mapnik/mapnik/issues/3696))
- Made `freetype_engine` singleton again. This allows for better control of its life-time. Original interface is preserved via adding static methods ([#3688](https://github.com/mapnik/mapnik/issues/3688))


## 3.0.13

Released: February 8, 2017

(Packaged from [2a153c0](https://github.com/mapnik/mapnik/commit/2a153c0))

- Unbundle `unifont` font from distribution
- GeoJSON: improved parsing grammar avoiding temp synthesised attribute ([#3507](https://github.com/mapnik/mapnik/issues/3507))
- GeoJSON: expose `num_features_to_query` datasource parameter + unit test ([#3515](https://github.com/mapnik/mapnik/issues/3515))
- Fixed intersecting extents in different projections (PR [#3525](https://github.com/mapnik/mapnik/issues/3525) )
- Fixed `blur` implementation by taking into account `scale_factor`
- postgis.input - use 2D box for pgraster bounding box (PR [#3551](https://github.com/mapnik/mapnik/issues/3551))
- Fixed GroupSymbolizer PairLayout with 3+ items ([#3526](https://github.com/mapnik/mapnik/issues/3526))
- Simplified `hash` implementation ([204d30e](https://github.com/mapnik/mapnik/commit/204d30e58d3553278ab6bcda2d4122b0f13f6392))
- Simplified `mapnik::value` conversion rules ([#3570](https://github.com/mapnik/mapnik/issues/3570))
- Changed `render_thunk_list` to `std::list<render_thunk>` (PR [#3585](https://github.com/mapnik/mapnik/issues/3585))
- Upgraded to variant `v1.1.5`
- CSV.input - fixed `blank` line test ([8a3a380](https://github.com/mapnik/mapnik/commit/8a3a380b3b5c64681f2478b4f0d06f6a907f5eed))
- GeoJSON - handle empty elements in position grammar (ref [#3609](https://github.com/mapnik/mapnik/issues/3609))
- mapnik-index - return failure on invalid bounding box (ref [#3611](https://github.com/mapnik/mapnik/issues/3611))


## 3.0.12

Released: September 8, 2016

(Packaged from [1d22d86](https://github.com/mapnik/mapnik/commit/1d22d86))

- Ensured gdal.input is registered once (refs [#3093](https://github.com/mapnik/mapnik/issues/3093) [#3339](https://github.com/mapnik/mapnik/issues/3339) [#3340](https://github.com/mapnik/mapnik/issues/3340))
- Fixed `mapnik::util::is_clockwise` implementation to use coordinates relative to the origin and avoid numeric precision issues
- `mapnik-index` is updated to fail on first error in input (csv)
- Added `guard` to `get_object_severity` method (ref [#3322](https://github.com/mapnik/mapnik/issues/3322))
- Improved `hash` calculation for `mapnik::value` (ref [#3406](https://github.com/mapnik/mapnik/issues/3406))
- AGG - made cover `unsigned` to avoid left shift of negative values (ref [#3406](https://github.com/mapnik/mapnik/issues/3406))
- Fixed using `scale_factor` in `evaluate_transform(..)`
- Fixed line spacing logic by applying `scale factor`
- ~~Fixed `stringify_object/stringify_array` implementations by disabling white space skipping (ref [#3419](https://github.com/mapnik/mapnik/issues/3419))~~
- Added geojson unit test for property types/values
- JSON - added support for object and array type in `json_value` and update `stringifier`
- GDAL.input - fallback to using `overviews` if present ([8e84828](https://github.com/mapnik/mapnik/commit/8e8482803bb435726534c3b686a56037b7d3e8ad))
- TopoJSON.input - improved and simplified grammar/parser implementation ([#3429](https://github.com/mapnik/mapnik/pull/3429))
- GDAL.input - Added support for non-alpha mask band
- TopoJSON.input - fixed order of ellements limitation (ref [#3434](https://github.com/mapnik/mapnik/issues/3434))
- Fixed stroke-width size not included in markers ellipse bounding box (ref [#3445](https://github.com/mapnik/mapnik/issues/3445))
- Implemented `char_array_buffer` and removed `boost::iostreams` dependency ([2e8c0d3](https://github.com/mapnik/mapnik/commit/2e8c0d36c2237f2815d8004c1b96bad909056eb9))
- JSON.input - `extract_bounding_box_grammar` - make features optional (ref [#3463](https://github.com/mapnik/mapnik/issues/3463))
- Ensure input plugins return `empty_featureset` rather than `nullptr` (feature_ptr())
- Added support for quantising small (less than 3 pixel) images (ref [#3466](https://github.com/mapnik/mapnik/issues/3466))
- Added support for natural logarithm function in expressions (ref [#3475](https://github.com/mapnik/mapnik/issues/3475))
- Improved logic determining if certain compiler features are available e.g `inheriting constructors` (MSVC)
- GeoJSON - corrected quoting in `stringify` objects (ref [#3491](https://github.com/mapnik/mapnik/issues/3491))
- GeoJSON - ensured consistent ordering of attribute descriptors (ref [#3494](https://github.com/mapnik/mapnik/issues/3494))
- GeoJSON - exposed `num_features_to_query` as datasource paramer (ref [#3495](https://github.com/mapnik/mapnik/issues/3495))
- Replaced `boost::mpl::vector<Types...>` with `std::tuple<Types...>` (ref [#3503](https://github.com/mapnik/mapnik/issues/3503))
- BuildingSymbolizer - fixed closing segment of polygon in building symbolizer (ref [#3505](https://github.com/mapnik/mapnik/issues/3505))
- Update dependencies versions
- Fixed warnings when compiling with g++5
- Fixed image swap (ref [#3513](https://github.com/mapnik/mapnik/issues/3513))
- Stop bundling testdata in source tarball (ref [#3335](https://github.com/mapnik/mapnik/issues/3335))


## 3.0.11

Released: April 1, 2016

(Packaged from [8d9dc27](https://github.com/mapnik/mapnik/commit/8d9dc27))

 - Raster scaling: fixed crash and clipping negative pixel values of floating point rasters ([#3349](https://github.com/mapnik/mapnik/pull/3349))
 - Restored support for unquoted strings in expressions ([#3390](https://github.com/mapnik/mapnik/pull/3390))
 - [TWKB](https://github.com/TWKB/) support via [#3356](https://github.com/mapnik/mapnik/pull/3356) ([#3355](https://github.com/mapnik/mapnik/issues/3355))
 - Visual test runner can render SVG, PDF and Postscript with Cairo renderer ([#3418](https://github.com/mapnik/mapnik/pull/3418))
 - Scale factor is now applied also to `text-line-spacing` and transforms ([#3416](https://github.com/mapnik/mapnik/pull/3416))


## 3.0.10

Released: February 25, 2016

(Packaged from [5c0d496](https://github.com/mapnik/mapnik/commit/5c0d496))

 - The `shapeindex` command now has a `--index-parts` option. When used the index will be bigger
   but will allow the Shapefile datasource to only parse polygon parts within the query bounds.
 - WARNING: index files generated with this newer Mapnik are invalid for older versions of Mapnik.
 - Any `.index` files accompanying a `.shp` must now be regenerated otherwise
   it will be skipped. To avoid this problem you can delete the existing `.index` files, or ideally run `shapeindex` to recreate the `.index`. ([#3300](https://github.com/mapnik/mapnik/pull/3300))
   The trigger for this change was an optimization that required a new binary format for the shapefile indexes ([#3217](https://github.com/mapnik/mapnik/pull/3217)).
 - Shapeindex - another fix for skipping `null` shapes ([#3288](https://github.com/mapnik/mapnik/issues/3288))
 - Fixed support for filter expressions starting with `not` ([#3017](https://github.com/mapnik/mapnik/issues/3017))
 - Ensure `mapped_memory_cache` acts as singleton across shared objects ([#3306](https://github.com/mapnik/mapnik/issues/3306))
 - Removed miniz support in PNG encoder ([#3281](https://github.com/mapnik/mapnik/issues/3281))
 - Added `-fvisibility=hidden -fvisibility-inlines-hidden` to default compiler flags
 - Fixed parsing of SVG `PathElement` ([#3225](https://github.com/mapnik/mapnik/issues/3225))
 - JSON parsing now supports arbitrary (nested) attributes in `geometry`
 - Support for rendering `dash-array` in SVGs
 - SVG parser is now stricter (fails is all input is not parsable) ([#3251](https://github.com/mapnik/mapnik/issues/3251))
 - SVG parser now correctly handles optional separator `(,)` between multiple command parts
 - Optimized parsing of `png` format string
 - The `memory_datasource` now dynamically reports correct datasource type (vector or raster)
 - Upgraded `mapbox::variant v1.1.0`
 - Compare: https://github.com/mapnik/mapnik/compare/v3.0.9...v3.0.10


## 3.0.9

Released: November 26, 2015

(Packaged from [03a0926](https://github.com/mapnik/mapnik/commit/03a0926))

 - Fixed offsetting of complex paths and sharp angles ([#3160](https://github.com/mapnik/mapnik/pull/3160)) (via [@winni159](https://github.com/winni159))
 - Fixed mapnik.util.variant issue when compiling with gcc-5.x and SSO enabled by default ([#3103](https://github.com/mapnik/mapnik/issues/3103)) (via [@nkovacs](https://github.com/nkovacs))
 - Fixed issue with complex scripts where some character sequences weren't rendered correctly ([#3050](https://github.com/mapnik/mapnik/issues/3050)) (via [@jkroll20](https://github.com/jkroll20))
 - Revived postgis.input tests
 - JSON: geometry grammar has been re-factored and optimized to have expectation points
 - Filled missing specializations for value_bool in `mapnik::value` comparison operators
 - `mapnik.Image` - fixed copy semantics implementation for internal buffer
 - JSON parsing: unified error_handler across all grammars
 - Improved unit test coverage
 - Raster scaling: fixed nodata handling, accuracy when working with small floats and clipping floats by \[0; 255\] ([#3147](https://github.com/mapnik/mapnik/pull/3147))
 - Added [`code of conduct`](http://contributor-covenant.org)
 - GeoJSON plug-in is updated to skip feature with empty geometries
 - GeoJSON plug-in : ensure original order of features is preserved (fixed) ([#3182](https://github.com/mapnik/mapnik/issues/3182))
 - Shapeindex utility: fixed `empty` shapes handling and ported tests to c++
 - Centroid algorithm: fixed invalid input handling, particularly empty geometries ([#3185](https://github.com/mapnik/mapnik/pull/3185))
 - Updated SCons build system to the latest version 2.4.1 (http://scons.org/)


## 3.0.8

Released: October 23, 2015

(Packaged from [2d15567](https://github.com/mapnik/mapnik/commit/2d15567))

 - Renamed `SHAPE_MEMORY_MAPPED_FILE` define to `MAPNIK_MEMORY_MAPPED_FILE`. Pass `./configure MEMORY_MAPPED_FILE=True|False` to request
   support for memory mapped files across Mapnik plugins (currently shape, csv, and geojson).
 - Unified `mapnik-index` utility supporting GeoJSON and CSV formats
 - Increased unit test coverage for GeoJSON and CSV plugins
 - shape.input - re-factor to support \*.shx and improve handling various bogus shapefiles
 - geojson.input - make JSON parser stricter + support single Feature/Geometry as well as FeatureCollection
 - maintain 'FT_LOAD_NO_HINTING' + support >= harfbuzz 1.0.5
 - geojson.input - implement on-disk-index support


## 3.0.7

Released: October 12, 2015

(Packaged from [e161253](https://github.com/mapnik/mapnik/commit/e161253))

 - Removed `MAPNIK_VERSION_IS_RELEASE` define / `mapnik-config --version` not longer reports `-pre` for non-release versions.
   Use `mapnik-config --git-revision` instead ([#3123](https://github.com/mapnik/mapnik/issues/3123))
 - Renamed `nik2img` command to `mapnik-render`
 - PostGIS: Fixed handling of all attributes when `key_field_as_attribute=false` ([#3120](https://github.com/mapnik/mapnik/issues/3120))
 - PostGIS: Fixed parsing of `key_field_as_attribute` as boolean: now `true/false` can be used in addition to `0/1`


## 3.0.6

Released: October 7, 2015

(Packaged from [3cebe97](https://github.com/mapnik/mapnik/commit/3cebe97))

- PostGIS plugin: added `key_field_as_attribute` option. Defaults to `True` to preserve current behavior of having the `key_field` added both
  as an attribute and as the `feature.id` value. If `key_field_as_attribute=false` is passed then the attribute is discarded ([#3115](https://github.com/mapnik/mapnik/issues/3115))
- CSV plugin has been further optimized and has gained experimental support for on-disk indexes ([#3089](https://github.com/mapnik/mapnik/issues/3089))
- SVG parser now fallsback to using `viewbox` if explicit dimensions are lacking ([#3081](https://github.com/mapnik/mapnik/issues/3081))
- Visual tests: new command line arguments `--agg`, `--cairo`, `--svg`, `--grid` for selecting renderers ([#3074](https://github.com/mapnik/mapnik/pull/3074))
- Visual tests: new command line argument `--scale-factor` or abbreviated `-s` for setting scale factor ([#3074](https://github.com/mapnik/mapnik/pull/3074))
- Fixed parsing colors in hexadecimal notation ([#3075](https://github.com/mapnik/mapnik/pull/3075))
- Removed mapnik::Feature type alias of mapnik::feature_impl ([#3099](https://github.com/mapnik/mapnik/pull/3099))
- Fixed linking order for plugins to avoid possible linking errors on linux systems ([#3105](https://github.com/mapnik/mapnik/issues/3105))


## 3.0.5

Released: September 16, 2015

(Packaged from [165c704](https://github.com/mapnik/mapnik/commit/165c704))

- `scale-hsla` image filter: parameters are no longer limited by interval \[0, 1\] ([#3054](https://github.com/mapnik/mapnik/pull/3054))
- Windows: Fixed SVG file loading from unicode paths
- `colorize-alpha` image filter: fixed normalization of color components ([#3058](https://github.com/mapnik/mapnik/pull/3058))
- `colorize-alpha` image filter: added support for transparent colors ([#3061](https://github.com/mapnik/mapnik/pull/3061))
- Enable reading optional `MAPNIK_LOG_FORMAT` environment variable([6d1ffc8](https://github.com/mapnik/mapnik/commit/6d1ffc8a93008b8c0a89d87d68b59afb2cb3757f))
- CSV.input uses memory mapped file by default on \*nix.
- Updated bundled fonts to the latest version
- Topojson.input - fixed geometry_index logic which was causing missing features
- Fixed SVG file loading from unicode paths ([mapnik/node-mapnik#517](https://github.com/mapnik/node-mapnik/issues/517))
- CSV.input - improved support for LF/CR/CRLF line endings on all platforms ([#3065](https://github.com/mapnik/mapnik/issues/3065))
- Revive `zero allocation image interface` and add unit tests
- Benchmark: use return values of test runner.


## 3.0.4

Released: August 26, 2015

(Packaged from [17bb81c](https://github.com/mapnik/mapnik/commit/17bb81c))

- CSV.input: plug-in has been re-factored to minimise memory usage and to improve handling of larger input.
  (NOTE: [large_csv](https://github.com/mapnik/mapnik/tree/large_csv) branch adds experimental trunsduction parser with deferred string initialisation)
- CSV.input: added internal spatial index (boost::geometry::index::tree) for fast `bounding box` queries ([#3010](https://github.com/mapnik/mapnik/pull/3010))
- Fixed deadlock in recursive datasource registration via [@zerebubuth](https://github.com/zerebubuth) ([#3038](https://github.com/mapnik/mapnik/pull/3038))
- Introduced new command line argument `--limit` or `-l` to limit number of failed tests via [@talaj](https://github.com/talaj) ([#2996](https://github.com/mapnik/mapnik/pull/2996))


## 3.0.3

Released: August 12, 2015

(Packaged from [3d262c7](https://github.com/mapnik/mapnik/commit/3d262c7))

- Fixed an issue with fields over size of `int32` in `OGR` plugin ([mapnik/node-mapnik#499](https://github.com/mapnik/node-mapnik/issues/499))
- Added 3 new image-filters to simulate types of colorblindness (`color-blind-protanope`,`color-blind-deuteranope`,`color-blind-tritanope`)
- Fix so that null text boxes have no bounding boxes when attempting placement ( [162f82c](https://github.com/mapnik/mapnik/commit/162f82cba5b0fb984c425586c6a4b354917abc47) )
- Patch to add legacy method for setting JPEG quality in images ( [#3024](https://github.com/mapnik/mapnik/issues/3024) )
- Added `filter_image` method which can modify an image in place or return a new image that is filtered
- Added missing typedef's in `mapnik::geometry` to allow experimenting with different containers


## 3.0.2

Released: July 31, 2015

(Packaged from [8305e74](https://github.com/mapnik/mapnik/commit/8305e74))

#### Summary

This release is centered around improvements to the SVG parsing within mapnik. Most work was done in pull request [#3003](https://github.com/mapnik/mapnik/issues/3003).

- Added container to log SVG parsing errors
- Reimplemented to use rapidxml for parsing XML (DOM)
- Support both xml:id and id attributes ( xml:id takes precedence )
- Added parse_id_from_url using boost::spirit
- Added error tracking when parsing doubles
- Unit tests for svg_parser to improve coverage
- Fixed rx/ry validation for rounded_rect
- Fixed dimensions parsing
- Remove libxml2 dependency


## 3.0.1

Released: July 27th, 2015

(Packaged from [28f6f4d](https://github.com/mapnik/mapnik/commit/28f6f4d))

#### Summary

The 3.0.1 fixes a few bugs in geojson parsing, svg parsing, and rendering. It also avoids a potential hang when using `line-geometry-transform` and includes a speedup for text rendering compared to v3.0.0. It is fully back compatible with v3.0.0 and everyone is encouraged to upgrade.

- Fixed text placement performance after [#2949](https://github.com/mapnik/mapnik/issues/2949) ([#2963](https://github.com/mapnik/mapnik/issues/2963))
- Fixed rendering behavior for `text-minimum-path-length` which regressed in 3.0.0 ([#2990](https://github.com/mapnik/mapnik/issues/2990))
- Fixed handling of `xml:id` in SVG parsing ([#2989](https://github.com/mapnik/mapnik/issues/2989))
- Fixed handling of out of range `rx` and `ry` in SVG `rect` ([#2991](https://github.com/mapnik/mapnik/issues/2991))
- Fixed reporting of envelope from `mapnik::memory_datasource` when new features are added ([#2985](https://github.com/mapnik/mapnik/issues/2985))
- Fixed parsing of GeoJSON when unknown properties encountered at `FeatureCollection` level ([#2983](https://github.com/mapnik/mapnik/issues/2983))
- Fixed parsing of GeoJSON when properties contained `{}` ([#2964](https://github.com/mapnik/mapnik/issues/2964))
- Fixed potential hang due to invalid use of `line-geometry-transform` ([6d6cb15](https://github.com/mapnik/mapnik/commit/6d6cb15))
- Moved unmaintained plugins out of core: `osm`, `occi`, and `rasterlite` ([#2980](https://github.com/mapnik/mapnik/issues/2980))


## 3.0.0

Released: July 7th, 2015

(Packaged from [e6891a0](https://github.com/mapnik/mapnik/commit/e6891a0))

#### Summary

The 3.0 release is a major milestone for Mapnik and includes many performance and design improvements. The is the first release to provide text shaping using the harfbuzz library. This harfbuzz support unlocks improved rendering and layer for many new languages, particularly SE Asian scripts. The internal storage for working with images and geometries has been made more flexible, faster, and strongly typed. The python bindings that were previously bundled with Mapnik have now been moved to <https://github.com/mapnik/python-mapnik> and are versioned independently.

#### Notice

 - Mapnik 3.0.0 requires a compiler capable of `std=c++11`.
 - It is highly recommended you use the `clang++` compiler on both OS X and Linux since it has robust c++11 support lower memory requirements.

##### Major Changes

- Improved support for International Text (now uses harfbuzz library for text shaping)

- Uses latest C++11 features for better performance (especially map loading)

- Expressions everywhere: all symbolizer properties can now be data driven expressions (with the exception of `face-name` and `fontset-name` on the `TextSymbolizer`).

- Rewritten geometry storage based on `std::vector` ([#2739](https://github.com/mapnik/mapnik/issues/2739))
  - Separate storage of polygon exterior rings and interior rings to allow for more robust clipping of parts.
  - Enforces consistent winding order per OGC spec (exterior rings are CCW, interior CW)
  - Reduced memory consumption for layers with many points
  - Ability to adapt Mapnik geometries to boost::geometry operations (in a zero-copy way)
  - Ability to have i/o grammars for json/wkt work on geometries rather than paths for better efficiency and simpler code

- Added new and experimental `dot` symbolizer for fast rendering of points

- New functions supported in expressions: `exp`, `sin`, `cos`, `tan`, `atan`, `abs`.

- New constants supported in expressions: `PI`, `DEG_TO_RAD`, `RAD_TO_DEG`

- Added support for a variety of different grayscale images:
  - `mapnik.imageType.null`
  - `mapnik.imageType.rgba8`
  - `mapnik.imageType.gray8`
  - `mapnik.imageType.gray8s`
  - `mapnik.imageType.gray16`
  - `mapnik.imageType.gray16s`
  - `mapnik.imageType.gray32`
  - `mapnik.imageType.gray32s`
  - `mapnik.imageType.gray32f`
  - `mapnik.imageType.gray64`
  - `mapnik.imageType.gray64s`
  - `mapnik.imageType.gray64f`

- Pattern symbolizers now support SVG input and applying transformations on them dynamically

- Experimental / interface may change: `@variables` can be passed to renderer and evaluated in expressions

- Supports being built with clang++ using `-fvisibility=hidden -flto` for smaller binaries

- Supports being built with Visual Studio 2014 CTP \#3

- Shield icons are now pixel snapped for crisp rendering

- `MarkersSymbolizer` now supports `avoid-edges`, `offset`, `geometry-transform`, `simplify` for `line` placement and two new `placement` options called `vertex-last` and `vertex-first` to place a single marker at the end or beginning of a path. Also `clip` is now respected when rendering markers on a LineString
geometry.

- `TextSymbolizer` now supports `smooth`, `simplify`, `halo-opacity`, `halo-comp-op`, and `halo-transform`

- `ShieldSymbolizer` now supports `smooth`, `simplify`, `halo-opacity`, `halo-comp-op`, and `halo-transform`

- The `text-transform` property of `TextSymbolizer` now supports `reverse` value to flip direction of text.

- The `TextSymbolizer` now supports `font-feature-settings` for advanced control over Opentype font rendering (https://developer.mozilla.org/en-US/docs/Web/CSS/font-feature-settings)

- New GroupSymbolizer for applying multiple symbolizers in a single layout

- AGG renderer: fixed geometry offsetting to work after smoothing to produce more consistent results ([#2202](https://github.com/mapnik/mapnik/issues/2202))

- AGG renderer: increased `vertex_dist_epsilon` to ensure nearly coincident points are discarded more readily ([#2196](https://github.com/mapnik/mapnik/issues/2196))

- GDAL plugin
  - Now keeps datasets open for the lifetime of the datasource (rather than per featureset)
  - Added back support for user driven `nodata` on rgb(a) images ([#2023](https://github.com/mapnik/mapnik/issues/2023))
  - Allowed nodata to override alpha band if set on rgba images ([#2023](https://github.com/mapnik/mapnik/issues/2023))
  - Added `nodata_tolerance` option to set nearby pixels transparent (has similar effect to the `nearblack` program) ([#2023](https://github.com/mapnik/mapnik/issues/2023))
  - At process exit Mapnik core no longer calls `dlclose` on gdal.input ([#2716](https://github.com/mapnik/mapnik/issues/2716))

- TopoJSON plugin
  - Now supporting optional `bbox` property on layer
  - Fixed support for reporting correct `feature.id()`
  - Now supports `inline` string for passing data from memory
  - Faster parsing via static initialization of grammars
  - Fix crash on invalid arc index

- GeoJSON plugin
  - Now supporting optional `bbox` property on layer
  - Fixed support for reporting correct `feature.id()`
  - Now supports `inline` string for passing data from memory
  - Faster parsing via static initialization of grammars

- SQLite plugin
  - Fixed support for handling all column types

- CSV Plugin
  - Added the ability to pass an `extent` in options

- PostGIS plugin
  - Added Async support to  - https://github.com/mapnik/mapnik/wiki/PostGIS-Async
  - Added support for rendering 3D and 4D geometries (previously silently skipped) ([#44](https://github.com/mapnik/mapnik/issues/44))

- Added support for web fonts: `.woff` format ([#2113](https://github.com/mapnik/mapnik/issues/2113))

- Added missing support for `geometry-transform` in `line-pattern` and `polygon-pattern` symbolizers ([#2065](https://github.com/mapnik/mapnik/issues/2065))

- Dropped support for Sun compiler

- Upgraded unifont to `unifont-6.3.20131020`

- Fixed crash when rendering to cairo context from python ([#2031](https://github.com/mapnik/mapnik/issues/2031))

- Moved `label-position-tolerance` from `unsigned` type to `double`

- Added support for more seamless blurring by rendering to a larger internal image to avoid edge effects ([#1478](https://github.com/mapnik/mapnik/issues/1478))

- Fixed rendering of large shapes at high zoom levels, which might dissapear due to integer overflow. This
  bug was previously fixable when geometries were clipped, but would, until now, re-appear if clipping was turned
  off for a symbolizer ([#2000](https://github.com/mapnik/mapnik/issues/2000))

- Added single color argument support to `colorize-alpha` to allow colorizing alpha with one color.

- Added `color-to-alpha` `image-filter` to allow for applying alpha in proportion to color similiarity ([#2023](https://github.com/mapnik/mapnik/issues/2023))

- Fixed alpha handling bug with `comp-op:dst-over` ([#1995](https://github.com/mapnik/mapnik/issues/1995))

- Fixed alpha handling bug with building-fill-opacity ([#2011](https://github.com/mapnik/mapnik/issues/2011))

- Optimized mapnik.Path.to_wkb

- Python: added `__geo_interface__` to mapnik.Feature and mapnik.Path ([#2009](https://github.com/mapnik/mapnik/issues/2009))

- Python: Exposed optimized WKTReader for parsing WKT into geometry paths ([6bfbb53](https://github.com/mapnik/mapnik/commit/6bfbb53))

- Optimized expression evaluation of text by avoiding extra copy ([1dd1275](https://github.com/mapnik/mapnik/commit/1dd1275))

- Added Map level `background-image-comp-op` to control the compositing operation used to blend the
`background-image` onto the `background-color`. Has no meaning if `background-color` or `background-image`
are not set. ([#1966](https://github.com/mapnik/mapnik/issues/1966))

- Added Map level `background-image-opacity` to dynamically set the opacity of the `background-image` ([#1966](https://github.com/mapnik/mapnik/issues/1966))

- Removed `RENDERING_STATS` compile option since it should be replaced with a better solution ([#1956](https://github.com/mapnik/mapnik/issues/1956))

- Added support to experimental `svg_renderer` for grouping layers for inkscape and illustrator ([#1917](https://github.com/mapnik/mapnik/issues/1917))

- Fixed compile of python bindings against Python 3.x

- Optimized SVG loading by improving color parsing speed ([#1918](https://github.com/mapnik/mapnik/issues/1918))

- Fixed startup problem when fonts cannot be read due to lacking permissions ([#1919](https://github.com/mapnik/mapnik/issues/1919))

- Fixed bad behavior when negative image dimensions are requested ([#1927](https://github.com/mapnik/mapnik/issues/1927))

- Fixed handling of `marker-ignore-placement:true` when `marker-placement:line` ([#1931](https://github.com/mapnik/mapnik/issues/1931))

- Fixed handling of svg `opacity` in Cairo renderer ([#1943](https://github.com/mapnik/mapnik/issues/1943))

- Fixed handling of SVG files which contain empty `<g>` ([#1944](https://github.com/mapnik/mapnik/issues/1944))

- Fixed various 32bit test failures

- Fixed compile against icu when by using `U_NAMESPACE_QUALIFIER`

- Fixed missing support for using PathExpression in `marker-file` ([#1952](https://github.com/mapnik/mapnik/issues/1952))

- Added support for `line-pattern-offset` ([#1991](https://github.com/mapnik/mapnik/issues/1991))

- Added support for building on Android (tested with `android-ndk-r9`)

- Added support for compiling with both -ansi (aka -std=c++98) and -std=c++11

- Added support for compiling and linking on OS X against libc++

- Fixed regression in handling `F` type dbf fields, introduced in v2.2.0.

- Added the ability to create a mapnik Feature from a geojson feature with `mapnik.Feature.from_geojson` in python.

- Added to python bindings: `has_tiff`, `has_png`, `has_webp`, `has_proj4`, `has_svg_renderer`, and `has_grid_renderer`

- Made it possible to disable compilation of `grid_renderer` with `./configure GRID_RENDERER=False` ([#1962](https://github.com/mapnik/mapnik/issues/1962))

- Added `premultiplied` property on mapnik::image_32 / mapnik.Image to enable knowledge of premultiplied status of image buffer.

- Added `webp` image encoding and decoding support ([#1955](https://github.com/mapnik/mapnik/issues/1955))

- Added `scale-hsla` image-filter that allows scaling colors in HSL color space. RGB is converted to HSL (hue-saturation-lightness) and then each value (and the original alpha value) is stretched based on the specified scaling values. An example syntax is `scale-hsla(0,1,0,1,0,1,0,1)` which means no change because the full range will be kept (0 for lowest, 1 for highest). Other examples are: 1) `scale-hsla(0,0,0,1,0,1,0,1)` which would force all colors to be red in hue in the same way `scale-hsla(1,1,0,1,0,1,0,1)` would, 2) `scale-hsla(0,1,1,1,0,1,0,1)` which would cause all colors to become fully saturated, 3) `scale-hsla(0,1,1,1,0,1,.5,1)` which would force no colors to be any more transparent than half, and 4) `scale-hsla(0,1,1,1,0,1,0,.5)` which would force all colors to be at least half transparent. ([#1954](https://github.com/mapnik/mapnik/issues/1954))

- The `shapeindex` tool now works correctly with point 3d geometry types


## 2.2.0

Released June 3rd, 2013

(Packaged from [9231205](https://github.com/mapnik/mapnik/commit/9231205))

Summary: The 2.2.0 release is primarily a performance and stability release. The code line represents development in the master branch since the release of 2.1.0 in Aug 2012 and therefore includes nearly a year of bug-fixes and optimizations. Nearly 500 new tests have been added bring the total coverage to 925. Shapefile and PostGIS datasources have benefited from numerous stability fixes, 64 bit integer support has been added to support OSM data in the grid renderer and in attribute filtering, and many fixes have landed for higher quality output when using a custom `scale_factor` during rendering. Critical code paths have been optimized include raster rendering, xml map loading, string to number conversion, vector reprojection when using `epsg:4326` and `epsg:3857`, `hextree` encoding, halo rendering, and rendering when using a custom `gamma`. Mapnik 2.2 also compiles faster than previous releases in the 2.x series and drops several unneeded and hard to install dependencies making builds on OS X and Windows easier than any previous release.

- Removed 3 dependencies without loosing any functionality: `ltdl`, `cairomm` and `libsigc++` ([#1804](https://github.com/mapnik/mapnik/issues/1804),[#806](https://github.com/mapnik/mapnik/issues/806),[#1681](https://github.com/mapnik/mapnik/issues/1681))

- Added 64 bit integer support in expressions, feature ids, and the grid_renderer ([#1661](https://github.com/mapnik/mapnik/issues/1661),[#1662](https://github.com/mapnik/mapnik/issues/1662),[#1662](https://github.com/mapnik/mapnik/issues/1662))

- Added the ability to disable the need for various dependencies: `proj4`, `libpng`, `libtiff`, `libjpeg`

- Added faster reprojection support between `epsg:3857` and `epsg:4326` ([#1705](https://github.com/mapnik/mapnik/issues/1705),[#1703](https://github.com/mapnik/mapnik/issues/1703),[#1579](https://github.com/mapnik/mapnik/issues/1579))

- Added `colorize-alpha` image filter that applies user provided color gradients based on level of alpha.
  Accepts one or more colors separated by commas. Each color can be paired with an `offset` value separated
  by a space that is either `0-100%` or `0.0-1.0`. An `offset` of `0` is implied and the default. For background
  on where this design came from see http://www.w3.org/TR/SVG/pservers.html#GradientStops. A simple example
  of colorizing alpha into a "rainbow" is `colorize-alpha(blue,cyan,lightgreen, yellow, orange, red)`. An example of
  using offsets and the variety of supported color encodings is to produce a ramp which sharp contrast between `blue`
  and `cyan` is `colorize-alpha(blue 30%, cyan, yellow 0.7 , rgb(0%,80%,0%) 90%)` ([#1371](https://github.com/mapnik/mapnik/issues/1371)).

- Fixed concurrency problem when using cursors in postgis plugin ([#1823](https://github.com/mapnik/mapnik/issues/1823),[#1588](https://github.com/mapnik/mapnik/issues/1588))

- Fixed postgres connection pool leaks when using `persist_connection=false` ([#1764](https://github.com/mapnik/mapnik/issues/1764))

- Fixed postgres connection key to respect highest value of `max_size` and `initial_size` for any layer in map ([#1599](https://github.com/mapnik/mapnik/issues/1599))

- Fixed potential crash in wkb parsing when postgis returns null geometry ([#1843](https://github.com/mapnik/mapnik/issues/1843))

- Fixed blurry rendering of image and SVG icons ([#1316](https://github.com/mapnik/mapnik/issues/1316))

- Added detection of invalid srs values when loading xml ([#646](https://github.com/mapnik/mapnik/issues/646))

- Added support for specifying a base_path as a third, optional argument to load_xml

- Removed muffling of projection errors while rendering ([#646](https://github.com/mapnik/mapnik/issues/646))

- Improved logging system (https://github.com/mapnik/mapnik/wiki/Logging)

- Added support for reading images from in memory streams ([#1805](https://github.com/mapnik/mapnik/issues/1805))

- Optimized halo rendering. When halo radius is < 1 new method will be used automatically ([#1781](https://github.com/mapnik/mapnik/issues/1781))

- Added `text-halo-rasterizer` property. Set to `fast` for lower quality but faster
  halo rendering ([#1298](https://github.com/mapnik/mapnik/issues/1298)) which matched new default method when radius is < 1.

- Added support in `shape`, `sqlite`, `geojson`, and `csv` plugin for handling non-latin characters in the paths to file-based resources ([#1177](https://github.com/mapnik/mapnik/issues/1177))

- Fixed rendering of markers when their size is greater than the specified `spacing` value ([#1487](https://github.com/mapnik/mapnik/issues/1487))

- Fixed handling of alpha premultiplication in image scaling ([#1489](https://github.com/mapnik/mapnik/issues/1489))

- Optimized rendering when a style with no symbolizers is encountered ([#1517](https://github.com/mapnik/mapnik/issues/1517))

- Optimized string handling and type conversion by removing `boost::to_lower`, `boost::trim`, and `boost::lexical_cast` usage ([#1687](https://github.com/mapnik/mapnik/issues/1687),[#1687](https://github.com/mapnik/mapnik/issues/1687),[#1633](https://github.com/mapnik/mapnik/issues/1633))

- Optimized alpha preserving `hextree` method for quantization of png images ([#1629](https://github.com/mapnik/mapnik/issues/1629))

- Faster rendering of rasters by reducing memory allocation of temporary buffers ([#1516](https://github.com/mapnik/mapnik/issues/1516))

- Fixed some raster reprojection artifacts ([#1501](https://github.com/mapnik/mapnik/issues/1501))

- Fixed raster alignment when width != height and raster is being scaled ([#1748](https://github.com/mapnik/mapnik/issues/1748),[#1622](https://github.com/mapnik/mapnik/issues/1622))

- Added support for caching rasters for re-use during rendering when styling more than once per layer ([#1543](https://github.com/mapnik/mapnik/issues/1543))

- Improved compile speeds of the code - in some cases by up to 2x and removed need for freetype dependency when building code against mapnik ([#1688](https://github.com/mapnik/mapnik/issues/1688), [#1756](https://github.com/mapnik/mapnik/issues/1756))

- Removed internal rule cache on `mapnik::Map` c++ object ([#1723](https://github.com/mapnik/mapnik/issues/1723))

- Improved the scaled rendering of various map features when using `scale_factor` > 1 ([#1280](https://github.com/mapnik/mapnik/issues/1280),[#1100](https://github.com/mapnik/mapnik/issues/1100),[#1273](https://github.com/mapnik/mapnik/issues/1273),[#1792](https://github.com/mapnik/mapnik/issues/1792),[#1291](https://github.com/mapnik/mapnik/issues/1291),[#1344](https://github.com/mapnik/mapnik/issues/1344),[#1279](https://github.com/mapnik/mapnik/issues/1279),[#1624](https://github.com/mapnik/mapnik/issues/1624),[#1767](https://github.com/mapnik/mapnik/issues/1767),[#1766](https://github.com/mapnik/mapnik/issues/1766))

- Added C++ api for overriding scale_denominator to enable rendering at fixed scale ([#1582](https://github.com/mapnik/mapnik/issues/1582))

- Added Layer `buffer-size` that can be used to override Map `buffer-size` to avoid
  over-fetching of data that does not need to be buffered as much as other layers.
  Map level `buffer-size` will be default if layers do not set the option. Renamed a
  previously undocumented parameter by the same name that impacted clipping extent and
  was not needed (clipping padding should likely be a symbolizer level option) ([#1566](https://github.com/mapnik/mapnik/issues/1566))

- Fixed potential file descriptor leaks in image readers when invalid images were encountered ([#1783](https://github.com/mapnik/mapnik/issues/1783))

- Fixed alpha handling in the `blur` and `invert` image filters ([#1541](https://github.com/mapnik/mapnik/issues/1541))

- Fixed error reporting in the python plugin ([#1422](https://github.com/mapnik/mapnik/issues/1422))

- Added the ability to run tests without installing with `make test-local`

- Reduced library binary size by adding support for `-fvisibility-inlines-hidden` and `-fvisibility=hidden` ([#1826](https://github.com/mapnik/mapnik/issues/1826),[#1832](https://github.com/mapnik/mapnik/issues/1832))

- Added `mapnik::map_request` class, a special object to allow passing mutable map objects to renderer ([#1737](https://github.com/mapnik/mapnik/issues/1737))

- Added the ability to use `boost::hash` on `mapnik::value` types ([#1729](https://github.com/mapnik/mapnik/issues/1729))

- Removed obsolete `geos` plugin (functionality replaced by `csv` plugin) and unmaintained `kismet` plugin ([#1809](https://github.com/mapnik/mapnik/issues/1809),[#1833](https://github.com/mapnik/mapnik/issues/1833))

- Added new `mapnik-config` flags: `--all-flags`, `--defines`, `--git-describe`, `--includes`, `--dep-includes`, `--cxxflags`, `--cxx` ([#1443](https://github.com/mapnik/mapnik/issues/1443))

- Added support for unicode strings as arguments in python bindings ([#163](https://github.com/mapnik/mapnik/issues/163))

- Added DebugSymbolizer which is able to render the otherwise invisible collision boxes ([#1366](https://github.com/mapnik/mapnik/issues/1366))

- Optimized rendering by reducing overhead of using `gamma` property ([#1174](https://github.com/mapnik/mapnik/issues/1174))

- Fixed rendering artifacts when using `polygon-gamma` or `line-gamma` equal to 0 ([#761](https://github.com/mapnik/mapnik/issues/761),[#1763](https://github.com/mapnik/mapnik/issues/1763))

- Fixed and optimized the display of excessive precision of some float data in labels ([#430](https://github.com/mapnik/mapnik/issues/430),[#1697](https://github.com/mapnik/mapnik/issues/1697))

- Removed the `bind` option for datasources ([#1654](https://github.com/mapnik/mapnik/issues/1654))

- Added ability to access style list from map by (name,obj) in python ([#1725](https://github.com/mapnik/mapnik/issues/1725))

- Added `is_solid` method to python mapnik.Image and mapnik.ImageView classes ([#1728](https://github.com/mapnik/mapnik/issues/1728))

- Changed scale_denominator C++ interface to take scale as first argument rather than map.

- Added support for `background-image` in cairo_renderer ([#1724](https://github.com/mapnik/mapnik/issues/1724))

- Fixed building symbolizer rendering to be fully sensitive to alpha ([8b66128](https://github.com/mapnik/mapnik/commit/8b66128c892), [bc8ea1c](https://github.com/mapnik/mapnik/commit/bc8ea1c5a7a))

- `<Filter>[attr]</Filter>` now returns false if attr is an empty string ([#1665](https://github.com/mapnik/mapnik/issues/1665))

- `<Filter>[attr]!=null</Filter>` now returns true if attr is not null ([#1642](https://github.com/mapnik/mapnik/issues/1642))

- Added support for DBF `Logical` type: [#1614](https://github.com/mapnik/mapnik/issues/1614)

- Added serialization of `line-offset` to save_map ([#1562](https://github.com/mapnik/mapnik/issues/1562))

- Enabled default input plugin directory and fonts path to be set inherited from environment settings in
  python bindings to make it easier to run tests locally ([#1594](https://github.com/mapnik/mapnik/issues/1594)). New environment settings are:
    - MAPNIK_INPUT_PLUGINS_DIRECTORY
    - MAPNIK_FONT_DIRECTORY

- Added support for controlling rendering behavior of markers on multi-geometries `marker-multi-policy` ([#1555](https://github.com/mapnik/mapnik/issues/1555),[#1573](https://github.com/mapnik/mapnik/issues/1573))

- Added alternative PNG/ZLIB implementation (`miniz`) that can be enabled with `e=miniz` ([#1554](https://github.com/mapnik/mapnik/issues/1554))

- Added support for setting zlib `Z_FIXED` strategy with format string: `png:z=fixed`

- Fixed handling of transparency level option in `octree` png encoding ([#1556](https://github.com/mapnik/mapnik/issues/1556))

- Added ability to pass a pre-created collision detector to the cairo renderer ([#1444](https://github.com/mapnik/mapnik/issues/1444))

- Tolerance parameter is now supported for querying datasources at a given point ([#503](https://github.com/mapnik/mapnik/issues/503)/[#1499](https://github.com/mapnik/mapnik/issues/1499))

- Improved detection of newlines in CSV files - now more robust in the face of mixed newline types ([#1497](https://github.com/mapnik/mapnik/issues/1497))

- Allow style level compositing operations to work outside of featureset extents across tiled requests ([#1477](https://github.com/mapnik/mapnik/issues/1477))

- Support for encoding `literal` postgres types as strings [69fb17c](https://github.com/mapnik/mapnik/commit/69fb17cd3)/[#1466](https://github.com/mapnik/mapnik/issues/1466)

- Fixed zoom_all behavior when Map maximum-extent is provided. Previously maximum-extent was used outright but
  now the combined layer extents will be again respected: they will be clipped to the maximum-extent if possible
  and only when back-projecting fails for all layers will the maximum-extent be used as a fallback ([#1473](https://github.com/mapnik/mapnik/issues/1473))

- Compile time flag called `PLUGIN_LINKING` to allow input datasource plugins to be statically linked with the mapnik library ([#249](https://github.com/mapnik/mapnik/issues/249))

- Fixed `dasharray` rendering in cairo backend ([#1740](https://github.com/mapnik/mapnik/issues/1740))

- Fixed handling of `opacity` in svg rendering ([#1744](https://github.com/mapnik/mapnik/issues/1744))

- Fixed uneven rendering of markers along lines ([#1693](https://github.com/mapnik/mapnik/issues/1693))

- Fixed handling of extra bytes in some shapefile fields ([#1605](https://github.com/mapnik/mapnik/issues/1605))

- Fixed handling (finally) of null shapes and partially corrupt shapefiles ([#1630](https://github.com/mapnik/mapnik/issues/1630),[#1621](https://github.com/mapnik/mapnik/issues/1621))

- Added ability to re-use `mapnik::image_32` and `mapnik::grid` by exposing a `clear` method ([#1571](https://github.com/mapnik/mapnik/issues/1571))

- Added support for writing RGB (no A) png images by using the format string of `png:t=0` ([#1559](https://github.com/mapnik/mapnik/issues/1559))

- Added experimental support for geometry simplification at symbolizer level ([#1385](https://github.com/mapnik/mapnik/issues/1385))

## Mapnik 2.1.0

Released Aug 23, 2012

(Packaged from [a25aac8](https://github.com/mapnik/mapnik/commit/a25aac8))

- Feature-level compositing (comp-op) for all symbolizers (except building) in AGG and Cairo renderers ([#1409](https://github.com/mapnik/mapnik/issues/1409))

- Style-level compositing (comp-op) ([#1409](https://github.com/mapnik/mapnik/issues/1409)) and style-level opacity for AGG renderer ([#314](https://github.com/mapnik/mapnik/issues/314))

- New experimental framework for image manipulation called `image-filters` to allow things to be done across entire layer canvas like burring ([#1412](https://github.com/mapnik/mapnik/issues/1412))

- Support for recoloring stroke, fill, and opacity of SVG files ([#1410](https://github.com/mapnik/mapnik/issues/1410) / [#659](https://github.com/mapnik/mapnik/issues/659))

- Support for data-driven transform expressions ([#664](https://github.com/mapnik/mapnik/issues/664))

- New support for offsetting geometries / parallel lines in line_symbolizer ([#927](https://github.com/mapnik/mapnik/issues/927)/[#1269](https://github.com/mapnik/mapnik/issues/1269))

- New support for clipping geometries - now default enabled on all symbolizers ([#1116](https://github.com/mapnik/mapnik/issues/1116))

- Framework for chainable geometry transformations (called `vertex_converters`) so that you can do things like clip, smooth, and offset at the same time ([#927](https://github.com/mapnik/mapnik/issues/927))

- WKT parsing now is more robust and supports multi-geometries ([#745](https://github.com/mapnik/mapnik/issues/745))

- New support for outputting WKT/WKB/GeoJSON/SVG from mapnik.Geometry objects ([#1411](https://github.com/mapnik/mapnik/issues/1411))

- New experimental python datasource plugin ([#1337](https://github.com/mapnik/mapnik/issues/1337))

- New experimental geojson datasource plugin using in-memory rtree indexing ([#1413](https://github.com/mapnik/mapnik/issues/1413))

- Cairo rendering is now much more similiar to AGG rendering as cairo backend now supports `scale_factor` ([#1280](https://github.com/mapnik/mapnik/issues/1280)) and other fixed have landed ([#1343](https://github.com/mapnik/mapnik/issues/1343), [#1233](https://github.com/mapnik/mapnik/issues/1233), [#1344](https://github.com/mapnik/mapnik/issues/1344), [#1242](https://github.com/mapnik/mapnik/issues/1242), [#687](https://github.com/mapnik/mapnik/issues/687), [#737](https://github.com/mapnik/mapnik/issues/737), [#1006](https://github.com/mapnik/mapnik/issues/1006), [#1071](https://github.com/mapnik/mapnik/issues/1071))

- mapnik::Feature objects and datasource plugins now use a `Context` to store attribute schemas to reduce the memory footprint of features ([#834](https://github.com/mapnik/mapnik/issues/834))

- Added Stroke `miterlimit` ([#786](https://github.com/mapnik/mapnik/issues/786))

- Python: exposed Map `background_image` (and aliased `background` to `background_color`)

- Python: exposed BuildingSymbolizer

- Support in the CSV plugin for reading JSON encoded geometries ([#1392](https://github.com/mapnik/mapnik/issues/1392))

- Increased grid encoding performance ([#1315](https://github.com/mapnik/mapnik/issues/1315))

- Added support for setting opacity dynamically on images in polygon pattern and markers symbolizers

- Added support for filtering on a features geometry type, either `point`, `linestring`, `polygon`,
  or `collection` using the expression keyword of `[mapnik::geometry_type]` ([#546](https://github.com/mapnik/mapnik/issues/546))

- MarkersSymbolizer width and height moved to expressions ([#1102](https://github.com/mapnik/mapnik/issues/1102))

- PostGIS: Added `simplify_geometries` option - will trigger ST_Simplify on geometries before returning to Mapnik ([#1179](https://github.com/mapnik/mapnik/issues/1179))

- Improved error feedback for invalid values passed to map.query_point

- Fixed rendering of thin svg lines ([#1129](https://github.com/mapnik/mapnik/issues/1129))

- Improved logging/debugging system with release logs and file redirection (https://github.com/mapnik/mapnik/wiki/Runtime-Logging) ([#937](https://github.com/mapnik/mapnik/issues/937) and partially [#986](https://github.com/mapnik/mapnik/issues/986), [#467](https://github.com/mapnik/mapnik/issues/467))

- GDAL: allow setting `nodata` value on the fly (will override value if `nodata` is set in data) ([#1161](https://github.com/mapnik/mapnik/issues/1161))

- GDAL: respect `nodata` for paletted/colormapped images ([#1160](https://github.com/mapnik/mapnik/issues/1160))

- PostGIS: Added a new option called `autodetect_key_field` (by default false) that if true will
  trigger autodetection of the table primary key allowing for feature.id() to represent
  globally unique ids. This option has no effect if the user has not manually supplied the `key_field` option. ([#804](https://github.com/mapnik/mapnik/issues/804))

- Cairo: Add full rendering support for markers to match AGG renderer functionality ([#1071](https://github.com/mapnik/mapnik/issues/1071))

- Fix Markers rendering so that ellipse height/width units are pixels (previously were unintentionally radii) ([#1134](https://github.com/mapnik/mapnik/issues/1134))

- Added `ignore-placement` attribute to markers-symbolizer ([#1135](https://github.com/mapnik/mapnik/issues/1135))

- Removed PointDatasource - use more robust MemoryDatasource instead ([#1032](https://github.com/mapnik/mapnik/issues/1032))

- SQLite - Added support for !intersects! token in sql subselects ([#809](https://github.com/mapnik/mapnik/issues/809)) allow custom positioning of rtree spatial filter.

- New CSV plugin - reads tabular files - autodetecting geo columns, newlines, and delimiters. Uses in-memory featureset for fast rendering and is not designed for large files ([#902](https://github.com/mapnik/mapnik/issues/902))

- Fixed bug in shield line placement when dx/dy are used to shift the label relative to the placement point (Matt Amos) ([#908](https://github.com/mapnik/mapnik/issues/908))

- Added <layer_by_sql> parameter in OGR plugin to select a layer by SQL query (besides name or index): see http://www.gdal.org/ogr/ogr_sql.html for specifications (kunitoki) ([#472](https://github.com/mapnik/mapnik/issues/472))

- Added support for output maps as tiff files (addresses [#967](https://github.com/mapnik/mapnik/issues/967) partially)

- Added support for justify-alignment=auto. This is the new default. ([#1125](https://github.com/mapnik/mapnik/issues/1125))

- Added support for grouped rendering using the `group-by` layer option: https://github.com/mapnik/mapnik/wiki/Grouped-rendering


## Mapnik 2.0.2

Released Aug 3, 2012

(Packaged from [adb2ec7](https://github.com/mapnik/mapnik/commit/adb2ec741))

- Fixed handling of empty WKB geometries ([#1334](https://github.com/mapnik/mapnik/issues/1334))

- Fixed naming of `stroke-dashoffset` in save_map ([cc3cd5f](https://github.com/mapnik/mapnik/commit/cc3cd5f63f28))

- Fixed support for boost 1.50 ([8dea5a5](https://github.com/mapnik/mapnik/commit/8dea5a5fe239233))

- Fixed TextSymbolizer placement in Cairo backend so it respects avoid-edges and minimum-padding across all renderers ([#1242](https://github.com/mapnik/mapnik/issues/1242))

- Fixed ShieldSymbolizer placement so it respects avoid-edges and minimum-padding across all renderers ([#1242](https://github.com/mapnik/mapnik/issues/1242))

- Rolled back change made in 2.0.1 to marker width/height meaning that Mapnik > 2.0.2 will stick to assuming width/heigh are radii for back compatibility with 2.0.0. The reverted change is seen below as "Fix Markers rendering so that ellipse height/width units are pixels (previously were unintentionally radii)". Issue tracking this is [#1163](https://github.com/mapnik/mapnik/issues/1163)

- XML: Fixed to avoid throwing if a `<Parameters>` element is encountered (which is supported in >= 2.1.x)

- Support for PostGIS 2.0 in the pgsql2sqlite command ([e69c44e](https://github.com/mapnik/mapnik/commit/e69c44e), [47e5b3c](https://github.com/mapnik/mapnik/commit/47e5b3c))

- Fixed reference counting of Py_None when returning null attributes from Postgres during UTFGrid encoding, which could cause a Fatal Python error: deallocating None ([#1221](https://github.com/mapnik/mapnik/issues/1221))

- Fixed possible breakage registering plugins via python if a custom PREFIX or DESTDIR was used (e.g. macports/homebrew) ([#1171](https://github.com/mapnik/mapnik/issues/1171))

- Fixed memory leak in the case of proj >= 4.8 and a projection initialization error ([#1173](https://github.com/mapnik/mapnik/issues/1173))


## Mapnik 2.0.1

Released April 10, 2012

(Packaged from [57347e9](https://github.com/mapnik/mapnik/commit/57347e9106))

- Support for PostGIS 2.0 ([#956](https://github.com/mapnik/mapnik/issues/956),[#1083](https://github.com/mapnik/mapnik/issues/1083))

- Switched back to "libmapnik" and "import mapnik" rather than "mapnik2" (mapnik2 will still work from python) ([#941](https://github.com/mapnik/mapnik/issues/941))

- Restored Python 2.5 compatibility ([#904](https://github.com/mapnik/mapnik/issues/904))

- Fixed `mapnik-config --version` ([#903](https://github.com/mapnik/mapnik/issues/903))

- Cairo: Add full rendering support for markers to match AGG renderer functionality ([#1071](https://github.com/mapnik/mapnik/issues/1071))

- Fix Markers rendering so that ellipse height/width units are pixels (previously were unintentially radii) ([#1134](https://github.com/mapnik/mapnik/issues/1134))

- Added `ignore-placement` attribute to markers-symbolizer ([#1135](https://github.com/mapnik/mapnik/issues/1135))

- Removed svn_revision info from mapnik-config and python bindings as git is now used

- Removed OGCServer from core - now at https://github.com/mapnik/OGCServer ([e7f6267](https://github.com/mapnik/mapnik/commit/e7f6267))

- Fixed SQLite open stability across platforms/versions ([#854](https://github.com/mapnik/mapnik/issues/854))

- Workaround for boost interprocess compile error with recent gcc versions ([#950](https://github.com/mapnik/mapnik/issues/950),[#1001](https://github.com/mapnik/mapnik/issues/1001),[#1082](https://github.com/mapnik/mapnik/issues/1082))

- Fix possible memory corruption when using `hextree` mode for png color reduction ([#1087](https://github.com/mapnik/mapnik/issues/1087))

- Fixed bug in shield line placement when dx/dy are used to shift the label relative to the placement point (Matt Amos) ([#908](https://github.com/mapnik/mapnik/issues/908))

- Fix to avoid modifying a feature if an attribute is requested that does not exist ([0f5ab18](https://github.com/mapnik/mapnik/commit/0f5ab18ed))

- Fixed ability to save to jpeg format from python ([7387afd](https://github.com/mapnik/mapnik/commit/7387afd96)) ([#896](https://github.com/mapnik/mapnik/issues/896))


## Mapnik 2.0.0

Released September 26, 2011

(Packaged from [5b4c20e](https://github.com/mapnik/mapnik/commit/5b4c20eab3))

- Add minimum-path-length property to text_symbolizer to allow labels to be placed only on lines of a certain length ([#865](https://github.com/mapnik/mapnik/issues/865))

- Add support for png quantization using fixed palettes ([#843](https://github.com/mapnik/mapnik/issues/843))

- Add AlsoFilter functionality - https://github.com/mapnik/mapnik/wiki/AlsoFilter

- SQLite Plugin: optimize i/o using shared cache and no mutexes ([#797](https://github.com/mapnik/mapnik/issues/797))

- Directly link input plugins to libmapnik to avoid having to set dlopen flags from binding languages ([#790](https://github.com/mapnik/mapnik/issues/790))

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

- Support for `nodata` values with grey and rgb images in GDAL plugin ([#727](https://github.com/mapnik/mapnik/issues/727))

- Print warning if invalid XML property names are used ([#110](https://github.com/mapnik/mapnik/issues/110))

- Made XML property names use consistent dashes, never underscores ([#644](https://github.com/mapnik/mapnik/issues/644))

- Added support for drawing only first matching rule using filter-mode="first" in Style ([#706](https://github.com/mapnik/mapnik/issues/706))

- Added support to PointSymbolizer (`ignore_placement`) for skipping adding placed points to collision detector ([#564](https://github.com/mapnik/mapnik/issues/564))

- Added ability to register fonts within XML using Map level `font-directory` parameter ([#168](https://github.com/mapnik/mapnik/issues/168))

- TextSymbolizer: Change text_convert to text_transform to better match css naming (r2211)

- Shapefile Plugin: Throw error if attribute name is requested that does not exist ([#604](https://github.com/mapnik/mapnik/issues/604))

- Upgraded to the latest proj4 string literal for EPSG:4326 (WGS84) as global default projection ([#333](https://github.com/mapnik/mapnik/issues/333))

- Added `mapnik_version_from_string()` function in python bindings to easily convert string representation
  of version number to the integer format used in `mapnik/version.hpp`. e.g. `0.7.1` --> `701`.

- Added xinclude (http://www.w3.org/TR/xinclude/) support to libxml2-based xml parser (oldtopos) ([#567](https://github.com/mapnik/mapnik/issues/567))

- Optimized rendering speeds by avoiding locking in the projection code (r2063) (r2713)

- Added support for setting global alignment of polygon pattern fills ([#203](https://github.com/mapnik/mapnik/issues/203))

- Added support for choosing OGR layer by index number using `layer_by_index` parameter (r1904)

- Added support for fractional halo widths (using FT Stroker) ([#93](https://github.com/mapnik/mapnik/issues/93))

- Added support for reading jpeg images (in addition to png/tiff) for image symbolizers ([#518](https://github.com/mapnik/mapnik/issues/518))

- Made libjpeg dependency optional at compile time and added mapnik2.has_jpeg() method to check for support in python ([#545](https://github.com/mapnik/mapnik/issues/545)).

- Fixed reading of PostGIS data on Big Endian systems ([#515](https://github.com/mapnik/mapnik/issues/515))

- PostGIS: Added better support for alternative schemas ([#500](https://github.com/mapnik/mapnik/issues/500))

- AGG Renderer - Enforced default gamma function on all symbolizers to ensure proper antialiasing
  even when gamma is modified on the PolygonSymbolizer. ([#512](https://github.com/mapnik/mapnik/issues/512))

- Added ability to read pre 2.0.0 stylesheets, but prints a warning for deprecated syntax (r1592, [#501](https://github.com/mapnik/mapnik/issues/501))

- Rasterlite Plugin: Experimental support for Rasterlite, to practically use sqlite database with wavelet compressed rasters ([#469](https://github.com/mapnik/mapnik/issues/469))

- PNG: fixed png256 for large images and some improvements to reduce color corruptions ([#522](https://github.com/mapnik/mapnik/issues/522))

- Implement MarkersSymbolizer in Cairo render and improve the markers placement finder. ([#553](https://github.com/mapnik/mapnik/issues/553))


# Mapnik 0.7.2

Released Oct 18, 2011

(Packaged from [bc5cabe](https://github.com/mapnik/mapnik/commit/bc5cabeb6a))

- Added forward compatibility for Mapnik 2.0 XML syntax (https://github.com/mapnik/mapnik/wiki/Mapnik2/Changes)

- Build fixes to ensure boost_threads are not used unless THREADING=multi build option is used

- Fixes for the clang compiler

- Support for latest libpng (>= 1.5.x) (r2999)

- Fixes to the postgres pool

- Fix for correct transparency levels in png256/png8 output ([#540](https://github.com/mapnik/mapnik/issues/540))

- Various build system fixes, especially for gcc compiler on open solaris.

- When plugins are not found, report the searched directories ([#568](https://github.com/mapnik/mapnik/issues/568))

- Improved font loading support ([#559](https://github.com/mapnik/mapnik/issues/559))

- Fix to shapeindex for allowing indexing of directory of shapefiles like `shapeindex dir/*shp`

- Fixed handling of null and multipatch shapes in shapefile driver - avoiding inf loop ([#573](https://github.com/mapnik/mapnik/issues/573))

- Fixed raster alpha blending ([#589](https://github.com/mapnik/mapnik/issues/589),[#674](https://github.com/mapnik/mapnik/issues/674))

- Enhanced support for faster reprojection if proj >= 4.8 is used ([#575](https://github.com/mapnik/mapnik/issues/575))

- Allow for late-binding of datasources ([#622](https://github.com/mapnik/mapnik/issues/622))

- Fix to OSM plugin to avoid over-caching of data ([#542](https://github.com/mapnik/mapnik/issues/542))

- Various fixes to sqlite, ogr, and occi driver backported from trunk.

- Ensured that `\n` triggers linebreaks in text rendering ([#584](https://github.com/mapnik/mapnik/issues/584))

- Support for boost filesystem v3

- Fixes to cairo renderer to avoid missing images (r2526)

- Fixed reading of label_position_tolerance on text_symbolizer and height for building_symbolizer


# Mapnik 0.7.1

Released March 23, 2010

(Packaged from [db89f1c](https://github.com/mapnik/mapnik/commit/db89f1ca75) / r1745)

- Rasters: Various fixes and improvements to 8bit png output ([#522](https://github.com/mapnik/mapnik/issues/522),[#475](https://github.com/mapnik/mapnik/issues/475))

- XML: Save map buffer_size when serializing map.

- SCons: Added new build options `PRIORITIZE_LINKING` and `LINK_PRIORITY`. The first is a boolean (default True)
  of whether to use the new sorting implementation that gives explcit preference to custom or local paths
  during compile and linking that will affect builds when duplicate libraries and include directories are on the
  system. LINK_PRIORITY defaults to prioritizing internal sources of the mapnik source folder, then local/user
  installed libraries over system libraries, but the option can be customized. Sorting not only ensures that
  compiling and linking will more likely match the desired libraries but also gives more likelyhood to avoid
  the scenario where libraries are linked that don't match the includes libmapnik compiled against.

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

(Packaged from [a0da946](https://github.com/mapnik/mapnik/commit/a0da946be9) / r1574)

- Core: Fixed linking to external libagg (r1297,r1299)

- Core: Completed full support for PPC (Big endian) architectures (r1352 -> r1357)

- Gdal Plugin: Added support for Gdal overviews, enabling fast loading of > 1GB rasters ([#54](https://github.com/mapnik/mapnik/issues/54))

    * Use the gdaladdo utility to add overviews to existing GDAL datasets

- PostGIS: Added an optional `geometry_table` parameter. The `geometry_table` used by Mapnik to look up
  metadata in the geometry_columns and calculate extents (when the `geometry_field` and `srid` parameters
  are not supplied). If `geometry_table` is not specified Mapnik will attempt to determine the name of the
  table to query based on parsing the `table` parameter, which may fail for complex queries with more than
  one `from` keyword. Using this parameter should allow for existing metadata and table indexes to be used
  while opening the door to much more complicated subqueries being passed to the `table` parameter without
  failing ([#260](https://github.com/mapnik/mapnik/issues/260), [#426](https://github.com/mapnik/mapnik/issues/426)).

- PostGIS Plugin: Added optional `geometry_field` and `srid` parameters. If specified these will allow
  Mapnik to skip several queries to try to determine these values dynamically, and can be helpful to avoid
  possible query failures during metadata lookup with complex subqueries as discussed in [#260](https://github.com/mapnik/mapnik/issues/260) and [#436](https://github.com/mapnik/mapnik/issues/436), but
  also solvable by specifying the `geometry_table` parameter. (r1300,[#376](https://github.com/mapnik/mapnik/issues/376))

- PostGIS: Added an optional `extent_from_subquery` parameter that when true (while the `extent` parameter is
  not provided and `estimate_extent` is false) will direct Mapnik to calculate the extent upon the exact table
  or sql provided in the `table` parameter. If a sub-select is used for the table parameter then this will,
  in cases where the subquery limits results, provide a faster and more accurate layer extent. It will have
  no effect if the `table` parameter is simply an existing table. This parameter is false by default. ([#456](https://github.com/mapnik/mapnik/issues/456))

- PostGIS Plugin: Added `!bbox!` token substitution ability in sql query string. This opens the door for various
  complex queries that may aggregate geometries to be kept fast by allowing proper placement of the bbox
  query to be used by indexes. ([#415](https://github.com/mapnik/mapnik/issues/415))

    * Pass the bbox token inside a subquery like: !bbox!

    * Valid Usages include:
      ```sql
        <Parameter name="table">
          (Select ST_Union(geom) as geom from table where ST_Intersects(geometry,!bbox!)) as map
        </Parameter>

        <Parameter name="table">
          (Select * from table where geom && !bbox!) as map
        </Parameter>
      ```

- PostGIS Plugin: Added `scale_denominator` substitution ability in sql query string ([#415](https://github.com/mapnik/mapnik/issues/415)/[#465](https://github.com/mapnik/mapnik/issues/465))

    * Pass the scale_denominator token inside a subquery like: !scale_denominator!

    * e.g. (Select * from table where field_value > !scale_denominator!) as map

- PostGIS Plugin: Added support for quoted table names (r1454) ([#393](https://github.com/mapnik/mapnik/issues/393))

- PostGIS: Add a `persist_connection` option (default true), that when false will release
  the idle psql connection after datasource goes out of scope (r1337) ([#433](https://github.com/mapnik/mapnik/issues/433),[#434](https://github.com/mapnik/mapnik/issues/434))

- PostGIS: Added support for BigInt (int8) postgres type (384)

- PostGIS Plugin: Throw and report errors if SQL execution fails (r1291) ([#363](https://github.com/mapnik/mapnik/issues/363), [#242](https://github.com/mapnik/mapnik/issues/242))

- PostGIS Plugin: Fixed problem in conversion of long numbers to strings (r1302,1303)

- PostGIS Plugin: Added missing support for BigInt(int8) postgres datatypes (r1250) ([#384](https://github.com/mapnik/mapnik/issues/384))

- OGR Plugin: Added support for reading multipoint features ([#458](https://github.com/mapnik/mapnik/issues/458))

- Shape Plugin: Fixed bug in file extension stripping ([#413](https://github.com/mapnik/mapnik/issues/413))

- Shape Plugin: Fixed missing compiler flags that causes crashing on newer g++ versions ([#436](https://github.com/mapnik/mapnik/issues/436))

- PNG: Fixed problem with garbled/striped png256 output along sharp edges([#416](https://github.com/mapnik/mapnik/issues/416),[#445](https://github.com/mapnik/mapnik/issues/445),[#447](https://github.com/mapnik/mapnik/issues/447),[#202](https://github.com/mapnik/mapnik/issues/202))

- PNG: Added support for semi-transparency in png256 output ([#477](https://github.com/mapnik/mapnik/issues/477),[#202](https://github.com/mapnik/mapnik/issues/202))

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

- XML: Fixed memory leak in libxml2 implementation ([#473](https://github.com/mapnik/mapnik/issues/473))

- XML: Added function to serialize map to string, called `mapnik.save_map_to_string()` ([#396](https://github.com/mapnik/mapnik/issues/396))

- XML: Added parameter to <Map> called `minimum_version` to allow for enforcing the minimum Mapnik version
  needed for XML features used in the mapfiles. Uses Major.Minor.Point syntax, for example
  <Map minimum_version="0.6.1"> would throw an error if the user is running Mapnik less than 0.6.1.

- XML: Added support for relative paths when using entities and `mapnik.load_map_from_string()` ([#440](https://github.com/mapnik/mapnik/issues/440))

- XML: Made width and height optional for symbolizers using images (r1543)

- XML: Ensured that default values for layers are not serialized in save_map() (r1366)

- XML: Added missing serialization of PointSymbolizer `opacity` and `allow_overlap` attributes (r1358)

- XML: Default text vertical_alignment now dependent on dy ([#485](https://github.com/mapnik/mapnik/issues/485), r1527)

- Python: Exposed ability to write to Cairo formats using `mapnik.render_to_file()` and without pycairo ([#381](https://github.com/mapnik/mapnik/issues/381))

- Python: Fixed potential crash if pycairo support is enabled but python-cairo module is missing ([#392](https://github.com/mapnik/mapnik/issues/392))

- Python: Added `mapnik.has_pycairo()` function to test for pycairo support (r1278) ([#284](https://github.com/mapnik/mapnik/issues/284))

- Python: Added `mapnik.register_plugins()` and `mapnik.register_fonts()` functions (r1256)

- Python: Pickling support for point_symbolizer (r1295) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Ensured mapnik::config_errors now throw RuntimeError exception instead of UserWarning exception ([#442](https://github.com/mapnik/mapnik/issues/442))

- Filters: Added support for `!=` as an alias to `<>` for not-equals filters (avoids &lt;&gt;) (r1326) ([#427](https://github.com/mapnik/mapnik/issues/427))

- SCons: Improved boost auto-detection (r1255,r1279)

- SCons: Fixed support for JOBS=N and FAST=True to enable faster compiling (r1440)

- SCons: Ensured that -h or --help will properly print help on custom Mapnik options before a user
  has been able to properly run `configure`. (r1514)

- SCons: Added ability to link to custom icu library name using ICU_LIB_NAME (r1414)

- SCons: Improved reliability of python linking on OSX ([#380](https://github.com/mapnik/mapnik/issues/380))

- Fonts: Added unifont to auto-installed fonts, which is used by the OSM styles as a fallback font (r1328)


# Mapnik 0.6.1

Released July 14, 2009

(Packaged from [353ff57](https://github.com/mapnik/mapnik/commit/353ff576c7) / r1247)

- Plugins: expose list of registered plugins as a `plugin_names()` method of DatasourceCache (r1180)

- XML: Fixed serialization and parsing bugs related to handling of integers and Enums ([#328](https://github.com/mapnik/mapnik/issues/328),[#353](https://github.com/mapnik/mapnik/issues/353))

- SCons: Added the ability to set the PKG_CONFIG_PATH env setting ([#217](https://github.com/mapnik/mapnik/issues/217))

- SCons: Improved linking to only required libraries for libmapnik ([#371](https://github.com/mapnik/mapnik/issues/371))

- Shape Plugin: Added compile time flag to allow disabling the use of memory mapped files (r1213) ([#342](https://github.com/mapnik/mapnik/issues/342))

- Core: Improved support for PPC (Big endian) architectures (r1198 -> r1213)

- Scons: Improved auto-detection of boost libs/headers (r1200) ([#297](https://github.com/mapnik/mapnik/issues/297))

- Plugins: Exposed list of available/registered plugins (r1180) ([#246](https://github.com/mapnik/mapnik/issues/246))

- SCons: Improve build support for SunCC (patches from River Tarnell) (r1168, r1169)

- Python: Pickling support for text_symbolizer (r1164) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for proj_transform and view/coord_transform (r1163) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for parameters (r1162) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for stroke objects (r1161) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for line_symbolizer (r1160) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for projection objects (r1159) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for shield_symbolizer (r1158) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for polygon_symbolizer (r1157) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for query objects (r1156) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for pattern symbolizers (r1155) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Pickling support for raster_symbolizer (r1154) ([#345](https://github.com/mapnik/mapnik/issues/345))

- Python: Added `mapnik.has_cairo()` function to test for cairo support (r1152) ([#284](https://github.com/mapnik/mapnik/issues/284))

- Python: Exposed dash_array get method (r1151) ([#317](https://github.com/mapnik/mapnik/issues/317))

- Python: Pickling support for Coord objects ([#345](https://github.com/mapnik/mapnik/issues/345))

- GDAL Plugin: Added an experimental option to open files in `shared mode` (r1143)

- Python: Exposed RasterSymbolizer options in Python (r1139)

- Plugins: Fixed support for non-file based sources in GDAL and OGR plugins ([#336](https://github.com/mapnik/mapnik/issues/336),[#337](https://github.com/mapnik/mapnik/issues/337))

- Plugins: Formal inclusion of new plugin for Kismet server (r1127) ([#293](https://github.com/mapnik/mapnik/issues/293))

- Python: Made access to features and featuresets more Pythonic (r1121) ([#171](https://github.com/mapnik/mapnik/issues/171),[#280](https://github.com/mapnik/mapnik/issues/280),[#283](https://github.com/mapnik/mapnik/issues/283))

- XML: Ensured relative paths in XML are interpreted relative to XML file location (r1124) ([#326](https://github.com/mapnik/mapnik/issues/326))

- XML: Added ability to serialize all default symbolizer values by passing third argument to save_map(m,`file.xml`,True)(r1117) ([#327](https://github.com/mapnik/mapnik/issues/327))

- Core: Added support for alpha transparency when writing to png256 (patch from Marcin Rudowski) ([#202](https://github.com/mapnik/mapnik/issues/202))

- SCons: Ensured ABI compatibility information is embedded in libmapnik.dylib on Mac OS X ([#322](https://github.com/mapnik/mapnik/issues/322))

- SCons: Ensured that the full `install_name` path would be added to libmapnik.dylib on Mac OS X ([#374](https://github.com/mapnik/mapnik/issues/374))

- Tests: Added testing framework in Python using nose (r1101-r1105)

- Raster Plugin: Added a tile/bbox-based read policy for large (rasters width * height > 1024 * 1024 will be loaded in chunks) (r1089)

- OGCServer: Made lxml dependency optional (r1085) ([#303](https://github.com/mapnik/mapnik/issues/303))

- Rasters: Handle rounding to allow better alignment of raster layers (r1079) ([#295](https://github.com/mapnik/mapnik/issues/295))

- AGG Renderer: Added option to control output JPEG quality (r1078) ([#198](https://github.com/mapnik/mapnik/issues/198))

- Plugins: Fixed segfault in OGR Plugin with empty geometries (r1074) ([#292](https://github.com/mapnik/mapnik/issues/292))


# Mapnik 0.6.0

Released April 1, 2009

(Packaged from [c88e034](https://github.com/mapnik/mapnik/commit/c88e03436f) / r1066)

- Python: Added support for aspect_fix_mode (r1013)

- OGCServer Fixed axis-ordering for WMS 1.3.0 request (r1051) ([#241](https://github.com/mapnik/mapnik/issues/241))

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

- Plugins: Added OGR driver for reading all OGR supported formats (kunitoki) (r836) ([#170](https://github.com/mapnik/mapnik/issues/170))

- XML: Added serialization of Fontsets (r807)

- XML: Added support for reading xml from a string (r806)

- C++: renamed mapnik::Color to mapnik::color (r796)

- Python: Made available the scale_denominator property from the map in c++ and python (r794)

- Python: Added ability to resize map and clear all layers and styles from python (r793)

- Python: Exposed Proj to/from transformation for projected coordinate systems (r792,r822) ([#117](https://github.com/mapnik/mapnik/issues/117))

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

- Core: Use streams to write images (i/o re-factor) (r628) ([#15](https://github.com/mapnik/mapnik/issues/15))

# Mapnik 0.5.1

Released April 15, 2008

(Packaged from [c29cb73](https://github.com/mapnik/mapnik/commit/c29cb7386d))

# Mapnik 0.5.0

Released April 15, 2008

(Packaged from [0464a35](https://github.com/mapnik/mapnik/commit/0464a3563c))

# Mapnik 0.4.0

Released February 26, 2007

(Packaged from [8d73e3a](https://github.com/mapnik/mapnik/commit/8d73e3a8dc))

# Mapnik 0.3.0

Released May 22, 2006

(Packaged from [3ae046e](https://github.com/mapnik/mapnik/commit/3ae046ebe2))
