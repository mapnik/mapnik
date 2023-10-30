Epydocs
=======

September 2009 - Dane Springmeyer

This is a folder containing scripts and a CSS file to regenerate the python api documentation for Mapnik.


Requires
--------

 * The Python Epydoc module. Do `sudo easy_install epydoc` 
 * Mapnik Python bindings installed on your PYTHONPATH
 * Being run from the docs/epydoc_config folder inside a svn checkout of the Mapnik source code


Generating
----------

If you are editing or creating docstrings for the boost::python module
you will want to frequently view the output in a variety of documentation
viewers like `pydoc` and epydoc (eventually we will also likely use sphinx)

To test generation of new epidocs do:

$ cd docs/epydoc_config
$ ./test_build_epydoc.sh

Then open test_api/index.html in a browser.

If you have commit, then generate and add to svn updated docs by doing:

$ cd docs/epydoc_config
$ ./build_epydoc.sh


Errors
------

When re-generating the docs you will likely see an error like:
'Error: ImportError: No module named _apache (line 25)'. This comes
from the mod_python wrapper of the OGCServer and can be ignored.

Also you may see a warning about markup errors, which may or may not be harmless.
