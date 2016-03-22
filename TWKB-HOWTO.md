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
#### Spatial index

```sql
create index planet_osm_polygon_twkb_index on planet_osm_polygon_twkb using GIST(ST_GeomFromTWKB(twkb));
```

##### Vacuum to update stats

```sql
VACUUM FULL ANALYZE VERBOSE planet_osm_polygon_twkb ;
\d+
```

