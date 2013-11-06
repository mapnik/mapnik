/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  1.1                                                             *
* Date      :  4 April 2011                                                    *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2011                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#ifndef AGG_CONV_CLIPPER_INCLUDED
#define AGG_CONV_CLIPPER_INCLUDED

#include <cmath>
#include "agg_basics.h"
#include "agg_array.h"
#include "clipper.hpp"

namespace agg
{
  enum clipper_op_e { clipper_or,
    clipper_and, clipper_xor, clipper_a_minus_b, clipper_b_minus_a };
  enum clipper_PolyFillType {clipper_even_odd, clipper_non_zero, clipper_positive, clipper_negative};

  template<class VSA, class VSB> class conv_clipper
  {
    enum status { status_move_to, status_line_to, status_stop };
    typedef VSA source_a_type;
    typedef VSB source_b_type;
    typedef conv_clipper<source_a_type, source_b_type> self_type;

  private:
    source_a_type*                            m_src_a;
    source_b_type*                            m_src_b;
    status                                    m_status;
    int                                        m_vertex;
    int                                        m_contour;
    int                                        m_scaling_factor;
    clipper_op_e              m_operation;
    pod_bvector<ClipperLib::IntPoint, 8>        m_vertex_accumulator;
    ClipperLib::Paths         m_poly_a;
    ClipperLib::Paths         m_poly_b;
    ClipperLib::Paths         m_result;
    ClipperLib::Clipper       m_clipper;
    clipper_PolyFillType      m_subjFillType;
    clipper_PolyFillType      m_clipFillType;

    int Round(double val)
    {
    if ((val < 0)) return (int)(val - 0.5); else return (int)(val + 0.5);
    }

  public:
    conv_clipper(source_a_type &a, source_b_type &b,
      clipper_op_e op = clipper_or,
      clipper_PolyFillType subjFillType = clipper_even_odd,
      clipper_PolyFillType clipFillType = clipper_even_odd,
      int scaling_factor = 2) :
        m_src_a(&a),
        m_src_b(&b),
        m_status(status_move_to),
        m_vertex(-1),
        m_contour(-1),
        m_operation(op),
        m_subjFillType(subjFillType),
        m_clipFillType(clipFillType)
    {
        m_scaling_factor = std::max(std::min(scaling_factor, 6),0);
        m_scaling_factor = Round(std::pow((double)10, m_scaling_factor));
    }

    ~conv_clipper()
    {
    }

    void attach1(VSA &source, clipper_PolyFillType subjFillType = clipper_even_odd)
      { m_src_a = &source; m_subjFillType = subjFillType; }
    void attach2(VSB &source, clipper_PolyFillType clipFillType = clipper_even_odd)
      { m_src_b = &source; m_clipFillType = clipFillType; }

    void operation(clipper_op_e v) { m_operation = v; }

    void rewind(unsigned path_id);
    unsigned vertex(double* x, double* y);
  
    bool next_contour();
    bool next_vertex(double* x, double* y);
    void start_extracting();
    void add_vertex_(double &x, double &y);
    void end_contour(ClipperLib::Paths &p);

    template<class VS> void add(VS &src, ClipperLib::Paths &p){
        unsigned cmd;
        double x; double y; double start_x; double start_y;
        bool starting_first_line;

        start_x = 0.0;
        start_y = 0.0;
        starting_first_line = true;
        p.resize(0);

        cmd = src->vertex( &x , &y );
        while(!is_stop(cmd))
        {
          if(is_vertex(cmd))
          {
            if(is_move_to(cmd))
            {
              if(!starting_first_line ) end_contour(p);
              start_x = x;
              start_y = y;
            }
            add_vertex_( x, y );
            starting_first_line = false;
          }
          else if(is_end_poly(cmd))
          {
            if(!starting_first_line && is_closed(cmd))
              add_vertex_( start_x, start_y );
          }
          cmd = src->vertex( &x, &y );
        }
        end_contour(p);
    }
  };

  //------------------------------------------------------------------------

  template<class VSA, class VSB> 
  void conv_clipper<VSA, VSB>::start_extracting()
  {
    m_status = status_move_to;
    m_contour = -1;
    m_vertex = -1;
  }
  //------------------------------------------------------------------------------

