#include <mapnik/graphics.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/util/conversions.hpp>


// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <set>

// boost
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/process_cpu_clocks.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

using namespace boost::chrono;
using namespace mapnik;

static unsigned test_num = 1;
static bool dry_run = false;
static std::set<int> test_set;

typedef process_cpu_clock clock_type;
typedef clock_type::duration dur;

template <typename T>
void benchmark(T & test_runner, std::string const& name)
{
    try {
        bool should_run_test = true;
        if (!test_set.empty())
        {
            should_run_test = test_set.find(test_num) != test_set.end();
        }
        if (should_run_test || dry_run)
        {
            if (!test_runner.validate())
            {
                std::clog << "test did not validate: " << name << "\n";
                //throw std::runtime_error(std::string("test did not validate: ") + name);
            }
            if (dry_run)
            {
                std::clog << test_num << ") " << (test_runner.threads_ ? "threaded -> ": "")
                    << name << "\n";
            }
            else
            {
                process_cpu_clock::time_point start;
                dur elapsed;
                if (test_runner.threads_ > 0)
                {
                    boost::thread_group tg;
                    for (unsigned i=0;i<test_runner.threads_;++i)
                    {
                        tg.create_thread(test_runner);
                        //tg.create_thread(boost::bind(&T::operator(),&test_runner));
                    }
                    start = process_cpu_clock::now();
                    tg.join_all();
                    elapsed = process_cpu_clock::now() - start;
                }
                else
                {
                    start = process_cpu_clock::now();
                    test_runner();
                    elapsed = process_cpu_clock::now() - start;
                }
                std::clog << test_num << ") " << (test_runner.threads_ ? "threaded -> ": "")
                    << name << ": "
                    << boost::chrono::duration_cast<milliseconds>(elapsed) << "\n";
            }
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "test runner did not complete: " << ex.what() << "\n";
    }
    test_num++;
}

bool compare_images(std::string const& src_fn,std::string const& dest_fn)
{
    std::auto_ptr<mapnik::image_reader> reader1(mapnik::get_image_reader(dest_fn,"png"));
    if (!reader1.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + dest_fn);
    }
    boost::shared_ptr<image_32> image_ptr1 = boost::make_shared<image_32>(reader1->width(),reader1->height());
    reader1->read(0,0,image_ptr1->data());

    std::auto_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(src_fn,"png"));
    if (!reader2.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + src_fn);
    }
    boost::shared_ptr<image_32> image_ptr2 = boost::make_shared<image_32>(reader2->width(),reader2->height());
    reader2->read(0,0,image_ptr2->data());

    image_data_32 const& dest = image_ptr1->data();
    image_data_32 const& src = image_ptr2->data();

    unsigned int width = src.width();
    unsigned int height = src.height();
    if ((width != dest.width()) || height != dest.height()) return false;
    for (unsigned int y = 0; y < height; ++y)
    {
        const unsigned int* row_from = src.getRow(y);
        const unsigned int* row_to = dest.getRow(y);
        for (unsigned int x = 0; x < width; ++x)
        {
           if (row_from[x] != row_to[x]) return false;
        }
    }
    return true;
}

struct test1
{
    unsigned iter_;
    unsigned threads_;
    explicit test1(unsigned iterations, unsigned threads=0) :
      iter_(iterations),
      threads_(threads)
      {}

    bool validate()
    {
        return true;
    }

    void operator()()
    {
        mapnik::image_data_32 im(256,256);
        std::string out;
        for (unsigned i=0;i<iter_;++i) {
            out.clear();
            out = mapnik::save_to_string(im,"png");
        }
    }
};

struct test2
{
    unsigned iter_;
    unsigned threads_;
    boost::shared_ptr<image_32> im_;
    explicit test2(unsigned iterations, unsigned threads=0) :
      iter_(iterations),
      threads_(threads),
      im_()
    {
        std::string filename("./benchmark/data/multicolor.png");
        std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename,"png"));
        if (!reader.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + filename);
        }
        im_ = boost::make_shared<image_32>(reader->width(),reader->height());
        reader->read(0,0,im_->data());
    }

    bool validate()
    {
        std::string expected("./benchmark/data/multicolor-hextree-expected.png");
        std::string actual("./benchmark/data/multicolor-hextree-actual.png");
        mapnik::save_to_file(im_->data(),actual, "png8:m=h");
        return compare_images(actual,expected);
    }

    void operator()()
    {
        std::string out;
        for (unsigned i=0;i<iter_;++i) {
            out.clear();
            out = mapnik::save_to_string(im_->data(),"png8:m=h");
        }
    }
};


