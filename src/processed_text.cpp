#include <mapnik/processed_text.hpp>
namespace mapnik
{

void processed_text::push_back(char_properties const& properties, UnicodeString const& text)
{
    expr_list_.push_back(processed_expression(properties, text));
}

processed_text::expression_list::const_iterator processed_text::begin() const
{
    return expr_list_.begin();
}

processed_text::expression_list::const_iterator processed_text::end() const
{
    return expr_list_.end();
}

processed_text::processed_text(face_manager<freetype_engine> & font_manager, double scale_factor)
    : font_manager_(font_manager), scale_factor_(scale_factor)
{

}

void processed_text::clear()
{
    info_.clear();
    expr_list_.clear();
}


string_info &processed_text::get_string_info()
{
    info_.clear(); //if this function is called twice invalid results are returned, so clear string_info first
    expression_list::iterator itr = expr_list_.begin();
    expression_list::iterator end = expr_list_.end();
    for (; itr != end; ++itr)
    {
        char_properties const &p = itr->p;
        face_set_ptr faces = font_manager_.get_face_set(p.face_name, p.fontset);
        if (faces->size() <= 0)
        {
            throw config_error("Unable to find specified font face '" + p.face_name + "'");
        }
        faces->set_character_sizes(p.text_size * scale_factor_);
        faces->get_string_info(info_, itr->str, &(itr->p));
        info_.add_text(itr->str);
    }
    return info_;
}

} //ns mapnik
