This directory contains a simple c++ program demonstrating Mapnik API. It mimics python example with couple exceptions.

To build (using GCC/G++ toolkit):

g++ -O3 -I/usr/local/include/mapnik -I/opt/boost/include/boost-1_33_1 -I/usr/include/freetype2 -I../../agg/include -L/usr/local/lib -lmapnik rundemo.cpp -o rundemo


To run:

./rundemo

For more detailed comments have a look in demo/python/rundemo.py

Have fun!
Artem.
