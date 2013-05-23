## rundemo.cpp

This directory contains a simple c++ program demonstrating the Mapnik C++ API. It mimics the python 'rundemo.py' example with a couple exceptions.

If building on unix you can have this program automatically build by configuring Mapnik like:

    ./configure DEMO=True

However, this example code also should be able to be built standalone.

The following notes describe how to do that on various operating systems.

## Depends

 - Mapnik library development headers
 - `mapnik-config` on unix and `mapnik-config.bat` on windows

### Unix

On OS X and Linux you also need `make`.

### Windows

On windows, additional dependencies to build are:

 - MSVS 2010 with C++ compiler
 - Python 2.x
 - gyp: https://code.google.com/p/gyp | https://github.com/springmeyer/hello-gyp

`mapnik-config.bat` should come with your Mapnik installation.

First confirm it is on your path:

    mapnik-config # should give usage

To install gyp, which is pure python do:

    svn checkout http://gyp.googlecode.com/svn/trunk/ gyp
    cd gyp
    sudo python setup.py install


## Building the demo

### Unix

Simply type:

   make

Then to run do:

   ./rundemo `mapnik-config --prefix`

On OS X you can also create an xcode project:

    gyp rundemo.gyp --depth=. -f xcode --generator-output=./build/
    xcodebuild -project ./build/rundemo.xcodeproj
    ./build/out/Release/rundemo `mapnik-config --prefix`


### Windows

First you need to build he visual studio files with gyp:

    gyp rundemo.gyp --depth=. -f msvs -G msvs_version=2010

Then you can compile the demo with `msbuild`:

    msbuild build.sln