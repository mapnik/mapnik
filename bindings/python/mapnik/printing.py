# -*- coding: utf-8 -*-

"""Mapnik classes to assist in creating printable maps

basic usage is along the lines of

import mapnik

page = mapnik.printing.PDFPrinter()
m = mapnik.Map(100,100)
mapnik.load_map(m, "my_xml_map_description", True)
m.zoom_all()
page.render_map(m,"my_output_file.pdf")

see the documentation of mapnik.printing.PDFPrinter() for options

"""
from __future__ import absolute_import

from . import render, Map, Box2d, MemoryDatasource, Layer, Feature, Projection, ProjTransform, Coord, Style, Rule, Geometry2d
import math
import os
import tempfile

try:
    import cairo
    HAS_PYCAIRO_MODULE = True
except ImportError:
    HAS_PYCAIRO_MODULE = False

try:
    import pangocairo
    import pango
    HAS_PANGOCAIRO_MODULE = True
except ImportError:
    HAS_PANGOCAIRO_MODULE = False

try:
    import pyPdf
    HAS_PYPDF = True
except ImportError:
    HAS_PYPDF = False

class centering:
    """Style of centering to use with the map, the default is constrained

    none: map will be placed flush with the margin/box in the top left corner
    constrained: map will be centered on the most constrained axis (for a portrait page
                 and a square map this will be horizontally)
    unconstrained: map will be centered on the unconstrained axis
    vertical:
    horizontal:
    both:
    """
    none=0
    constrained=1
    unconstrained=2
    vertical=3
    horizontal=4
    both=5

"""Some predefined page sizes custom sizes can also be passed
a tuple of the page width and height in meters"""
pagesizes = {
    "a0": (0.841000,1.189000),
    "a0l": (1.189000,0.841000),
    "b0": (1.000000,1.414000),
    "b0l": (1.414000,1.000000),
    "c0": (0.917000,1.297000),
    "c0l": (1.297000,0.917000),
    "a1": (0.594000,0.841000),
    "a1l": (0.841000,0.594000),
    "b1": (0.707000,1.000000),
    "b1l": (1.000000,0.707000),
    "c1": (0.648000,0.917000),
    "c1l": (0.917000,0.648000),
    "a2": (0.420000,0.594000),
    "a2l": (0.594000,0.420000),
    "b2": (0.500000,0.707000),
    "b2l": (0.707000,0.500000),
    "c2": (0.458000,0.648000),
    "c2l": (0.648000,0.458000),
    "a3": (0.297000,0.420000),
    "a3l": (0.420000,0.297000),
    "b3": (0.353000,0.500000),
    "b3l": (0.500000,0.353000),
    "c3": (0.324000,0.458000),
    "c3l": (0.458000,0.324000),
    "a4": (0.210000,0.297000),
    "a4l": (0.297000,0.210000),
    "b4": (0.250000,0.353000),
    "b4l": (0.353000,0.250000),
    "c4": (0.229000,0.324000),
    "c4l": (0.324000,0.229000),
    "a5": (0.148000,0.210000),
    "a5l": (0.210000,0.148000),
    "b5": (0.176000,0.250000),
    "b5l": (0.250000,0.176000),
    "c5": (0.162000,0.229000),
    "c5l": (0.229000,0.162000),
    "a6": (0.105000,0.148000),
    "a6l": (0.148000,0.105000),
    "b6": (0.125000,0.176000),
    "b6l": (0.176000,0.125000),
    "c6": (0.114000,0.162000),
    "c6l": (0.162000,0.114000),
    "a7": (0.074000,0.105000),
    "a7l": (0.105000,0.074000),
    "b7": (0.088000,0.125000),
    "b7l": (0.125000,0.088000),
    "c7": (0.081000,0.114000),
    "c7l": (0.114000,0.081000),
    "a8": (0.052000,0.074000),
    "a8l": (0.074000,0.052000),
    "b8": (0.062000,0.088000),
    "b8l": (0.088000,0.062000),
    "c8": (0.057000,0.081000),
    "c8l": (0.081000,0.057000),
    "a9": (0.037000,0.052000),
    "a9l": (0.052000,0.037000),
    "b9": (0.044000,0.062000),
    "b9l": (0.062000,0.044000),
    "c9": (0.040000,0.057000),
    "c9l": (0.057000,0.040000),
    "a10": (0.026000,0.037000),
    "a10l": (0.037000,0.026000),
    "b10": (0.031000,0.044000),
    "b10l": (0.044000,0.031000),
    "c10": (0.028000,0.040000),
    "c10l": (0.040000,0.028000),
    "letter": (0.216,0.279),
    "letterl": (0.279,0.216),
    "legal": (0.216,0.356),
    "legall": (0.356,0.216),
}
"""size of a pt in meters"""
pt_size=0.0254/72.0

def m2pt(x):
    """convert distance from meters to points"""
    return x/pt_size

def pt2m(x):
    """convert distance from points to meters"""
    return x*pt_size

def m2in(x):
    """convert distance from meters to inches"""
    return x/0.0254

