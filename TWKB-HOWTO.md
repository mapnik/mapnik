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
*NOTE: `update planet_osm_polygon_twkb set twkb = ST_AsTWKB(way, 2);` should be using `ST_AsTWKB(ST_Simplify(ST_RemoveRepeatedPoints(way, <tolerance>),<tolerance>),2)`
`


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
