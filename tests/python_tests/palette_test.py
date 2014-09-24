#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nose.tools import *
from utilities import execution_path, run_all
import os, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

expected_64 = '[Palette 64 colors #494746 #c37631 #89827c #d1955c #7397b9 #fc9237 #a09f9c #fbc147 #9bb3ce #b7c9a1 #b5d29c #c4b9aa #cdc4a5 #d5c8a3 #c1d7aa #ccc4b6 #dbd19c #b2c4d5 #eae487 #c9c8c6 #e4db99 #c9dcb5 #dfd3ac #cbd2c2 #d6cdbc #dbd2b6 #c0ceda #ece597 #f7ef86 #d7d3c3 #dfcbc3 #d1d0cd #d1e2bf #d3dec1 #dbd3c4 #e6d8b6 #f4ef91 #d3d3cf #cad5de #ded7c9 #dfdbce #fcf993 #ffff8a #dbd9d7 #dbe7cd #d4dce2 #e4ded3 #ebe3c9 #e0e2e2 #f4edc3 #fdfcae #e9e5dc #f4edda #eeebe4 #fefdc5 #e7edf2 #edf4e5 #f2efe9 #f6ede7 #fefedd #f6f4f0 #f1f5f8 #fbfaf8 #ffffff]'

expected_256 = '[Palette 256 colors #272727 #3c3c3c #484847 #564b41 #605243 #6a523e #555555 #785941 #5d5d5d #746856 #676767 #956740 #ba712e #787777 #cb752a #c27c3d #b68049 #dc8030 #df9e10 #878685 #e1a214 #928b82 #a88a70 #ea8834 #e7a81d #cb8d55 #909090 #94938c #e18f48 #f68d36 #6f94b7 #e1ab2e #8e959b #c79666 #999897 #ff9238 #ef9447 #a99a88 #f1b32c #919ca6 #a1a09f #f0b04b #8aa4bf #f8bc39 #b3ac8f #d1a67a #e3b857 #a8a8a7 #ffc345 #a2adb9 #afaeab #f9ab69 #afbba4 #c4c48a #b4b2af #dec177 #9ab2cf #a3bebb #d7b491 #b6cd9e #b5d29c #b9c8a2 #f1c969 #c5c79e #bbbab9 #cabdaa #a6bcd1 #cec4a7 #e7cc89 #dad98a #d5c9a3 #fabd8a #c1d7aa #cec5b4 #d1d1a5 #d9cf9f #c5c4c3 #d3c7b5 #ddd59d #b4c6d6 #d1cbb4 #d1c7ba #d7d1aa #e1c6ab #cbc7c2 #dbd0a9 #e8e58a #fee178 #d3cbba #dfd7a3 #d2cfb9 #c9ddb5 #d2cbbe #c3cbce #d7cbba #dcceb2 #dfd3aa #e5dd9a #dbd3b1 #ceccc6 #d7cbbe #d7cfba #dfc3be #dfd3ae #cbcbcb #cbd3c3 #d3cfc0 #e0d8aa #d7cfbe #dbd3b8 #ebe596 #dfd8b0 #c0ceda #f1ee89 #decfbc #d7cfc4 #d7d3c3 #d1d0cd #d2dfc0 #dbd3c3 #e7c7c3 #e7d7b3 #f2ed92 #d1e2bf #dad7c3 #fef383 #d3d3cf #dbd3c7 #e0d3c2 #dfd7c0 #ebe4a8 #dbd7c7 #dfd3c7 #f7f38f #c9d4de #dcdcc5 #dfd7c7 #e7d5c2 #d6d5d4 #faf78e #d7dfca #fbfb8a #fffb86 #dfd7cb #e5ddc0 #dad7d2 #ecd6c1 #cfd7de #e8d0cc #fbfb8e #fffb8a #eae3b8 #e3d7cd #dfdbce #fffb8e #ffff8a #f5efa6 #dae6cc #e3dbcf #edddc3 #dddbd6 #d5dbdf #ffff91 #e3dbd3 #fefc99 #e7dbd2 #eaddcd #e3dfd3 #ebd7d3 #dddddd #d4dee6 #e2dfd7 #fcdcc0 #e7dbd7 #e7dfd3 #ebe4cb #f4eeb8 #e3dfdb #e7dfd7 #ebded5 #e7e3d7 #fefea6 #e1ecd6 #ece5d3 #e7e3db #dee3e5 #ebe3db #efdfdb #efe3d8 #f4efc9 #e6ecdb #ebe3df #ebe7db #f0ecd3 #e5e6e5 #efe7da #ebe7df #efe3df #fefeb8 #dfe7ef #ebe7e3 #edebde #efe7e0 #e8efe0 #e7f3df #ebebe3 #e7ebe8 #f5edd9 #efebe3 #e3ebf1 #e9efe7 #ebebea #efebe7 #f0efe2 #ecf3e5 #fefdc9 #efefe7 #f3efe7 #f5f3e1 #f2efe9 #e9eef4 #ffeddf #efefef #f3efeb #f3f3eb #f0f7eb #fbf7e1 #fefed8 #f3f3ef #f7f3eb #eef3f7 #f7f7ea #f3f3f3 #f3f7ef #f7f3ef #f3f3f7 #f7f3f3 #f7f7ef #fffee3 #f3f7f7 #f7f7f3 #fcf7ee #f7f7f7 #f7fbf4 #f5f7fb #fbf7f6 #fffeef #f7fbfb #fbfbf7 #fbfbfb #fbfbff #fbfffb #fffbfb #fbffff #fffffb #ffffff]'

expected_rgb = '[Palette 2 colors #ff00ff #ffffff]'

def test_reading_palettes():
    act = open('../data/palettes/palette64.act','rb')
    palette = mapnik.Palette(act.read(),'act')
    eq_(palette.to_string(),expected_64);
    act = open('../data/palettes/palette256.act','rb')
    palette = mapnik.Palette(act.read(),'act')
    eq_(palette.to_string(),expected_256);
    palette = mapnik.Palette('\xff\x00\xff\xff\xff\xff', 'rgb')
    eq_(palette.to_string(),expected_rgb);

if 'shape' in mapnik.DatasourceCache.plugin_names():

    def test_render_with_palette():
        m = mapnik.Map(600,400)
        mapnik.load_map(m,'../data/good_maps/agg_poly_gamma_map.xml')
        m.zoom_all()
        im = mapnik.Image(m.width,m.height)
        mapnik.render(m,im)
        act = open('../data/palettes/palette256.act','rb')
        palette = mapnik.Palette(act.read(),'act')
        # test saving directly to filesystem
        im.save('/tmp/mapnik-palette-test.png','png',palette)
        # test saving to a string
        open('/tmp/mapnik-palette-test2.png','wb').write(im.tostring('png',palette));
        # compare the two methods
        eq_(mapnik.Image.open('/tmp/mapnik-palette-test.png').tostring(),mapnik.Image.open('/tmp/mapnik-palette-test2.png').tostring(),'%s not eq to %s' % ('/tmp/mapnik-palette-test.png','/tmp/mapnik-palette-test2.png'))
        # compare to expected
        eq_(mapnik.Image.open('/tmp/mapnik-palette-test.png').tostring(),mapnik.Image.open('./images/support/mapnik-palette-test.png').tostring(),'%s not eq to %s' % ('/tmp/mapnik-palette-test.png','./images/support/mapnik-palette-test.png'))

if __name__ == "__main__":
    setup()
    exit(run_all(eval(x) for x in dir() if x.startswith("test_")))
