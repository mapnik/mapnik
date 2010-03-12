
import mapnik2
from nose.tools import *

class test_raster_colorizer():
    colorizer = mapnik2.RasterColorizer()
    # Setup the color bands. band[N].color will apply to all
    # values 'v' if band[N].value <= v < band[N+1].color
    # If no color is found then "transparent" will be assigned
    bands = [(value, mapnik2.Color(color)) for value, color in [
        (  0, "#0044cc"),
        ( 10, "#00cc00"),
        ( 20, "#ffff00"),
        ( 30, "#ff7f00"),
        ( 40, "#ff0000"),
        ( 50, "#ff007f"),
        ( 60, "#ff00ff"),
        ( 70, "#cc00cc"),
        ( 80, "#990099"),
        ( 90, "#660066"),
        ( 200, "#00000"), # last band denotes upper limit, values above it will
                          # not return the color
        ]]
    for value, color in bands:
        colorizer.append_band(value, color)

    eq_(colorizer.get_color(-1), mapnik2.Color("transparent"))
    eq_(colorizer.get_color(0), bands[0][1])
    eq_(colorizer.get_color(5), bands[0][1])
    eq_(colorizer.get_color(10), bands[1][1])
    eq_(colorizer.get_color(200), mapnik2.Color("transparent"))
    eq_(colorizer.get_color(201), mapnik2.Color("transparent"))
