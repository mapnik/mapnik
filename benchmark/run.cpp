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
void benchmark(T test, std::string const& name)
{
    bool should_run_test = true;
    if (!test_set.empty()) {
        should_run_test = test_set.find(test_num) != test_set.end();
    }
    if (should_run_test || dry_run) {
        if (!test.validate()) {
            std::clog << "test did not validate: " << name << "\n";
            //throw std::runtime_error(std::string("test did not validate: ") + name);
        }
        if (dry_run) {
            std::clog << test_num << ") " << (test.threads_ ? "threaded -> ": "")
                << name << "\n";
        } else {
            process_cpu_clock::time_point start;
            dur elapsed;
            if (test.threads_ > 0) {
                boost::thread_group tg;
                for (unsigned i=0;i<test.threads_;++i)
                {
                    tg.create_thread(test);
                }
                start = process_cpu_clock::now();
                tg.join_all();
                elapsed = process_cpu_clock::now() - start;
            } else {
                start = process_cpu_clock::now();
                test();
                elapsed = process_cpu_clock::now() - start;
            }
            std::clog << test_num << ") " << (test.threads_ ? "threaded -> ": "")
                << name << ": "
                << boost::chrono::duration_cast<milliseconds>(elapsed) << "\n";
        }
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
        for (int i=-180;i<180;i=++i)
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
            test6 runner(1,5,
                         "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                         "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over",
                         from,to,false);
            benchmark(runner,"lonlat -> merc coord transformation with proj4 init (literal)");
        }

        {
            test6 runner(1,5,
                         "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                         "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over",
                         from,to,true);
            benchmark(runner,"lonlat -> merc coord transformation with lazy proj4 init (literal)");
        }

        /*{
            // echo -180 -60 | cs2cs -f "%.10f" +init=epsg:4326 +to +init=epsg:3857
            test6 runner(100000000,100,
                         "+init=epsg:4326",
                         "+init=epsg:3857",
                         from,to,false);
            benchmark(runner,"lonlat -> merc coord transformation (epsg)");
        }

        {
            test6 runner(100000000,100,
                         "+init=epsg:3857",
                         "+init=epsg:4326",
                         to,from,false);
            benchmark(runner,"merc -> lonlat coord transformation (epsg)");
        }*/

        /*{
            test6 runner(10,2,
                         "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over",
                         "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                         to,from,false);
            benchmark(runner,"merc -> lonlat coord transformation (literal)");
        }*/

        std::cout << "...benchmark done\n";
        return 0;
    }
    catch (std::exception const& ex)
    {
        std::clog << "test error: " << ex.what() << "\n";
        return -1;
    }
}