def m2px(x,resolution):
    """convert distance from meters to pixels at the given resolution in DPI/PPI"""
    return m2in(x)*resolution

class resolutions:
    """some predefined resolutions in DPI"""
    dpi72=72
    dpi150=150
    dpi300=300
    dpi600=600

def any_scale(scale):
    """Scale helper function that allows any scale"""
    return scale

def sequence_scale(scale,scale_sequence):
    """Default scale helper, this rounds scale to a 'sensible' value"""
    factor = math.floor(math.log10(scale))
    norm = scale/(10**factor)

    for s in scale_sequence:
        if norm <= s:
            return s*10**factor
    return scale_sequence[0]*10**(factor+1)

def default_scale(scale):
    """Default scale helper, this rounds scale to a 'sensible' value"""
    return sequence_scale(scale, (1,1.25,1.5,1.75,2,2.5,3,4,5,6,7.5,8,9,10))

def deg_min_sec_scale(scale):
    for x in (1.0/3600,
              2.0/3600,
              5.0/3600,
              10.0/3600,
              30.0/3600,
              1.0/60,
              2.0/60,
              5.0/60,
              10.0/60,
              30.0/60,
              1,
              2,
              5,
              10,
              30,
              60
              ):
        if scale < x:
            return x
    else:
        return x

def format_deg_min_sec(value):
    deg = math.floor(value)
    min = math.floor((value-deg)/(1.0/60))
    sec = int((value - deg*1.0/60)/1.0/3600)
    return "%d°%d'%d\"" % (deg,min,sec)

def round_grid_generator(first,last,step):
        val = (math.floor(first / step) + 1) * step
        yield val
        while val < last:
            val += step
            yield val


def convert_pdf_pages_to_layers(filename,output_name=None,layer_names=(),reverse_all_but_last=True):
    """
    opens the given multipage PDF and converts each page to be a layer in a single page PDF
    layer_names should be a sequence of the user visible names of the layers, if not given
    or if shorter than num pages generic names will be given to the unnamed layers

    if output_name is not provided a temporary file will be used for the conversion which
    will then be copied back over the source file.

    requires pyPdf >= 1.13 to be available"""


    if not HAS_PYPDF:
        raise Exception("pyPdf Not available")

    infile = file(filename, 'rb')
    if output_name:
        outfile = file(output_name, 'wb')
    else:
        (outfd,outfilename) = tempfile.mkstemp(dir=os.path.dirname(filename))
        outfile = os.fdopen(outfd,'wb')

    i = pyPdf.PdfFileReader(infile)
    o = pyPdf.PdfFileWriter()

    template_page_size = i.pages[0].mediaBox
    op = o.addBlankPage(width=template_page_size.getWidth(),height=template_page_size.getHeight())

    contentkey = pyPdf.generic.NameObject('/Contents')
    resourcekey = pyPdf.generic.NameObject('/Resources')
    propertieskey = pyPdf.generic.NameObject('/Properties')
    op[contentkey] = pyPdf.generic.ArrayObject()
    op[resourcekey] = pyPdf.generic.DictionaryObject()
    properties = pyPdf.generic.DictionaryObject()
    ocgs = pyPdf.generic.ArrayObject()

    for (i, p) in enumerate(i.pages):
        # first start an OCG for the layer
        ocgname = pyPdf.generic.NameObject('/oc%d' % i)
        ocgstart = pyPdf.generic.DecodedStreamObject()
        ocgstart._data = "/OC %s BDC\n" % ocgname
        ocgend = pyPdf.generic.DecodedStreamObject()
        ocgend._data = "EMC\n"
        if isinstance(p['/Contents'],pyPdf.generic.ArrayObject):
            p[pyPdf.generic.NameObject('/Contents')].insert(0,ocgstart)
            p[pyPdf.generic.NameObject('/Contents')].append(ocgend)
        else:
            p[pyPdf.generic.NameObject('/Contents')] = pyPdf.generic.ArrayObject((ocgstart,p['/Contents'],ocgend))

        op.mergePage(p)

        ocg = pyPdf.generic.DictionaryObject()
        ocg[pyPdf.generic.NameObject('/Type')] = pyPdf.generic.NameObject('/OCG')
        if len(layer_names) > i:
            ocg[pyPdf.generic.NameObject('/Name')] = pyPdf.generic.TextStringObject(layer_names[i])
        else:
            ocg[pyPdf.generic.NameObject('/Name')] = pyPdf.generic.TextStringObject('Layer %d' % (i+1))
        indirect_ocg = o._addObject(ocg)
        properties[ocgname] = indirect_ocg
        ocgs.append(indirect_ocg)

    op[resourcekey][propertieskey] = o._addObject(properties)

    ocproperties = pyPdf.generic.DictionaryObject()
    ocproperties[pyPdf.generic.NameObject('/OCGs')] = ocgs
    defaultview = pyPdf.generic.DictionaryObject()
    defaultview[pyPdf.generic.NameObject('/Name')] = pyPdf.generic.TextStringObject('Default')
    defaultview[pyPdf.generic.NameObject('/BaseState ')] = pyPdf.generic.NameObject('/ON ')
    defaultview[pyPdf.generic.NameObject('/ON')] = ocgs
    if reverse_all_but_last:
        defaultview[pyPdf.generic.NameObject('/Order')] = pyPdf.generic.ArrayObject(reversed(ocgs[:-1]))
        defaultview[pyPdf.generic.NameObject('/Order')].append(ocgs[-1])
    else:
        defaultview[pyPdf.generic.NameObject('/Order')] = pyPdf.generic.ArrayObject(reversed(ocgs))
    defaultview[pyPdf.generic.NameObject('/OFF')] = pyPdf.generic.ArrayObject()

    ocproperties[pyPdf.generic.NameObject('/D')] = o._addObject(defaultview)

    o._root.getObject()[pyPdf.generic.NameObject('/OCProperties')] = o._addObject(ocproperties)

    o.write(outfile)

    outfile.close()
    infile.close()

    if not output_name:
        os.rename(outfilename, filename)