struct test3
{
    unsigned iter_;
    unsigned threads_;
    double val_;
    explicit test3(unsigned iterations, unsigned threads=0) :
      iter_(iterations),
      threads_(threads),
      val_(-0.123) {}
    bool validate()
    {
        std::ostringstream s;
        s << val_;
        return (s.str() == "-0.123");
    }
    void operator()()
    {
        std::string out;
        for (unsigned i=0;i<iter_;++i) {
            std::ostringstream s;
            s << val_;
            out = s.str();
        }
    }
};

struct test4
{
    unsigned iter_;
    unsigned threads_;
    double val_;
    explicit test4(unsigned iterations, unsigned threads=0) :
      iter_(iterations),
      threads_(threads),
      val_(-0.123) {}

    bool validate()
    {
        std::string s;
        mapnik::util::to_string(s,val_);
        return (s == "-0.123");
    }
    void operator()()
    {
        std::string out;
        for (unsigned i=0;i<iter_;++i) {
            out.clear();
            mapnik::util::to_string(out,val_);
        }
    }
};


struct test5
{
    unsigned iter_;
    unsigned threads_;
    double val_;
    explicit test5(unsigned iterations, unsigned threads=0) :
      iter_(iterations),
      threads_(threads),
      val_(-0.123) {}

    bool validate()
    {
        std::string s;
        to_string_impl(s,val_);
        return (s == "-0.123");
    }
    bool to_string_impl(std::string &s , double val)
    {
        s.resize(s.capacity());
        while (true)
        {
            size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size()+1, "%g", val_));
            if (n2 <= s.size())
            {
                s.resize(n2);
                break;
            }
            s.resize(n2);
        }
        return true;
    }
    void operator()()
    {
        std::string out;
        for (unsigned i=0;i<iter_;++i)
        {
            out.clear();
            to_string_impl(out , val_);
        }
    }
};


#include <mapnik/box2d.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>

struct test6
{
    unsigned iter_;
    unsigned threads_;
    std::string src_;
    std::string dest_;
    mapnik::box2d<double> from_;
    mapnik::box2d<double> to_;
    bool defer_proj4_init_;
    explicit test6(unsigned iterations,
                   unsigned threads,
                   std::string const& src,
                   std::string const& dest,
                   mapnik::box2d<double> from,
                   mapnik::box2d<double> to,
                   bool defer_proj) :
      iter_(iterations),
      threads_(threads),
      src_(src),
      dest_(dest),
      from_(from),
      to_(to),
      defer_proj4_init_(defer_proj) {}

    bool validate()
    {
        mapnik::projection src(src_,defer_proj4_init_);
        mapnik::projection dest(dest_,defer_proj4_init_);
        mapnik::proj_transform tr(src,dest);
        mapnik::box2d<double> bbox = from_;
        if (!tr.forward(bbox)) return false;
        return ((std::fabs(bbox.minx() - to_.minx()) < .5) &&
                (std::fabs(bbox.maxx() - to_.maxx()) < .5) &&
                (std::fabs(bbox.miny() - to_.miny()) < .5) &&
                (std::fabs(bbox.maxy() - to_.maxy()) < .5)
               );
    }
    void operator()()
    {
        unsigned count=0;
        for (int i=-180;i<180;++i)
        {
            for (int j=-85;j<85;++j)
            {
                mapnik::projection src(src_,defer_proj4_init_);
                mapnik::projection dest(dest_,defer_proj4_init_);
                mapnik::proj_transform tr(src,dest);
                mapnik::box2d<double> box(i,j,i,j);
                if (!tr.forward(box)) throw std::runtime_error("could not transform coords");
                ++count;
            }
        }
    }
};

#include <mapnik/unicode.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>

struct test7
{
    unsigned iter_;
    unsigned threads_;
    std::string expr_;
    explicit test7(unsigned iterations,
                   unsigned threads,
                   std::string const& expr) :
      iter_(iterations),
      threads_(threads),
      expr_(expr)
      {}

    bool validate()
    {
        mapnik::expression_ptr expr = mapnik::parse_expression(expr_,"utf-8");
        return mapnik::to_expression_string(*expr) == expr_;
    }
    void operator()()
    {
         for (unsigned i=0;i<iter_;++i) {
             mapnik::expression_ptr expr = mapnik::parse_expression(expr_,"utf-8");
         }
    }
};

#include <mapnik/expression_grammar.hpp>

