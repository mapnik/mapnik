#!/usr/bin/env /opt/python/bin/python
import sys
from qt import *
from qtcanvas import *
from dl import *
sys.setdlopenflags(RTLD_NOW | RTLD_GLOBAL)
from mapnik import *
        
def load_styles() :
    print 'loading styles...'
    styles=style_cache.instance()
    styles.insert("mystyle",style(create_polygon_symbolizer(color(255,255,204))))
    styles.insert("water",style(create_polygon_symbolizer(color(102,153,255))))
    styles.insert("parks",style(create_polygon_symbolizer(color(200,230,120))))
    styles.insert("roads",style(create_line_symbolizer(color(204,204,153),2.3)))
    styles.insert("dots",style(create_point_symbolizer("/home/artem/smiley.png",12,12)))
    print 'done'

def create_map(width,height) :
    
    m=map(width,height)
    bg=color(102,153,255)
    m.background(bg)

    p={"type":"raster",
       "format":"tiff",
       "name":"world dem",
       "file":"/home/artem/projects/data/raster/world_tiled1.tif",
       "lox":"-180","loy":"-90","hix":"180","hiy":"90"}
    lyr=create_layer(p)
    lyr.minzoom(0.2)
    lyr.maxzoom(1.0)
    m.add(lyr)
     
    p={"type":"raster",
       "format":"tiff",
       "name":"world dem",
       "file":"/home/artem/projects/data/raster/world_tiled2.tif",
       "lox":"-180","loy":"-90","hix":"180","hiy":"90"}
    print p
    lyr=create_layer(p)
    lyr.minzoom(0.005)
    lyr.maxzoom(0.2)
    m.add(lyr)
    
    p=parameters()
    p.add("type","shape")
    p.add("name","usa")
    p.add("file","/home/artem/projects/data/usa/dtl_cnty")
    lyr=layer(p)
    #lyr.maxzoom(0.005)
    lyr.style("mystyle")
    m.add(lyr)

    p=parameters()
    #p.add("type","shape")
    #p.add("file","/home/artem/projects/data/usa/mjwater")
    p.add("type","postgis")
    p.add("host","localhost")
    p.add("dbname","gis")
    p.add("post","5432")
    p.add("user","postgres")
    p.add("name","water")
    p.add("table","water")
    
    lyr=layer(p)
    lyr.maxzoom(0.005)
    lyr.style("water")
    m.add(lyr)
    
    p=parameters()
    p.add("type","shape")
    p.add("name","parks")
    p.add("file","/home/artem/projects/data/usa/parks")
    lyr=layer(p)
    lyr.maxzoom(0.02)
    lyr.style("parks")
    m.add(lyr)

    p=parameters()
    p.add("type","shape")
    p.add("name","tiger")
    p.add("file","/home/artem/projects/data/tiger/ca/tiger")
    lyr=layer(p)
    lyr.maxzoom(0.0005)
    lyr.style("roads")
    m.add(lyr)

    p=parameters()
    #p.add("type","shape")
    #p.add("name","maj roads")
    p.add("type","postgis")
    p.add("host","localhost")
    p.add("dbname","gis")
    p.add("post","5432")
    p.add("user","postgres")
    p.add("name","roads")
    p.add("table","roads")
    
    #p.add("file","/home/artem/projects/data/usa/mjrrds")
    lyr=layer(p)
    lyr.minzoom(0.0005)
    lyr.maxzoom(0.005)
    lyr.style("roads")
    m.add(lyr)
    
    p=parameters()
    #p.add("type","shape")
    
    p.add("name","cities")
    #p.add("file","/home/artem/projects/data/usa/cities")
    p.add("type","postgis")
    p.add("host","localhost")
    p.add("dbname","gis")
    p.add("post","5432")
    p.add("user","postgres")
    p.add("table","cities")
    lyr=layer(p)
    lyr.style("dots")
    m.add(lyr)
    
    box=envelope(-123.24,37.094,-121.349,38.512)
    m.zoom_to_box(box)
    return m

class MapWidget(QWidget) :
    def __init__(self,*args):
        QWidget.__init__(self,*args)
        self.buffer = QPixmap(600,300)
        self.buffer.fill(QColor(200,200,200))
        #self.pixbuf.load("/home/artem/map.png")
        self.map=create_map(600,300)
            
    def paintEvent(self,ev):
        bitBlt(self,0,0,self.buffer)
        
    def keyPressEvent(self,ev) :
        if ev.key() == 45 :
            self.zoom_out()
            self.render_image()
        elif ev.key() == 61:
            self.zoom_in()
            self.render_image()
        else:
             QWidget.keyPressEvent(self,ev)
        
    def mousePressEvent(self,ev):
        if ev.button() == 1 :
            self.map.pan(ev.x(),ev.y())
            self.render_image()      
        else :
            print "todo"

    def zoom_in (self) :
        self.map.zoom(0.5)

    def zoom_out(self):
        self.map.zoom(2.0)
        
    def render_image (self) :
        render_to_file(self.map,"/home/artem/map.png","png")
        self.buffer.load("/home/artem/map.png")
        self.repaint()

        
class MainWindow(QMainWindow):
    def __init__(self,*args):
        QMainWindow.__init__(self,*args)
        self.setCaption("simple map viewer")
        self.painting = MapWidget(self)
        self.setCentralWidget(self.painting)
        
        
def main(args):
    app=QApplication(args)
    load_styles()
    win=MapWidget()
    win.resize(600,300)
    win.show()
    win.setFocusPolicy(QWidget.StrongFocus)
    app.connect(app, SIGNAL("lastWindowClosed()"),
                app, SLOT("quit()"))
    app.exec_loop()

if __name__=="__main__":
    main(sys.argv)
