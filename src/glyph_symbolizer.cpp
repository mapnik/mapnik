#include <mapnik/glyph_symbolizer.hpp>
#include <mapnik/value.hpp>

#include <boost/lexical_cast.hpp>


namespace mapnik
{
    text_path_ptr glyph_symbolizer::get_text_path(face_set_ptr const& faces,
                                                  Feature const& feature) const
    {
        text_path_ptr path_ptr = text_path_ptr(new text_path());
        return path_ptr;
    }
}