struct test8
{
    unsigned iter_;
    unsigned threads_;
    std::string expr_;
    explicit test8(unsigned iterations,
                   unsigned threads,
                   std::string const& expr) :
      iter_(iterations),
      threads_(threads),
      expr_(expr)
      {}

    bool validate()
    {
         transcoder tr("utf-8");
         mapnik::expression_grammar<std::string::const_iterator> expr_grammar(tr);
         mapnik::expression_ptr expr = mapnik::parse_expression(expr_,expr_grammar);
         return mapnik::to_expression_string(*expr) == expr_;
    }
    void operator()()
    {
         transcoder tr("utf-8");
         mapnik::expression_grammar<std::string::const_iterator> expr_grammar(tr);
         for (unsigned i=0;i<iter_;++i) {
             mapnik::expression_ptr expr = mapnik::parse_expression(expr_,expr_grammar);
         }
    }
};

#include <mapnik/rule_cache.hpp>

#if BOOST_VERSION >= 105300
#include <boost/container/vector.hpp>
#include <boost/move/utility.hpp>

class rule_cache_move
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(rule_cache_move)
public:
    typedef std::vector<rule const*> rule_ptrs;
    rule_cache_move()
     : if_rules_(),
       else_rules_(),
       also_rules_() {}

    rule_cache_move(BOOST_RV_REF(rule_cache_move) rhs) // move ctor
        :  if_rules_(boost::move(rhs.if_rules_)),
           else_rules_(boost::move(rhs.else_rules_)),
           also_rules_(boost::move(rhs.also_rules_))
    {}

    rule_cache_move& operator=(BOOST_RV_REF(rule_cache_move) rhs) // move assign
    {
        std::swap(if_rules_, rhs.if_rules_);
        std::swap(else_rules_,rhs.else_rules_);
        std::swap(also_rules_, rhs.also_rules_);
        return *this;
   }

    void add_rule(rule const& r)
    {
        if (r.has_else_filter())
        {
            else_rules_.push_back(&r);
        }
        else if (r.has_also_filter())
        {
            also_rules_.push_back(&r);
        }
        else
        {
            if_rules_.push_back(&r);
        }
    }

    rule_ptrs const& get_if_rules() const
    {
        return if_rules_;
    }

    rule_ptrs const& get_else_rules() const
    {
        return else_rules_;
    }

    rule_ptrs const& get_also_rules() const
    {
        return also_rules_;
    }

private:
    rule_ptrs if_rules_;
    rule_ptrs else_rules_;
    rule_ptrs also_rules_;
};

struct test9
{
    unsigned iter_;
    unsigned threads_;
    unsigned num_rules_;
    unsigned num_styles_;
    std::vector<rule> rules_;
    explicit test9(unsigned iterations,
                   unsigned threads,
                   unsigned num_rules,
                   unsigned num_styles) :
      iter_(iterations),
      threads_(threads),
      num_rules_(num_rules),
      num_styles_(num_styles),
      rules_() {
          mapnik::rule r("test");
          for (unsigned i=0;i<num_rules_;++i) {
              rules_.push_back(r);
          }
      }

    bool validate()
    {
        return true;
    }
    void operator()()
    {
         for (unsigned i=0;i<iter_;++i) {
             boost::container::vector<rule_cache_move> rule_caches;
             for (unsigned i=0;i<num_styles_;++i) {
                 rule_cache_move rc;
                 for (unsigned i=0;i<num_rules_;++i) {
                     rc.add_rule(rules_[i]);
                 }
                 rule_caches.push_back(boost::move(rc));
             }
         }
    }
};

#endif

class rule_cache_heap
{
public:
    typedef std::vector<rule const*> rule_ptrs;
    rule_cache_heap()
     : if_rules_(),
       else_rules_(),
       also_rules_() {}

    void add_rule(rule const& r)
    {
        if (r.has_else_filter())
        {
            else_rules_.push_back(&r);
        }
        else if (r.has_also_filter())
        {
            also_rules_.push_back(&r);
        }
        else
        {
            if_rules_.push_back(&r);
        }
    }

    rule_ptrs const& get_if_rules() const
    {
        return if_rules_;
    }

    rule_ptrs const& get_else_rules() const
    {
        return else_rules_;
    }

    rule_ptrs const& get_also_rules() const
    {
        return also_rules_;
    }

private:
    rule_ptrs if_rules_;
    rule_ptrs else_rules_;
    rule_ptrs also_rules_;
};

