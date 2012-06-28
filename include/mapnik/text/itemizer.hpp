#ifndef MAPNIK_TEXT_ITEMIZER_HPP
#define MAPNIK_TEXT_ITEMIZER_HPP

//mapnik
#include <mapnik/text_properties.hpp> //TODO: Move to text/properties.hpp

// stl
#include <string>
#include <list>

// ICU
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>
namespace mapnik
{

struct text_item
{
    UnicodeString str;
    UScriptCode script;
    char_properties format;
    UBiDiDirection rtl;
    text_item(UnicodeString const& str) :
        str(str), script(), format(), rtl(UBIDI_LTR)
    {

    }
};

/** This class splits text into parts which all have the same
 * - direction (LTR, RTL)
 * - format
 * - script (http://en.wikipedia.org/wiki/Scripts_in_Unicode)
 **/
class text_itemizer
{
public:
    text_itemizer();
    void add_text(UnicodeString str, char_properties const& format);
    std::list<text_item> const& itemize();
    void clear();
    UnicodeString const& get_text() { return text; }
private:
    template<typename T> struct run
    {
        run(T data, unsigned limit) :  limit(limit), data(data){}
        unsigned limit;
        T data;
    };
    typedef run<char_properties> format_run_t;
    typedef run<UBiDiDirection> direction_run_t;
    typedef run<UScriptCode> script_run_t;
    UnicodeString text;
    std::list<format_run_t> format_runs;
    std::list<direction_run_t> direction_runs;
    std::list<script_run_t> script_runs;
    void itemize_direction();
    void itemize_script();
    void create_item_list();
    std::list<text_item> output;
};
} //ns mapnik

#endif // TEXT_ITEMIZER_HPP
