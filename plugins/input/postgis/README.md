# PostGIS Driver Parameters

* `table`: Either the name of a table to map from or a SQL query to attempt to map (required)
* `geometry_table`: In the case of a SQL query in the `table` parameter, the table in the query that is the source of the geometry
* `geometry_field`: The name of the column in the table or SQL query that will return the geometry
* `key_field`: The name of the column in the table or SQL query that contains a unique key
* `autodetect_key_field`: Try and figure out the keyfield from metadata tables. (default: false)
* `cursor_size`: If non-zero, the maximum number of rows to fetch while processing a query. For memory constrained situations, this limits the amount of data held in memory during query processing. (default: 0)
* `row_limit`: If non-zero, apply a this `LIMIT` to the query, effectively placing a maximum on the number of features to be mapped. (default: 0)
* `srid`: If non-zero, the EPSG SRID to use for the geometries coming from this query, notwithstanding the SRID that is actually on them. (default: 0)
* `host`, `port`, `dbname`, `user`, `password`: PostgreSQL database connection parameters (required)
* `connect_timeout`: Database connection timeout. (default: 4)
* `pool_max_size`: How big to make the connection pool. (default: 10)
* `persist_connection`: Keep connections open between requests? (default: true)
* `estimate_extent`: Try and estimate extent of data? (default: false)
* `extent_from_subquery`: If trying to estimate extent, and SQL is subquery, estimate on that? (default: false)
* `max_async_connection`: If > 1, turn on the asyncronous connection feature. (default: 1)
* `simplify_geometries`: Use PostGIS ST_Simplify call on the geometry to make input data smaller. (default: false)
* `simplify_dp_ratio`: Use a PostGIS simplification factor of this proportion of the ground size of a pixel. For example, if pixels are 20m square for this rendering, and the ratio is set to 0.05, then `ST_Simplify(geom, 1.0)` would be used. (default: 0.05)

## 2.3.x.cartodb

These features all require PostGIS 2.2+ (or a backport), as they use features that do not appear in earlier PostGIS versions. When `twkb_encoding` is true, only `twkb_rounding_adjustment` has effect, other parameters are ignored as filtering and simplification are slaved to the TWKB rounding level.

* `twkb_encoding`: Use TWKB to encode geometries for transport from the database to the renderer instead of standard WKB? (default: false)
* `twkb_rounding_adjustment`: Adjust the resolution the TWKB aims for. At 0.5, the rounding aims for a maximum coarseness of one pixel. At 0.0 rounding rarely exceeds about 0.2 pixels and gets as small as 0.05 pixels at times. Recommended range, -0.5 to 0.5. (Default: 0.0).
* `simplify_dp_preserve`: Set the `preserve` option in `ST_Simplify` when in `simplify_geometries` mode? This will ensure that features that get simplified down to nothing aren't dropped but are retained in point-like form. Useful for rendering to avoid gaps where small feature "disappear". (default: false)
* `simplify_clip_resolution`: If non-zero, sets the map scale at which geometries start getting clipped to the rendering window. (default: 0.0)
* `simplify_prefilter`: If non-zero, runs a distance filter on geometries to ensure all points are at least a certain distance apart. Expressed as a proportion of the current pixel size. (default: 0.0, feature is OFF) (recommended: 0.5)
* `simplify_snap_ratio`: Use PostGIS `ST_SnapToGrid` call to make the input data smaller. This is applied to geometry **before** any `ST_Simplify` is called, so should use a tolerance smaller than the simplify tolerance. Tolerance is expressed as a proportion of a pixel, as with `simplify_dp_ratio`. (default: 1/40)

## SQL Tokens

SQL tokens are strings that are replaced in the input SQL before the SQL is sent to the database for execution. This is information that Mapnik knows as the result of the request, and can be useful in forming complex SQL queries.

* `!bbox!`
* `!scale_denominator!`
* `!pixel_width!`
* `!pixel_height!`

