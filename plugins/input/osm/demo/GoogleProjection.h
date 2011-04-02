#ifndef GOOGLEPROJECTION_H
#define GOOGLEPROJECTION_H

#include <vector>
using std::vector;
#include <cmath>

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

struct ScreenPos
{
  int x,y;
  ScreenPos() { x=y=0; }
  ScreenPos(int x,int y) { this->x=x; this->y=y; }
};

struct EarthPoint
{
  double x,y; 
  EarthPoint() { x=y=0.0; }
  EarthPoint(double x,double y) { this->x=x; this->y=y; }
};

class GoogleProjection
{
   private:

   vector<double> Bc,Cc,zc,Ac;
   int levels;

  double minmax (double a,double b, double c)
  {
      a = max(a,b);
      a = min(a,c);
      return a;
  }

  public:
   GoogleProjection(int levels=18)
   {
       this->levels=levels;
        double c = 256;
    double e;
        for (int d=0; d<levels; d++) 
    {
            e = c/2;
            Bc.push_back(c/360.0);
            Cc.push_back(c/(2 * M_PI));
            zc.push_back(e);
            Ac.push_back(c);
            c *= 2;
    }
  }
                
  ScreenPos fromLLToPixel(double lon,double lat,int zoom)
  {
    double d = zc[zoom];
    double e = round(d + lon * Bc[zoom]);
    double f = minmax(sin((M_PI/180.0) * lat),-0.9999,0.9999);
    double g = round(d + 0.5*log((1+f)/(1-f))*-Cc[zoom]);
    return ScreenPos(e,g);
  }

  EarthPoint fromPixelToLL(int x,int y,int zoom)
  {
    double e = zc[zoom];
    double f = (x - e)/Bc[zoom];
    double g = (y - e)/-Cc[zoom];
    double h = (180.0/M_PI) * ( 2 * atan(exp(g)) - 0.5 * M_PI);
    return EarthPoint(f,h);
  }

  // convert to the zoom independent Google system; TBH I don't really
  // understand what it represents.... 
  static EarthPoint fromLLToGoog(double lon,double lat)
  {
    double a = log(tan((90+lat)*M_PI / 360))/(M_PI / 180);
     double custLat = a * 20037508.34 / 180;
    double custLon=lon;
     custLon = custLon * 20037508.34 / 180;
    return EarthPoint(custLon,custLat);
  }

  // other way round
  static EarthPoint fromGoogToLL(double x,double y)
  {
    double lat_deg,lon_deg;
    lat_deg = (y / 20037508.34) * 180;
         lon_deg = (x / 20037508.34) * 180;
         lat_deg = 180/M_PI * 
           (2 * atan(exp(lat_deg * M_PI / 180)) - M_PI / 2);
    return EarthPoint(lon_deg,lat_deg);
  }
};

#endif // GOOGLEPROJECTION_H
