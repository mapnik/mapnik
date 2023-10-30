# Design philosophy

Above all Mapnik is about making beautiful maps. This aim drives us to be constantly forward looking. Progress in technology and design are reshaping the art of the possible for maps. Mapnik joins the best ideas in high quality graphics with robust patterns and algorithms for spatial data access. The goal should be no less than enabling a new generation of map makers and gorgeous maps.

Mapnik is a library. It is not a server - rather it's for writing servers. Or for writing desktop graphics engines to display maps. Or for visualizing new galaxies - its up to you. If you have an inclination for scripting and something to render in real world or celestial coordinates, Mapnik is for you. Mapnik is not a full solution, or even half a solution. At its best, Mapnik is a drawing api that provides the right tools for the developer to make sense of, and art from, geodata. 

But Mapnik is not just about drawing on a canvas. Beautiful maps can also be interactive maps and Mapnik aims to provide very flexible, custom access to geo features both in its C++ api and in binding languages. MetaWriters and the Grid renderer are two recent advances that enable highly interactive feature display in mapping applications using JSON serialized features, but more will come.

Mapnik core is a simple set of objects providing structures for features and the ability to query, filter, cache, and render them. This design can be lightweight, flexible, and high performance because the core library does not have to worry about where data comes from or where it goes after rendering. Datasources are separate plugin libraries loaded at runtime that, when called, provide feature arrays. The core classes make no assumptions about map size, format, or projection. The core is designed to make writing a high performance tile server, that renders on-demand and uses multi-threaded or multi-process concurrency, as easy as possible.

Chasing beauty and speed, as technology advances, requires experimentation and a 
frequent stream of new ideas. To write fast and creative code we strive to avoid hard dependencies on older libraries or libraries that attempt to do too much at the cost of performance. At the same time, to be able to maintain ambitious goals we cannot write boilerplate code or attempt to write robust code for domains outside of mapping and graphics. As such we rely as much as possible on the Boost libraries.

Mapnik is cross platform because the C++ language and Boost makes this feasible
enough it would be a shame not to be. But our development target is the latest
release of Linux, particularly server based distributions, and many Mapnik developers
use OSX as a development environment.

If you are having fun using Mapnik, stick with it. But, Mapnik is not always the right tool, of course. If you have small sets of data (MB's), try just rendering it in a web browser client using a javascript mapping api. Or if you need to set up an array of OGC Web services, use GeoServer. Lastly, if you are looking for a full solution for web cartography, use TileMill.