class PDFPrinter:
    """Main class for creating PDF print outs, basically contruct an instance
    with appropriate options and then call render_map with your mapnik map
    """
    def __init__(self, 
                 pagesize=pagesizes["a4"], 
                 margin=0.005, 
                 box=None,
                 percent_box=None,
                 scale=default_scale, 
                 resolution=resolutions.dpi72,
                 preserve_aspect=True,
                 centering=centering.constrained,
                 is_latlon=False,
                 use_ocg_layers=False):
        """Creates a cairo surface and context to render a PDF with.

        pagesize: tuple of page size in meters, see predefined sizes in pagessizes dict (default a4)
        margin: page margin in meters (default 0.01)
        box: box within the page to render the map into (will not render over margin). This should be 
             a Mapnik Box2d object. Default is the full page within the margin
        percent_box: as per box, but specified as a percent (0->1) of the full page size. If both box
                     and percent_box are specified percent_box will be used.
        scale: scale helper to use when rounding the map scale. This should be a function that
               takes a single float and returns a float which is at least as large as the value
               passed in. This is a 1:x scale.
        resolution: the resolution to render non vector elements at (in DPI), defaults to 72 DPI
        preserve_aspect: whether to preserve map aspect ratio. This defaults to True and it
                         is recommended you do not change it unless you know what you are doing
                         scales and so on will not work if this is False.
        centering: Centering rules for maps where the scale rounding has reduced the map size.
                   This should be a value from the centering class. The default is to center on the
                   maps constrained axis, typically this will be horizontal for portrait pages and
                   vertical for landscape pages.
        is_latlon: Is the map in lat lon degrees. If true magic anti meridian logic is enabled
        use_ocg_layers: Create OCG layers in the PDF, requires pyPdf >= 1.13
        """
        self._pagesize = pagesize
        self._margin = margin
        self._box = box
        self._scale = scale
        self._resolution = resolution
        self._preserve_aspect = preserve_aspect
        self._centering = centering
        self._is_latlon = is_latlon
        self._use_ocg_layers = use_ocg_layers

        self._s = None
        self._layer_names = []
        self._filename = None

        self.map_box = None
        self.scale = None

        # don't both to round the scale if they are not preserving the aspect ratio
        if not preserve_aspect:
            self._scale = any_scale

        if percent_box:
            self._box = Box2d(percent_box[0]*pagesize[0],percent_box[1]*pagesize[1],
                         percent_box[2]*pagesize[0],percent_box[3]*pagesize[1])

        if not HAS_PYCAIRO_MODULE:
            raise Exception("PDF rendering only available when pycairo is available")

        self.font_name = "DejaVu Sans"

    def finish(self):
        if self._s:
            self._s.finish()
            self._s = None

        if self._use_ocg_layers:
            convert_pdf_pages_to_layers(self._filename,layer_names=self._layer_names + ["Legend and Information"],reverse_all_but_last=True)

    def add_geospatial_pdf_header(self,m,filename,epsg=None,wkt=None):
        """ Postprocessing step to add geospatial PDF information to PDF file as per
        PDF standard 1.7 extension level 3 (also in draft PDF v2 standard at time of writing)

        one of either the epsg code or wkt text for the projection must be provided

        Should be called *after* the page has had .finish() called"""
        if HAS_PYPDF and (epsg or wkt):
            infile=file(filename,'rb')
            (outfd,outfilename) = tempfile.mkstemp(dir=os.path.dirname(filename))
            outfile = os.fdopen(outfd,'wb')

            i=pyPdf.PdfFileReader(infile)
            o=pyPdf.PdfFileWriter()

            # preserve OCProperties at document root if we have one
            if i.trailer['/Root'].has_key(pyPdf.generic.NameObject('/OCProperties')):
                o._root.getObject()[pyPdf.generic.NameObject('/OCProperties')] = i.trailer['/Root'].getObject()[pyPdf.generic.NameObject('/OCProperties')]

            for p in i.pages:
                gcs = pyPdf.generic.DictionaryObject()
                gcs[pyPdf.generic.NameObject('/Type')]=pyPdf.generic.NameObject('/PROJCS')
                if epsg:
                    gcs[pyPdf.generic.NameObject('/EPSG')]=pyPdf.generic.NumberObject(int(epsg))
                if wkt:
                    gcs[pyPdf.generic.NameObject('/WKT')]=pyPdf.generic.TextStringObject(wkt)

                measure = pyPdf.generic.DictionaryObject()
                measure[pyPdf.generic.NameObject('/Type')]=pyPdf.generic.NameObject('/Measure')
                measure[pyPdf.generic.NameObject('/Subtype')]=pyPdf.generic.NameObject('/GEO')
                measure[pyPdf.generic.NameObject('/GCS')]=gcs
                bounds=pyPdf.generic.ArrayObject()
                for x in (0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0):
                    bounds.append(pyPdf.generic.FloatObject(str(x)))
                measure[pyPdf.generic.NameObject('/Bounds')]=bounds
                measure[pyPdf.generic.NameObject('/LPTS')]=bounds
                gpts=pyPdf.generic.ArrayObject()

                proj=Projection(m.srs)
                env=m.envelope()
                for x in ((env.minx, env.miny), (env.minx, env.maxy), (env.maxx, env.maxy), (env.maxx, env.miny)):
                    latlon_corner=proj.inverse(Coord(*x))
                    # these are in lat,lon order according to the standard
                    gpts.append(pyPdf.generic.FloatObject(str(latlon_corner.y)))
                    gpts.append(pyPdf.generic.FloatObject(str(latlon_corner.x)))
                measure[pyPdf.generic.NameObject('/GPTS')]=gpts

                vp=pyPdf.generic.DictionaryObject()
                vp[pyPdf.generic.NameObject('/Type')]=pyPdf.generic.NameObject('/Viewport')
                bbox=pyPdf.generic.ArrayObject()

                for x in self.map_box:
                    bbox.append(pyPdf.generic.FloatObject(str(x)))
                vp[pyPdf.generic.NameObject('/BBox')]=bbox
                vp[pyPdf.generic.NameObject('/Measure')]=measure

                vpa = pyPdf.generic.ArrayObject()
                vpa.append(vp)
                p[pyPdf.generic.NameObject('/VP')]=vpa
                o.addPage(p)

            o.write(outfile)
            infile=None
            outfile.close()
            os.rename(outfilename,filename)


    def get_context(self):
        """allow access so that extra 'bits' can be rendered to the page directly"""
        return cairo.Context(self._s)

    def get_width(self):
        return self._pagesize[0]

    def get_height(self):
        return self._pagesize[1]

    def get_margin(self):
        return self._margin

    def write_text(self,ctx,text,box_width=None,size=10, fill_color=(0.0, 0.0, 0.0), alignment=None):
        if HAS_PANGOCAIRO_MODULE:
            (attr,t,accel) = pango.parse_markup(text)
            pctx = pangocairo.CairoContext(ctx)
            l = pctx.create_layout()
            l.set_attributes(attr)
            fd = pango.FontDescription("%s %d" % (self.font_name,size))
            l.set_font_description(fd)
            if box_width:
                l.set_width(int(box_width*pango.SCALE))
            if alignment:
                l.set_alignment(alignment)
            pctx.update_layout(l)
            l.set_text(t)
            pctx.set_source_rgb(*fill_color)
            pctx.show_layout(l)
            return l.get_pixel_extents()[0]

        else:
            ctx.rel_move_to(0,size)
            ctx.select_font_face(self.font_name, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
            ctx.set_font_size(size)
            ctx.show_text(text)
            ctx.rel_move_to(0,size)
            return (0,0,len(text)*size,size)

    def _get_context(self):
        if HAS_PANGOCAIRO_MODULE:
            return 
        elif HAS_PYCAIRO_MODULE:
            return cairo.Context(self._s)
        return None

    def _get_render_area(self):
        """return a bounding box with the area of the page we are allowed to render out map to
        in page coordinates (i.e. meters)
        """
        # take off our page margins
        render_area = Box2d(self._margin,self._margin,self._pagesize[0]-self._margin,self._pagesize[1]-self._margin)

        #then if user specified a box to render get intersection with that
        if self._box:
            return render_area.intersect(self._box)

        return render_area

    def _get_render_area_size(self):
        """Get the width and height (in meters) of the area we can render the map to, returned as a tuple"""
        render_area = self._get_render_area()
        return (render_area.width(),render_area.height())

    def _is_h_contrained(self,m):
        """Test if the map size is constrained on the horizontal or vertical axes"""
        available_area = self._get_render_area_size()
        map_aspect = m.envelope().width()/m.envelope().height()
        page_aspect = available_area[0]/available_area[1]

        return map_aspect > page_aspect

    def _get_meta_info_corner(self,render_size,m):
        """Get the corner (in page coordinates) of a possibly
        sensible place to render metadata such as a legend or scale"""
        (x,y) = self._get_render_corner(render_size,m)
        if self._is_h_contrained(m):
            y += render_size[1]+0.005
            x = self._margin
        else:
            x += render_size[0]+0.005
            y = self._margin

        return (x,y)

    def _get_render_corner(self,render_size,m):
        """Get the corner of the box we should render our map into"""
        available_area = self._get_render_area()

        x=available_area[0]
        y=available_area[1]

        h_is_contrained = self._is_h_contrained(m)

        if (self._centering == centering.both or
            self._centering == centering.horizontal or
            (self._centering == centering.constrained and h_is_contrained) or
            (self._centering == centering.unconstrained and not h_is_contrained)):
            x+=(available_area.width()-render_size[0])/2

        if (self._centering == centering.both or
            self._centering == centering.vertical or
            (self._centering == centering.constrained and not h_is_contrained) or
            (self._centering == centering.unconstrained and h_is_contrained)):
            y+=(available_area.height()-render_size[1])/2
        return (x,y)

    def _get_map_pixel_size(self, width_page_m, height_page_m):
        """for a given map size in paper coordinates return a tuple of the map 'pixel' size we
        should create at the defined resolution"""
        return (int(m2px(width_page_m,self._resolution)), int(m2px(height_page_m,self._resolution)))

    def render_map(self,m, filename):
        """Render the given map to filename"""

        # store this for later so we can post process the PDF
        self._filename = filename

        # work out the best scale to render out map at given the available space
        (eff_width,eff_height) = self._get_render_area_size()
        map_aspect = m.envelope().width()/m.envelope().height()
        page_aspect = eff_width/eff_height

        scalex=m.envelope().width()/eff_width
        scaley=m.envelope().height()/eff_height

        scale=max(scalex,scaley)

        rounded_mapscale=self._scale(scale)
        scalefactor = scale/rounded_mapscale
        mapw=eff_width*scalefactor
        maph=eff_height*scalefactor
        if self._preserve_aspect:
            if map_aspect > page_aspect:
                maph=mapw*(1/map_aspect)
            else:
                mapw=maph*map_aspect

        # set the map size so that raster elements render at the correct resolution
        m.resize(*self._get_map_pixel_size(mapw,maph))
        # calculate the translation for the map starting point
        (tx,ty) = self._get_render_corner((mapw,maph),m)

        # create our cairo surface and context and then render the map into it
        self._s = cairo.PDFSurface(filename, m2pt(self._pagesize[0]),m2pt(self._pagesize[1]))
        ctx=cairo.Context(self._s)

        for l in m.layers:
            # extract the layer names for naming layers if we use OCG
            self._layer_names.append(l.name)

            layer_map = Map(m.width,m.height,m.srs)
            layer_map.layers.append(l)
            for s in l.styles:
                layer_map.append_style(s,m.find_style(s))
            layer_map.zoom_to_box(m.envelope())

            def render_map():
                ctx.save()
                ctx.translate(m2pt(tx),m2pt(ty))
                #cairo defaults to 72dpi
                ctx.scale(72.0/self._resolution,72.0/self._resolution)
                render(layer_map, ctx)
                ctx.restore()

            # antimeridian
            render_map()
            if self._is_latlon and (m.envelope().minx < -180 or m.envelope().maxx > 180):
                old_env = m.envelope()
                if m.envelope().minx < -180:
                    delta = 360
                else:
                    delta = -360
                m.zoom_to_box(Box2d(old_env.minx+delta,old_env.miny,old_env.maxx+delta,old_env.maxy))
                render_map()
                # restore the original env
                m.zoom_to_box(old_env)

            if self._use_ocg_layers:
                self._s.show_page()

        self.scale = rounded_mapscale
        self.map_box = Box2d(tx,ty,tx+mapw,ty+maph)

    def render_on_map_lat_lon_grid(self,m,dec_degrees=True):
        # don't render lat_lon grid if we are already in latlon
        if self._is_latlon:
            return
        p2=Projection(m.srs)

        latlon_bounds = p2.inverse(m.envelope())
        if p2.inverse(m.envelope().center()).x > latlon_bounds.maxx:
            latlon_bounds = Box2d(latlon_bounds.maxx,latlon_bounds.miny,latlon_bounds.minx+360,latlon_bounds.maxy)

        if p2.inverse(m.envelope().center()).y > latlon_bounds.maxy:
            latlon_bounds = Box2d(latlon_bounds.miny,latlon_bounds.maxy,latlon_bounds.maxx,latlon_bounds.miny+360)

        latlon_mapwidth = latlon_bounds.width()
        # render an extra 20% so we generally won't miss the ends of lines
        latlon_buffer = 0.2*latlon_mapwidth
        if dec_degrees:
            latlon_divsize = default_scale(latlon_mapwidth/7.0)
        else:
            latlon_divsize = deg_min_sec_scale(latlon_mapwidth/7.0)
        latlon_interpsize = latlon_mapwidth/m.width

        self._render_lat_lon_axis(m,p2,latlon_bounds.minx,latlon_bounds.maxx,latlon_bounds.miny,latlon_bounds.maxy,latlon_buffer,latlon_interpsize,latlon_divsize,dec_degrees,True)
        self._render_lat_lon_axis(m,p2,latlon_bounds.miny,latlon_bounds.maxy,latlon_bounds.minx,latlon_bounds.maxx,latlon_buffer,latlon_interpsize,latlon_divsize,dec_degrees,False)

    def _render_lat_lon_axis(self,m,p2,x1,x2,y1,y2,latlon_buffer,latlon_interpsize,latlon_divsize,dec_degrees,is_x_axis):
        ctx=cairo.Context(self._s)
        ctx.set_source_rgb(1,0,0)
        ctx.set_line_width(1)
        latlon_labelsize = 6

        ctx.translate(m2pt(self.map_box.minx),m2pt(self.map_box.miny))
        ctx.rectangle(0,0,m2pt(self.map_box.width()),m2pt(self.map_box.height()))
        ctx.clip()

        ctx.select_font_face("DejaVu", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        ctx.set_font_size(latlon_labelsize)

        box_top = self.map_box.height()
        if not is_x_axis:
            ctx.translate(m2pt(self.map_box.width()/2),m2pt(self.map_box.height()/2))
            ctx.rotate(-math.pi/2)
            ctx.translate(-m2pt(self.map_box.height()/2),-m2pt(self.map_box.width()/2))
            box_top = self.map_box.width()

        for xvalue in round_grid_generator(x1 - latlon_buffer,x2 + latlon_buffer,latlon_divsize):
            yvalue = y1 - latlon_buffer
            start_cross = None
            end_cross = None
            while yvalue < y2+latlon_buffer:
                if is_x_axis:
                    start = m.view_transform().forward(p2.forward(Coord(xvalue,yvalue)))
                else:
                    temp = m.view_transform().forward(p2.forward(Coord(yvalue,xvalue)))
                    start = Coord(m2pt(self.map_box.height())-temp.y,temp.x)
                yvalue += latlon_interpsize
                if is_x_axis:
                    end = m.view_transform().forward(p2.forward(Coord(xvalue,yvalue)))
                else:
                    temp = m.view_transform().forward(p2.forward(Coord(yvalue,xvalue)))
                    end = Coord(m2pt(self.map_box.height())-temp.y,temp.x)

                ctx.move_to(start.x,start.y)
                ctx.line_to(end.x,end.y)
                ctx.stroke()

                if cmp(start.y, 0) != cmp(end.y,0):
                    start_cross = end.x
                if cmp(start.y,m2pt(self.map_box.height())) != cmp(end.y, m2pt(self.map_box.height())):
                    end_cross = end.x

            if dec_degrees:
                line_text = "%g" % (xvalue)
            else:
                line_text = format_deg_min_sec(xvalue)
            if start_cross:
                ctx.move_to(start_cross+2,latlon_labelsize)
                ctx.show_text(line_text)
            if end_cross:
                ctx.move_to(end_cross+2,m2pt(box_top)-2)
                ctx.show_text(line_text)

    def render_on_map_scale(self,m):
        (div_size,page_div_size) = self._get_sensible_scalebar_size(m)

        first_value_x = (math.floor(m.envelope().minx / div_size) + 1) * div_size
        first_value_x_percent = (first_value_x-m.envelope().minx)/m.envelope().width()
        self._render_scale_axis(first_value_x,first_value_x_percent,self.map_box.minx,self.map_box.maxx,page_div_size,div_size,self.map_box.miny,self.map_box.maxy,True)

        first_value_y = (math.floor(m.envelope().miny / div_size) + 1) * div_size
        first_value_y_percent = (first_value_y-m.envelope().miny)/m.envelope().height()
        self._render_scale_axis(first_value_y,first_value_y_percent,self.map_box.miny,self.map_box.maxy,page_div_size,div_size,self.map_box.minx,self.map_box.maxx,False)

        if self._use_ocg_layers:
            self._s.show_page()
            self._layer_names.append("Coordinate Grid Overlay")

    def _get_sensible_scalebar_size(self,m,width=-1):
        # aim for about 8 divisions across the map
        # also make sure we can fit the bar with in page area width if specified
        div_size = sequence_scale(m.envelope().width()/8, [1,2,5])
        page_div_size = self.map_box.width()*div_size/m.envelope().width()
        while width > 0 and page_div_size > width:
            div_size /=2
            page_div_size /= 2
        return (div_size,page_div_size)

    def _render_box(self,ctx,x,y,w,h,text=None,stroke_color=(0,0,0),fill_color=(0,0,0)):
        ctx.set_line_width(1)
        ctx.set_source_rgb(*fill_color)
        ctx.rectangle(x,y,w,h)
        ctx.fill()

        ctx.set_source_rgb(*stroke_color)
        ctx.rectangle(x,y,w,h)
        ctx.stroke()

        if text:
            ctx.move_to(x+1,y)
            self.write_text(ctx,text,fill_color=[1-z for z in fill_color],size=h-2)

    def _render_scale_axis(self,first,first_percent,start,end,page_div_size,div_size,boundary_start,boundary_end,is_x_axis):
        prev = start
        text = None
        fill=(0,0,0)
        border_size=8
        value = first_percent * (end-start) + start
        label_value = first-div_size
        if self._is_latlon and label_value < -180:
            label_value += 360

        ctx=cairo.Context(self._s)

        if not is_x_axis:
            ctx.translate(m2pt(self.map_box.center().x),m2pt(self.map_box.center().y))
            ctx.rotate(-math.pi/2)
            ctx.translate(-m2pt(self.map_box.center().y),-m2pt(self.map_box.center().x))

        while value < end:
            ctx.move_to(m2pt(value),m2pt(boundary_start))
            ctx.line_to(m2pt(value),m2pt(boundary_end))
            ctx.set_source_rgb(0.5,0.5,0.5)
            ctx.set_line_width(1)
            ctx.stroke()

            for bar in (m2pt(boundary_start)-border_size,m2pt(boundary_end)):
                self._render_box(ctx,m2pt(prev),bar,m2pt(value-prev),border_size,text,fill_color=fill)

            prev = value
            value+=page_div_size
            fill = [1-z for z in fill]
            label_value += div_size 
            if self._is_latlon and label_value > 180:
                label_value -= 360
            text = "%d" % label_value
        else:
            for bar in (m2pt(boundary_start)-border_size,m2pt(boundary_end)):
                self._render_box(ctx,m2pt(prev),bar,m2pt(end-prev),border_size,fill_color=fill)


    def render_scale(self,m,ctx=None,width=0.05):
        """ m: map to render scale for
        ctx: A cairo context to render the scale to. If this is None (the default) then
            automatically create a context and choose the best location for the scale bar.
        width: Width of area available to render scale bar in (in m)

        will return the size of the rendered scale block in pts
        """

        (w,h) = (0,0)

        # don't render scale if we are lat lon
        # dont report scale if we have warped the aspect ratio
        if self._preserve_aspect and not self._is_latlon:
            bar_size=8.0
            box_count=3
            if ctx is None:
                ctx=cairo.Context(self._s)
                (tx,ty) = self._get_meta_info_corner((self.map_box.width(),self.map_box.height()),m)
                ctx.translate(tx,ty)

            (div_size,page_div_size) = self._get_sensible_scalebar_size(m, width/box_count)


            div_unit = "m"
            if div_size > 1000:
                div_size /= 1000
                div_unit = "km"

            text = "0%s" % div_unit
            ctx.save()
            if width > 0:
                ctx.translate(m2pt(width-box_count*page_div_size)/2,0)
            for ii in range(box_count):
                fill=(ii%2,)*3
                self._render_box(ctx, m2pt(ii*page_div_size), h, m2pt(page_div_size), bar_size, text, fill_color=fill)
                fill = [1-z for z in fill]
                text = "%g%s" % ((ii+1)*div_size,div_unit)
            #else:
            #    self._render_box(ctx, m2pt(box_count*page_div_size), h, m2pt(page_div_size), bar_size, text, fill_color=(1,1,1), stroke_color=(1,1,1))
            w = (box_count)*page_div_size
            h += bar_size
            ctx.restore()

            if width > 0:
                box_width=m2pt(width)
            else:
                box_width = None

            font_size=6
            ctx.move_to(0,h)
            if HAS_PANGOCAIRO_MODULE:
                alignment = pango.ALIGN_CENTER
            else:
                alignment = None

            text_ext=self.write_text(ctx,"Scale 1:%d" % self.scale,box_width=box_width,size=font_size, alignment=alignment)
            h+=text_ext[3]+2

        return (w,h)

    def render_legend(self,m, page_break=False, ctx=None, collumns=1,width=None, height=None, item_per_rule=False, attribution={}, legend_item_box_size=(0.015,0.0075)):
        """ m: map to render legend for
        ctx: A cairo context to render the legend to. If this is None (the default) then
            automatically create a context and choose the best location for the legend.
        width: Width of area available to render legend in (in m)
        page_break: move to next page if legen over flows this one
        collumns: number of collumns available in legend box
        attribution: additional text that will be rendered in gray under the layer name. keyed by layer name
        legend_item_box_size:  two tuple with width and height of legend item box size in meters

        will return the size of the rendered block in pts
        """

        (w,h) = (0,0)
        if self._s:
            if ctx is None:
                ctx=cairo.Context(self._s)
                (tx,ty) = self._get_meta_info_corner((self.map_box.width(),self.map_box.height()),m)
                ctx.translate(m2pt(tx),m2pt(ty))
                width = self._pagesize[0]-2*tx
                height = self._pagesize[1]-self._margin-ty

            x=0
            y=0
            if width:
                cwidth = width/collumns
                w=m2pt(width)
            else:
                cwidth = None
            current_collumn = 0

            processed_layers = []
            for l in reversed(m.layers):
                have_layer_header = False
                added_styles={}
                layer_title = l.name
                if layer_title in processed_layers:
                    continue
                processed_layers.append(layer_title)

                # check through the features to find which combinations of styles are active
                # for each unique combination add a legend entry
                for f in l.datasource.all_features():
                    if f.num_geometries() > 0:
                        active_rules = []
                        rule_text = ""
                        for s in l.styles:
                            st = m.find_style(s)
                            for r in st.rules:
                                # we need to do the scale test here as well so we don't
                                # add unused scale rules to the legend description
                                if ((not r.filter) or r.filter.evaluate(f) == '1') and \
                                    r.min_scale <= m.scale_denominator() and m.scale_denominator() < r.max_scale:
                                    active_rules.append((s,r.name))
                                    if r.filter and str(r.filter) != "true":
                                        if len(rule_text) > 0:
                                            rule_text += " AND "
                                        if r.name:
                                            rule_text += r.name
                                        else:
                                            rule_text += str(r.filter)
                        active_rules = tuple(active_rules)
                        if added_styles.has_key(active_rules):
                            continue

                        added_styles[active_rules] = (f,rule_text)
                        if not item_per_rule:
                            break
                    else:
                        added_styles[l] = (None,None)

                legend_items = added_styles.keys()
                legend_items.sort()
                for li in legend_items:
                    if True:
                        (f,rule_text) = added_styles[li]


                        legend_map_size = (int(m2pt(legend_item_box_size[0])),int(m2pt(legend_item_box_size[1])))
                        lemap=Map(legend_map_size[0],legend_map_size[1],srs=m.srs)
                        if m.background:
                            lemap.background = m.background
                        # the buffer is needed to ensure that text labels that overflow the edge of the
                        # map still render for the legend
                        lemap.buffer_size=1000
                        for s in l.styles:
                            sty=m.find_style(s)
                            lestyle = Style()
                            for r in sty.rules:
                                for sym in r.symbols:
                                    try:
                                        sym.avoid_edges=False
                                    except:
                                        print "**** Cant set avoid edges for rule", r.name
                                if r.min_scale <= m.scale_denominator() and m.scale_denominator() < r.max_scale:
                                    lerule = r
                                    lerule.min_scale = 0
                                    lerule.max_scale = float("inf")
                                    lestyle.rules.append(lerule)
                            lemap.append_style(s,lestyle)

                        ds = MemoryDatasource()
                        if f is None:
                            ds=l.datasource
                            layer_srs = l.srs
                        elif f.envelope().width() == 0:
                            ds.add_feature(Feature(f.id(),Geometry2d.from_wkt("POINT(0 0)"),**f.attributes))
                            lemap.zoom_to_box(Box2d(-1,-1,1,1))
                            layer_srs = m.srs
                        else:
                            ds.add_feature(f)
                            layer_srs = l.srs

                        lelayer = Layer("LegendLayer",layer_srs)
                        lelayer.datasource = ds
                        for s in l.styles:
                            lelayer.styles.append(s)
                        lemap.layers.append(lelayer)

                        if f is None or f.envelope().width() != 0:
                            lemap.zoom_all()
                            lemap.zoom(1.1)

                        item_size = legend_map_size[1]
                        if not have_layer_header:
                            item_size += 8

                        if y+item_size > m2pt(height):
                            current_collumn += 1
                            y=0
                            if current_collumn >= collumns:
                                if page_break:
                                    self._s.show_page()
                                    x=0
                                    current_collumn = 0
                                else:
                                    break

                        if not have_layer_header and item_per_rule:
                            ctx.move_to(x+m2pt(current_collumn*cwidth),y)
                            e=self.write_text(ctx, l.name, m2pt(cwidth), 8)
                            y+=e[3]+2
                            have_layer_header = True
                        ctx.save()
                        ctx.translate(x+m2pt(current_collumn*cwidth),y)
                        #extra save around map render as it sets up a clip box and doesn't clear it
                        ctx.save()
                        render(lemap, ctx)
                        ctx.restore()

                        ctx.rectangle(0,0,*legend_map_size)
                        ctx.set_source_rgb(0.5,0.5,0.5)
                        ctx.set_line_width(1)
                        ctx.stroke()
                        ctx.restore()

                        ctx.move_to(x+legend_map_size[0]+m2pt(current_collumn*cwidth)+2,y)
                        legend_entry_size = legend_map_size[1]
                        legend_text_size = 0
                        if not item_per_rule:
                            rule_text = layer_title
                        if rule_text:
                            e=self.write_text(ctx, rule_text, m2pt(cwidth-legend_item_box_size[0]-0.005), 6)
                            legend_text_size += e[3]
                            ctx.rel_move_to(0,e[3])
                        if attribution.has_key(layer_title):
                            e=self.write_text(ctx, attribution[layer_title], m2pt(cwidth-legend_item_box_size[0]-0.005), 6, fill_color=(0.5,0.5,0.5))
                            legend_text_size += e[3]

                        if legend_text_size > legend_entry_size:
                            legend_entry_size=legend_text_size

                        y+=legend_entry_size +2
                        if y > h:
                            h = y
        return (w,h)

