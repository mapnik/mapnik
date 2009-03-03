This directory contains a simple c++ program demonstrating the Mapnik C++ API. It mimics the python 'rundemo.py' example with a couple exceptions.

To build it re-configure SCons with DEMO=True then rebuild::

    $ python scons/scons.py configure DEMO=True
    $ python scons/scons.py


The sample program will be compiled (but not installed).


To run::

    $ cd demo/c++
    $ ./rundemo /usr/local/lib/mapnik

For more detailed comments have a look in demo/python/rundemo.py

Have fun!
Artem.