struct test10
{
    unsigned iter_;
    unsigned threads_;
    unsigned num_rules_;
    unsigned num_styles_;
    std::vector<rule> rules_;
    explicit test10(unsigned iterations,
                   unsigned threads,
                   unsigned num_rules,
                   unsigned num_styles) :
      iter_(iterations),
      threads_(threads),
      num_rules_(num_rules),
      num_styles_(num_styles),
      rules_() {
          mapnik::rule r("test");
          for (unsigned i=0;i<num_rules_;++i) {
              rules_.push_back(r);
          }
      }

    bool validate()
    {
        return true;
    }
    void operator()()
    {
         for (unsigned i=0;i<iter_;++i) {
             boost::ptr_vector<rule_cache_heap> rule_caches;
             for (unsigned i=0;i<num_styles_;++i) {
                 std::auto_ptr<rule_cache_heap> rc(new rule_cache_heap);
                 for (unsigned i=0;i<num_rules_;++i) {
                     rc->add_rule(rules_[i]);
                 }
                 rule_caches.push_back(rc);
             }
         }
    }
};


#include <mapnik/wkt/wkt_factory.hpp>
#include "agg_conv_clipper.h"
#include "agg_path_storage.h"
#include <mapnik/geometry.hpp>

struct test11
{
    unsigned iter_;
    unsigned threads_;
    std::string wkt_in_;
    mapnik::box2d<double> extent_;
    typedef agg::conv_clipper<mapnik::geometry_type, agg::path_storage> poly_clipper;
    test11(unsigned iterations,
           unsigned threads,
           std::string wkt_in,
           mapnik::box2d<double> const& extent)
        : iter_(iterations),
          threads_(threads),
          wkt_in_(wkt_in),
          extent_(extent) {

    }

    bool validate()
    {
        return true;
    }
    void operator()()
    {
        boost::ptr_vector<geometry_type> paths;
        if (!mapnik::from_wkt(wkt_in_, paths))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        agg::path_storage ps;
        ps.move_to(extent_.minx(), extent_.miny());
        ps.line_to(extent_.minx(), extent_.maxy());
        ps.line_to(extent_.maxx(), extent_.maxy());
        ps.line_to(extent_.maxx(), extent_.miny());
        ps.close_polygon();
        for (unsigned i=0;i<iter_;++i) {
            BOOST_FOREACH( geometry_type & geom, paths)
            {
                poly_clipper clipped(geom,ps,
                    agg::clipper_and,
                    agg::clipper_non_zero,
                    agg::clipper_non_zero,
                    1);
                clipped.rewind(0);
                unsigned cmd;
                double x,y;
                while ((cmd = geom.vertex(&x, &y)) != SEG_END) {}
            }
        }
    }
};

#include <mapnik/polygon_clipper.hpp>

struct test12
{
    unsigned iter_;
    unsigned threads_;
    std::string wkt_in_;

    mapnik::box2d<double> extent_;
    typedef mapnik::polygon_clipper<mapnik::geometry_type> poly_clipper;
    test12(unsigned iterations,
           unsigned threads,
           std::string wkt_in,
           mapnik::box2d<double> const& extent)
        : iter_(iterations),
          threads_(threads),
          wkt_in_(wkt_in),
          extent_(extent)
    {
    }

    bool validate()
    {
        return true;
    }
    void operator()()
    {
        boost::ptr_vector<geometry_type> paths;
        if (!mapnik::from_wkt(wkt_in_, paths))
        {
            throw std::runtime_error("Failed to parse WKT");
        }
        for (unsigned i=0;i<iter_;++i)
        {
            BOOST_FOREACH( geometry_type & geom, paths)
            {
                poly_clipper clipped(extent_, geom);
                unsigned cmd;
                double x,y;
                while ((cmd = geom.vertex(&x, &y)) != SEG_END) {}
            }
        }
    }
};

#include <mapnik/font_engine_freetype.hpp>
#include <boost/format.hpp>
struct test13
{
    unsigned iter_;
    unsigned threads_;

    test13(unsigned iterations,
           unsigned threads)
        : iter_(iterations),
          threads_(threads)
    {}

    bool validate()
    {
        return true;
    }

    void operator()()
    {
        mapnik::freetype_engine engine;
        unsigned long count = 0;
        for (unsigned i=0;i<iter_;++i)
        {
            BOOST_FOREACH( std::string const& name, mapnik::freetype_engine::face_names())
            {
                mapnik::face_ptr f = engine.create_face(name);
                if (f) ++count;
            }
        }
    }
};

