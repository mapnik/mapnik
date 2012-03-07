// mapnik
#include <mapnik/markers_placement.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/global.hpp> //round
// agg
#include "agg_basics.h"
#include "agg_conv_clip_polyline.h"
// stl
#include <cmath>

namespace mapnik
{
template <typename Locator,  typename Detector>
markers_placement<Locator, Detector>::markers_placement(
    Locator &locator, box2d<double> size, Detector &detector, double spacing, double max_error, bool allow_overlap)
    : locator_(locator), size_(size), detector_(detector), max_error_(max_error), allow_overlap_(allow_overlap)
{
    if (spacing >= 0)
    {
        spacing_ = spacing;
    } else if (spacing < 0)
    {
        spacing_ = find_optimal_spacing(-spacing);
    }
    rewind();
}

template <typename Locator, typename Detector>
double markers_placement<Locator, Detector>::find_optimal_spacing(double s)
{
    rewind();
    //Calculate total path length
    unsigned cmd = agg::path_cmd_move_to;
    double length = 0;
    while (!agg::is_stop(cmd))
    {
        double dx = next_x - last_x;
        double dy = next_y - last_y;
        length += std::sqrt(dx * dx + dy * dy);
        last_x = next_x;
        last_y = next_y;
        while (agg::is_move_to(cmd = locator_.vertex(&next_x, &next_y)))
        {
            //Skip over "move" commands
            last_x = next_x;
            last_y = next_y;
        }
    }
    unsigned points = round(length / s);
    if (points == 0) return 0.0; //Path to short
    return length / points;
}


template <typename Locator, typename Detector>
void markers_placement<Locator, Detector>::rewind()
{
    locator_.rewind(0);
    //Get first point
    done_ = agg::is_stop(locator_.vertex(&next_x, &next_y)) || spacing_ < size_.width();
    last_x = next_x;
    last_y = next_y; // Force request of new segment
    error_ = 0;
    marker_nr_ = 0;
}

template <typename Locator, typename Detector>
bool markers_placement<Locator, Detector>::get_point(
    double *x, double *y, double *angle, bool add_to_detector)
{
    if (done_) return false;

    unsigned cmd;

    /* This functions starts at the position of the previous marker,
       walks along the path, counting how far it has to go in spacing_left.
       If one marker can't be placed at the position it should go to it is
       moved a bit. The error is compensated for in the next call to this
       function.

       error > 0: Marker too near to the end of the path.
       error = 0: Perfect position.
       error < 0: Marker too near to the beginning of the path.
    */

    if (marker_nr_++ == 0)
    {
        //First marker
        spacing_left_ = spacing_ / 2;
    } else
    {
        spacing_left_ = spacing_;
    }

    spacing_left_ -= error_;
    error_ = 0;

    //Loop exits when a position is found or when no more segments are available
    while (true)
    {
        //Do not place markers too close to the beginning of a segment
        if (spacing_left_ < size_.width()/2)
        {
            set_spacing_left(size_.width()/2); //Only moves forward
        }
        //Error for this marker is too large. Skip to the next position.
        if (abs(error_) > max_error_ * spacing_)
        {
            if (error_ > spacing_) {
                error_ = 0; //Avoid moving backwards
#ifdef MAPNIK_DEBUG
                std::cerr << "WARNING: Extremely large error in markers_placement. Please file a bug report.\n";
#endif
            }
            spacing_left_ += spacing_ - error_;
            error_ = 0;
        }
        double dx = next_x - last_x;
        double dy = next_y - last_y;
        double segment_length = std::sqrt(dx * dx + dy * dy);
        if (segment_length <= spacing_left_)
        {
            //Segment is to short to place marker. Find next segment
            spacing_left_ -= segment_length;
            last_x = next_x;
            last_y = next_y;
            while (agg::is_move_to(cmd = locator_.vertex(&next_x, &next_y)))
            {
                //Skip over "move" commands
                last_x = next_x;
                last_y = next_y;
            }
            if (agg::is_stop(cmd))
            {
                done_ = true;
                return false;
            }
            continue; //Try again
        }

        /* At this point we know the following things:
           - segment_length > spacing_left
           - error is small enough
           - at least half a marker fits into this segment
        */

        //Check if marker really fits in this segment
        if (segment_length < size_.width())
        {
            //Segment to short => Skip this segment
            set_spacing_left(segment_length + size_.width()/2); //Only moves forward
            continue;
        } else if (segment_length - spacing_left_ < size_.width()/2)
        {
            //Segment is long enough, but we are to close to the end

            //Note: This function moves backwards. This could lead to an infinite
            // loop when another function adds a positive offset. Therefore we
            // only move backwards when there is no offset
            if (error_ == 0)
            {
                set_spacing_left(segment_length - size_.width()/2, true);
            } else
            {
                //Skip this segment
                set_spacing_left(segment_length + size_.width()/2); //Only moves forward
            }
            continue; //Force checking of max_error constraint
        }
        *angle = atan2(dy, dx);
        *x = last_x + dx * (spacing_left_ / segment_length);
        *y = last_y + dy * (spacing_left_ / segment_length);

        box2d<double> box = perform_transform(*angle, *x, *y);
        if (!allow_overlap_ && !detector_.has_placement(box))
        {
            //10.0 is the approxmiate number of positions tried and choosen arbitrarily
            set_spacing_left(spacing_left_ + spacing_ * max_error_ / 10.0); //Only moves forward
            continue;
        }
        if (add_to_detector) detector_.insert(box);
        last_x = *x;
        last_y = *y;
        return true;
    }
}


template <typename Locator, typename Detector>
box2d<double> markers_placement<Locator, Detector>::perform_transform(double angle, double dx, double dy)
{
    double c = cos(angle), s = sin(angle);
    double x1 = size_.minx();
    double x2 = size_.maxx();
    double y1 = size_.miny();
    double y2 = size_.maxy();

    double x1_ = dx + x1 * c - y1 * s;
    double y1_ = dy + x1 * s + y1 * c;
    double x2_ = dx + x2 * c - y2 * s;
    double y2_ = dy + x2 * s + y2 * c;

    return box2d<double>(x1_, y1_, x2_, y2_);
}

template <typename Locator, typename Detector>
void markers_placement<Locator, Detector>::set_spacing_left(double sl, bool allow_negative)
{
    double delta_error = sl - spacing_left_;
    if (!allow_negative && delta_error < 0)
    {
#ifdef MAPNIK_DEBUG
        std::cerr << "WARNING: Unexpected negative error in markers_placement. Please file a bug report.\n";
#endif
        return;
    }
#ifdef MAPNIK_DEBUG
    if (delta_error == 0.0)
    {
        std::cerr << "WARNING: Not moving at all in set_spacing_left()! Please file a bug report.\n";
    }
#endif
    error_ += delta_error;
    spacing_left_ = sl;
}

typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
typedef coord_transform2<CoordTransform,geometry_type> path_type;
typedef coord_transform2<CoordTransform,clipped_geometry_type> clipped_path_type;

template class markers_placement<path_type, label_collision_detector4>;
template class markers_placement<clipped_path_type, label_collision_detector4>;

} //ns mapnik
