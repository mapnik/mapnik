# $Id$

Mapnik OGC Server
-----------------


Introduction
------------

Mapnik provides a server package to allow the publishing of maps
through the open and standard WMS interface published by the Open Geospatial
Consortium (OGC).  It is in implemented in Python, around the core Mapnik C++
library.

This is the very first implementation of a WMS for Mapnik.  Although inital
testing seems to suggest it works well, there may be bugs, and it lacks some
useful features.  Comments, contributions, and requests for help should all be
directed to the Mapnik mailing list.


Features
--------

- WMS 1.1.1 and 1.3.0
- CGI/FastCGI, WSGI, mod_python 
- Supports all 3 requests: GetCapabilities, GetMap and GetFeatureInfo
- JPEG/PNG output
- XML/INIMAGE/BLANK error handling
- Multiple named styles support
- Reprojection support
- Supported layer metadata: title, abstract
- Ability to request all layers with LAYERS=__all__


Caveats
----------------
- GetFeatureInfo supports text/plain output only
- PNG256(8-bit PNG not yet supported)
- CGI/FastCGI interface needs to be able to write to tempfile.gettempdir() (most likely "/tmp")
- Need to be further evaluated for thread safety


Dependencies
------------

Please properly install the following before proceeding further:

- Mapnik python bindings (which will also install the `ogcserver` module code)
- lxml (http://codespeak.net/lxml/)
- PIL (http://www.pythonware.com/products/pil)

For the CGI/FastCGI interface also install:

- jonpy (http://jonpy.sourceforge.net/)


Installation
------------

- The OGCServer uses the Mapnik interface to the Proj.4 library for projection support
  and depends on integer EPSG codes. Confirm that you have installed Proj.4 with
  all necessary data files (http://trac.osgeo.org/proj/wiki/FAQ) and have added any custom
  projections you need to the 'epsg' file usually located at '/usr/local/share/proj/epsg'.

- Test that the server code is available and installed properly by importing it within a
  python interpreter::
  
  >>> from mapnik import ogcserver
  >>> # no error means proper installation

- There is a sample python script called "wms.py" in the utils/ogcserver folder of the
  Mapnik source code that will work for both CGI and FastCGI operations. Where to place it
  will depend on your server choice and configuration and is beyond this documentation.
  For information on FastCGI go to http://www.fastcgi.com/.


Configuring the server
----------------------

- You will need to create two simple python scripts:

  1) The web-accessible python script ('wms.py') which will import the 
     ogcserver module code and associate itself with the 'ogcserver.conf'
     configuration file. The code of this script will depend upon whether
     you deploy the server as cgi/fastcgi/wsgi/mod_python. See the Mapnik
     Community Wiki for examples: http://trac.mapnik.org/wiki/OgcServer and
     see the cgi sample in the /utils/ogcserver folder.
     
  2) A 'map_factory' script which loads your layers and styles. Samples of this
     script can be found below.

  
- Next you need to edit the ogcserver.conf file to:
  
  1) Point to the 'map_factory' script by using the "module" parameter

  2) Fill out further settings for the server.
    
  Edit the configuration file to your liking, the comments within the file will
  help you further.  Be sure to at the very minimum edit the "module"
  parameter, the server will not work without you setting it properly first.
  

Defining Layers and Styles
--------------------------

The ogcserver obviously needs layers to publish and styles for how to display those layers.

You create you layers and styles in the 'map_factory' script.

For now this can be done by either loading an XML mapfile inside that script using the 
'loadXML()' function or by writing your layers and styles in python code, or both.

The 'map_factory' module must look, at a bare minimum like if you load layers and styles
using an existing XML mapfile::

  from mapnik.ogcserver.WMS import BaseWMSFactory
  
  class WMSFactory(BaseWMSFactory):
    def __init__(self):
      BaseWMSFactory.__init__(self)
      self.loadXMl('/full/path/to/mapfile.xml')
      self.finalize()

Or if you want to define your layers and styles in pure python dynamically you might
have a 'map_factory' more like::

  from mapnik.ogcserver.WMS import BaseWMSFactory
  from mapnik import *

  SHAPEFILE = '/path/to/world_borders.shp'
  PROJ4_STRING = '+init=epsg:4326'  
  
  class WMSFactory(BaseWMSFactory):
    def __init__(self):
      BaseWMSFactory.__init__(self)
      sty,rl = Style(),Rule()
      poly = PolygonSymbolizer(Color('#f2eff9'))
      line = LineSymbolizer(Color('steelblue'),.1)
      rl.symbols.extend([poly,line])
      sty.rules.append(rl)
      self.register_style('world_style',sty)
      lyr = Layer('world',PROJ4_STRING)
      lyr.datasource = Shapefile(file=SHAPEFILE)
      lyr.title = 'World Borders'
      lyr.abstract = 'Country Borders of the World'
      self.register_layer(lyr,'world_style',('world_style',))
      self.finalize()
    
The rules for writing this class are:

- It MUST be called 'WMSFactory'.
- It MUST sub-class mapnik.ogcserver.WMS.BaseWMSFactory.
- The __init__ MUST call the base class.
- Layers MUST be named with the first parameter to the constructor.
- Layers MUST define an EPSG projection in the second parameter of the
  constructor.  This implies that the underlying data must be in an EPSG
  projection already.
- style and layer names are meant for machine readability, not human.  Keep
  them short and simple, without spaces or special characters.
- For human readable info, set the title and abstract properties on the layer
  object.
- DO NOT register styles using layer.styles.append(), instead, provide style
  information to the register_layer() call::
  
    register_layer(layerobject, defaultstylename, (tuple of alternative style names,))

- No Map() object is used or needed here.
- Be sure to call self.finalize() once you have registered everything! This will
  validate everything and let you know if there are any problems.
- For a layer to be queryable via GetFeatureInfo, simply set the 'queryable'
  property to True::
  
    lyr.queryable = True


To Do
-----

- Investigate moving to cElementTree from lxml.
- Add some internal "caching" for performance improvements.
- Switch to using C/C++ libs for image generation, instead of PIL (also
  requires core changes). PIL requirement will remain for INIMAGE/BLANK
  error handling.