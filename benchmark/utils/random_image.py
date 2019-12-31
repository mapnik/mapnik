import mapnik
import random

im = mapnik.Image(256,256)

for x in range(im.width()):
    for y in range(im.height()):
        r = int(random.random() * 255)
        g = random.random() * 255
        b = random.random() * 255
        a = random.random()
        color = mapnik.Color('rgba(%i,%i,%i,%f)' % (r,g,b,a))
        im.set_pixel(x,y,color)

im.save('./benchmark/data/multicolor.png')