int main( int argc, char** argv)
{
    if (argc > 0) {
        for (int i=0;i<argc;++i) {
            std::string opt(argv[i]);
            if (opt == "-d" || opt == "--dry-run") {
                dry_run = true;
            } else if (opt[0] != '-') {
                int arg;
                if (mapnik::util::string2int(opt,arg)) {
                    test_set.insert(arg);
                }
            }
        }
    }
    try
    {
        std::cout << "starting benchmark…\n";

        {
            test1 runner(100);
            benchmark(runner,"encoding blank image as png");
        }

        {
            test2 runner(100);
            benchmark(runner,"encoding multicolor image as png8:m=h");
        }

        {
            test1 runner(10,10);
            benchmark(runner,"encoding blank image as png");
        }

        {
            test2 runner(10,10);
            benchmark(runner,"encoding multicolor image as png8:m=h");
        }

        {
            test3 runner(1000000);
            benchmark(runner,"double to string conversion with std::ostringstream");
        }

        {
            test4 runner(1000000);
            benchmark(runner,"double to string conversion with mapnik::util_to_string");
        }

        {
            test5 runner(1000000);
            benchmark(runner,"double to string conversion with snprintf");
        }

        {
            test3 runner(1000000,10);
            benchmark(runner,"double to string conversion with std::ostringstream");
        }

        {
            test4 runner(1000000,10);
            benchmark(runner,"double to string conversion with mapnik::util_to_string");
        }

        {
            test5 runner(1000000,10);
            benchmark(runner,"double to string conversion with snprintf");
        }

        mapnik::box2d<double> from(-180,-80,180,80);
        mapnik::box2d<double> to(-20037508.3427892476,-15538711.0963092316,20037508.3427892476,15538711.0963092316);

        {
            // echo -180 -60 | cs2cs -f "%.10f" +init=epsg:4326 +to +init=epsg:3857
            test6 runner(100000000,100,
                         "+init=epsg:4326",
                         "+init=epsg:3857",
                         from,to,true);
            benchmark(runner,"lonlat -> merc coord transformation (epsg)");
        }

        {
            test6 runner(100000000,100,
                         "+init=epsg:3857",
                         "+init=epsg:4326",
                         to,from,true);
            benchmark(runner,"merc -> lonlat coord transformation (epsg)");
        }

        {
            test6 runner(100000000,100,
                         "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                         "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over",
                         from,to,true);
            benchmark(runner,"lonlat -> merc coord transformation (literal)");
        }

        {
            test6 runner(100000000,100,
                         "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over",
                         "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                         to,from,true);
            benchmark(runner,"merc -> lonlat coord transformation (literal)");
        }

        {
            test7 runner(10000,100,"([foo]=1)");
            benchmark(runner,"expression parsing with grammer per parse");
        }

        {
            test8 runner(10000,100,"([foo]=1)");
            benchmark(runner,"expression parsing by re-using grammar");
        }

        {
#if BOOST_VERSION >= 105300
            test9 runner(1000,10,200,50);
            benchmark(runner,"rule caching using boost::move");
#else
            std::clog << "not running: 'rule caching using boost::move'\n";
#endif
        }

        {
            test10 runner(1000,10,200,50);
            benchmark(runner,"rule caching using heap allocation");
        }

        {
            std::string filename_("benchmark/data/polygon.wkt");
            std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
            if (!in.is_open())
                throw std::runtime_error("could not open: '" + filename_ + "'");
            std::string wkt_in( (std::istreambuf_iterator<char>(in) ),
                       (std::istreambuf_iterator<char>()) );
            mapnik::box2d<double> clipping_box(0,0,40,40);

            test11 runner(100000,10,wkt_in,clipping_box);
            benchmark(runner,"clipping polygon with agg_conv_clipper");
        }

        {

            std::string filename_("benchmark/data/polygon.wkt");
            std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
            if (!in.is_open())
                throw std::runtime_error("could not open: '" + filename_ + "'");
            std::string wkt_in( (std::istreambuf_iterator<char>(in) ),
                       (std::istreambuf_iterator<char>()) );
            mapnik::box2d<double> clipping_box(0,0,40,40);

            test12 runner(100000,10,wkt_in,clipping_box);
            benchmark(runner,"clipping polygon with mapnik::polygon_clipper");
        }

        {
            mapnik::freetype_engine::register_fonts("./fonts", true);
            unsigned face_count = mapnik::freetype_engine::face_names().size();
            test13 runner(1000,10);
            benchmark(runner, (boost::format("font_engihe: created %ld faces in ") % (face_count * 1000 * 10)).str());
        }
        std::cout << "...benchmark done\n";
        return 0;
    }
    catch (std::exception const& ex)
    {
        std::clog << "test error: " << ex.what() << "\n";
        return -1;
    }
}
