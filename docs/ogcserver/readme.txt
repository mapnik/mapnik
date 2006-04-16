# $Id$

Mapnik OGC Server
-----------------


Introduction
------------

	Mapnik provides a server package to allow the publishing of maps
through the open and standard WMS interface published by the Open Geospatial
Consortium (OGC).  It is in implemented in Python, around the core C++
library.


Features/Caveats
----------------

- WMS 1.1.1 and 1.3.0
- CGI/FastCGI
- GetCapabilities and GetMap support only (NO GetFeatureInfo)
- GIF/JPEG/PNG output
- XML/INIMAGE/BLANK error handling
- No real layer metadata support yet
- No re-projection support


Dependencies
------------

Please properly install the following before proceeding further:

- jonpy (http://jonpy.sourceforge.net/)
- lxml (http://codespeak.net/lxml/)
- PIL (http://www.pythonware.com/products/pil)
- PROJ.4 (http://proj.maptools.org/)


Installation
------------

- Make sure you compile or re-compile Mapnik after installing PROJ.4, if this
  was not done already.  Mapnik includes a Python API to the PROJ.4 library
  used by the ogcserver.  The PROJ_INCLUDES and PROJ_LIBS parameters can
  be passed to mapnik's scons build script to help with this.
  
  You can test the availability of the PROJ.4 bindings by trying:
  
  $ python
  Python 2.3.4 (#1, Feb 22 2005, 04:09:37)
  [GCC 3.4.3 20041212 (Red Hat 3.4.3-9.EL4)] on linux2
  Type "help", "copyright", "credits" or "license" for more information.
  >>> from mapnik import Projection
  registered datasource : raster
  registered datasource : shape
  registered datasource : postgis
  >>>

- The executable "ogcserver" in utils/ogcserver will work for both CGI and
  FastCGI operations.  Where to place it will depend on your server's
  configuration and is beyond this documentation.  For information on FastCGI
  go to http://www.fastcgi.com/.


Configuring the server
----------------------

- You will need to edit the ogcserver executable for now.  It is a simple
  Python text script.
  
  1) Edit the path to the interpreter in the first line.
  2) Edit the path to the config file if you don't like the default.
  
- Copy the sample configuration "ogcserver.conf" file in utils/ogcserver to
  the location you specified in the previous step.
  
- Edit the configuration file to your liking, the comments within the file will
  help you further.  Be sure to at the very minimum edit the "module"
  parameter, the server will not work without you setting it properly first.
  

Defining layers and styles for use by the ogcserver
---------------------------------------------------

	The ogcserver obviously needs layers to publish.  For now, with Mapnik, this
can only be done by writing code. In this case, a Python script will need to be
written to describe the layers and respective styles.  For information on the Python
API, look in demo/python, or in docs/epydocs.

The server needs a python module, with code that looks like this:

from mapnik.ogcserver.WMS import BaseWMSFactory

class WMSFactory(BaseWMSFactory):

	def __init(self):
		BaseWMSFactory.__init__(self)
		sty = Style()
		...
		self.register_style('stylename', sty)
		
		lyr = Layer(name='layername')
		...
		self.register_layer(lyr)
		
The rules for writing this class are:

- It MUST be called 'WMSFactory'.
- It MUST sub-class mapnik.ogcserver.WMS.BaseWMSFactory.
- The __init__ MUST call the base class'.
- Layers MUST be named with the 'name' parameter to the constructor.
- style and layer names are meant for machine readability, not human.  Keep
  them short and simple, without spaces or special characters.
- The layers must have at least one style associated with them (a default).
- No Map() object is used or needed here.