  template<class VSA, class VSB>
  void conv_clipper<VSA, VSB>::rewind(unsigned path_id)
  {
    m_src_a->rewind( path_id );
    m_src_b->rewind( path_id );

    add( m_src_a , m_poly_a );
    add( m_src_b , m_poly_b );
    m_result.resize(0);

    ClipperLib::PolyFillType pftSubj, pftClip;
    switch (m_subjFillType)
    {
      case clipper_even_odd: pftSubj = ClipperLib::pftEvenOdd; break;
      case clipper_non_zero: pftSubj = ClipperLib::pftNonZero; break;
      case clipper_positive: pftSubj = ClipperLib::pftPositive; break;
      default: pftSubj = ClipperLib::pftNegative;
    }
    switch (m_clipFillType)
    {
      case clipper_even_odd: pftClip = ClipperLib::pftEvenOdd; break;
      case clipper_non_zero: pftClip = ClipperLib::pftNonZero; break;
      case clipper_positive: pftClip = ClipperLib::pftPositive; break;
      default: pftClip = ClipperLib::pftNegative;
    }

    m_clipper.Clear();
    switch( m_operation ) {
      case clipper_or:
        {
        m_clipper.AddPaths( m_poly_a , ClipperLib::ptSubject, true );
        m_clipper.AddPaths( m_poly_b , ClipperLib::ptClip, true );
        m_clipper.Execute( ClipperLib::ctUnion , m_result , pftSubj, pftClip);
        break;
        }
      case clipper_and:
        {
        m_clipper.AddPaths( m_poly_a , ClipperLib::ptSubject, true );
        m_clipper.AddPaths( m_poly_b , ClipperLib::ptClip, true );
        m_clipper.Execute( ClipperLib::ctIntersection , m_result, pftSubj, pftClip );
        break;
        }
      case clipper_xor:
        {
        m_clipper.AddPaths( m_poly_a , ClipperLib::ptSubject, true );
        m_clipper.AddPaths( m_poly_b , ClipperLib::ptClip, true );
        m_clipper.Execute( ClipperLib::ctXor , m_result, pftSubj, pftClip );
        break;
        }
      case clipper_a_minus_b:
        {
        m_clipper.AddPaths( m_poly_a , ClipperLib::ptSubject, true );
        m_clipper.AddPaths( m_poly_b , ClipperLib::ptClip, true );
        m_clipper.Execute( ClipperLib::ctDifference , m_result, pftSubj, pftClip );
        break;
        }
      case clipper_b_minus_a:
        {
        m_clipper.AddPaths( m_poly_b , ClipperLib::ptSubject, true );
        m_clipper.AddPaths( m_poly_a , ClipperLib::ptClip, true );
        m_clipper.Execute( ClipperLib::ctDifference , m_result, pftSubj, pftClip );
        break;
        }
    }
    start_extracting();
  }
  //------------------------------------------------------------------------------

  template<class VSA, class VSB>
  void conv_clipper<VSA, VSB>::end_contour( ClipperLib::Paths &p)
  {
  unsigned i, len;

  if( m_vertex_accumulator.size() < 3 ) return;
  len = p.size();
  p.resize(len+1);
  p[len].resize(m_vertex_accumulator.size());
  for( i = 0 ; i < m_vertex_accumulator.size() ; i++ )
    p[len][i] = m_vertex_accumulator[i];
  m_vertex_accumulator.remove_all();
  }
  //------------------------------------------------------------------------------

  template<class VSA, class VSB> 
  void conv_clipper<VSA, VSB>::add_vertex_(double &x, double &y)
  {
      ClipperLib::IntPoint v;

      v.X = Round(x * m_scaling_factor);
      v.Y = Round(y * m_scaling_factor);
      m_vertex_accumulator.add( v );
  }
  //------------------------------------------------------------------------------

  template<class VSA, class VSB> 
  bool conv_clipper<VSA, VSB>::next_contour()
  {   
    m_contour++;
    if(m_contour >= (int)m_result.size()) return false;
    m_vertex =-1;
    return true;
}
//------------------------------------------------------------------------------

  template<class VSA, class VSB> 
  bool conv_clipper<VSA, VSB>::next_vertex(double *x, double *y)
  {
    m_vertex++;
    if(m_vertex >= (int)m_result[m_contour].size()) return false;
    *x = (double)m_result[ m_contour ][ m_vertex ].X / m_scaling_factor;
    *y = (double)m_result[ m_contour ][ m_vertex ].Y / m_scaling_factor;
    return true;
  }
  //------------------------------------------------------------------------------

  template<class VSA, class VSB>
  unsigned conv_clipper<VSA, VSB>::vertex(double *x, double *y)
{
  if(  m_status == status_move_to )
  {
    if( next_contour() )
    {
      if(  next_vertex( x, y ) )
      {
        m_status =status_line_to;
        return path_cmd_move_to;
      }
      else
      {
        m_status = status_stop;
        return path_cmd_end_poly | path_flags_close;
      }
    }
    else
      return path_cmd_stop;
  }
  else
  {
    if(  next_vertex( x, y ) )
    {
      return path_cmd_line_to;
    }
    else
    {
      m_status = status_move_to;
      return path_cmd_end_poly | path_flags_close;
    }
  }
}
//------------------------------------------------------------------------------


} //namespace agg
#endif //AGG_CONV_CLIPPER_INCLUDED
