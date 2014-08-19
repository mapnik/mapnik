import mapnik
from nose.tools import *
from utilities import execution_path, run_all

def test_grayscale_conversion():
    im = mapnik.Image(2,2)
    im.background = mapnik.Color('white')
    im.set_grayscale_to_alpha()
    pixel = im.get_pixel(0,0)
    eq_((pixel >> 24) & 0xff,255);

if __name__ == "__main__":
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
