# Shared feature cache

Setting `cache-features="true"` on a vector layer also allows its feature query
to be shared with compatible later layers in the same render. Those later layers
need not enable caching themselves. No additional datasource parameter is needed.
Enabling caching on a later layer does not retroactively cache an earlier query.

Sharing requires the same datasource instance, or PostGIS datasources with
identical parameters. Equal parameters alone do not identify the contents of
arbitrary datasources, such as independently populated in-memory sources.
Sharing also requires identical layer SRS, plus matching
effective query bounds, resolution, scale denominator, filter factor and
variables. Requested attributes may differ: Mapnik queries their union once
and gives each consumer an independent reader. Rendering order is unchanged.

Decoded features are retained until the last reader finishes. The cache does
not persist across renders. Memory usage depends on feature size and how far
apart the consumers are; it is not currently bounded by a configurable limit.

Only enable caching when repeated queries may safely use the same result and consumers
do not mutate shared feature data. Do not use this with volatile SQL or with
queries that require fresh results for each layer. A later layer's
`cache-features="false"` does not prohibit consuming an existing shared result.
