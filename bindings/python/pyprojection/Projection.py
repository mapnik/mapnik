# This file was created automatically by SWIG.
import Projectionc
class Projection:
    def __init__(self,*args):
        self.this = apply(Projectionc.new_Projection,args)
        self.thisown = 1

    def __del__(self,Projectionc=Projectionc):
        if self.thisown == 1 :
            Projectionc.delete_Projection(self)
    def Forward(*args):
        val = apply(Projectionc.Projection_Forward,args)
        return val
    def Inverse(*args):
        val = apply(Projectionc.Projection_Inverse,args)
        return val
    def cobject(*args):
        val = apply(Projectionc.Projection_cobject,args)
        return val
    __setmethods__ = {
        "units" : Projectionc.Projection_units_set,
        "proj" : Projectionc.Projection_proj_set,
    }
    def __setattr__(self,name,value):
        if (name == "this") or (name == "thisown"): self.__dict__[name] = value; return
        method = Projection.__setmethods__.get(name,None)
        if method: return method(self,value)
        self.__dict__[name] = value
    __getmethods__ = {
        "units" : Projectionc.Projection_units_get,
        "proj" : Projectionc.Projection_proj_get,
    }
    def __getattr__(self,name):
        method = Projection.__getmethods__.get(name,None)
        if method: return method(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C Projection instance at %s>" % (self.this,)
    
    def __del__(self,Projectionc=Projectionc):
        if getattr(self, 'thisown', 0):
            Projectionc.delete_Projection(self)
    
class ProjectionPtr(Projection):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = Projection





#-------------- FUNCTION WRAPPERS ------------------

int_array = Projectionc.int_array

int_destroy = Projectionc.int_destroy

int_get = Projectionc.int_get

int_set = Projectionc.int_set

double_array = Projectionc.double_array

double_destroy = Projectionc.double_destroy

double_get = Projectionc.double_get

double_set = Projectionc.double_set

float_array = Projectionc.float_array

float_destroy = Projectionc.float_destroy

float_get = Projectionc.float_get

float_set = Projectionc.float_set

string_array = Projectionc.string_array

string_destroy = Projectionc.string_destroy

string_get = Projectionc.string_get

string_set = Projectionc.string_set



#-------------- VARIABLE WRAPPERS ------------------

DEGREES = Projectionc.DEGREES
RADIANS = Projectionc.RADIANS
