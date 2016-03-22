### Direct TWKB
#### Prerequisites

* Standard set of planet OSM tables imported into PostgreSQL with `osm2pgsql` or similar.

#### Creating and populating TWKB table

```sql
create table planet_osm_polygon_twkb as (select * from planet_osm_polygon);
alter table planet_osm_polygon_twkb add column twkb bytea;
update planet_osm_polygon_twkb set twkb = ST_AsTWKB(way, 2);
alter table planet_osm_polygon_twkb drop column way;
```
*NOTE: `update planet_osm_polygon_twkb set twkb = ST_AsTWKB(way, 2);` should be using `ST_AsTWKB(ST_Simplify(ST_RemoveRepeatedPoints(way, <tolerance>),<tolerance>, true),2)`*


#### Spatial index

```sql
create index planet_osm_polygon_twkb_index on planet_osm_polygon_twkb using GIST(ST_GeomFromTWKB(twkb));
```

##### Vacuum to update stats

```sql
VACUUM FULL ANALYZE VERBOSE planet_osm_polygon_twkb ;
\d+
```

##### XML style

Make sure style has following parameters are present on top of usual postgis.input setup.

```xml
<Parameter name="geometry_field">twkb</Parameter>
<Parameter name="twkb_encoding">True</Parameter>
<Parameter name="twkb_direct">True</Parameter>
```


##### Iceland (osm2pgsql)

```sql
....
update planet_osm_line_twkb set twkb = ST_AsTWKB(ST_Simplify(ST_RemoveRepeatedPoints(way,1), 1, true),0);
update planet_osm_polygon_twkb set twkb = ST_AsTWKB(ST_Simplify(ST_RemoveRepeatedPoints(way,1), 1, true),0);

```

```bash
osm=# \d+
							 List of relations
 Schema |          Name           | Type  | Owner |   Size   | Description
--------+-------------------------+-------+-------+----------+-------------
 public | geography_columns       | view  | artem | 0 bytes  |
 public | geometry_columns        | view  | artem | 0 bytes  |
 public | planet_osm_line         | table | artem | 30 MB    |
 public | planet_osm_line_twkb    | table | artem | 10216 kB |
 public | planet_osm_nodes        | table | artem | 93 MB    |
 public | planet_osm_point        | table | artem | 7136 kB  |
 public | planet_osm_polygon      | table | artem | 28 MB    |
 public | planet_osm_polygon_twkb | table | artem | 10 MB    |
 public | planet_osm_rels         | table | artem | 3136 kB  |
 public | planet_osm_roads        | table | artem | 4784 kB  |
 public | planet_osm_ways         | table | artem | 51 MB    |
 public | spatial_ref_sys         | table | artem | 4016 kB  |
(12 rows)

```
