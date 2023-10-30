#!/usr/bin/env bash
export LD_LIBRARY_PATH=/opt/mapnik/lib:/usr/local/lib:/usr/local/lib64:/usr/local/boost-1.73/lib:/usr/lib/aarch64-linux-gnu:/usr/local/freetype/lib:$LD_LIBRARY_PATH
sh /opt/mapnik/bin/mapnik-viewer
