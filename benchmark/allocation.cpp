#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>


#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/process_cpu_clocks.hpp>
#include <boost/chrono.hpp>

using namespace boost::chrono;
using namespace mapnik;

void threaded_benchmark(void test(),std::string const& name, unsigned num_threads) {
  using namespace boost::chrono;
  typedef process_cpu_clock clock_type;
  process_real_cpu_clock::time_point start = process_real_cpu_clock::now();
  //boost::thread_group threads;
  typedef std::vector<std::unique_ptr<std::thread> > thread_group;
  typedef thread_group::value_type value_type;
  thread_group threads;
  // create threads
  for (unsigned i=0; i<num_threads; ++i)
  {
      threads.emplace_back(new std::thread(test));
  }
  // join all
  std::for_each(threads.begin(), threads.end(), [](value_type & t) {if (t->joinable()) t->join();});

  clock_type::duration elapsed = process_real_cpu_clock::now() - start;
  std::clog << boost::chrono::duration_cast<milliseconds>(elapsed)
            << " (" << boost::chrono::duration_cast<seconds>(elapsed) << ")"
            << " <-- " << name << "\n";
}


void test_wkt_creation()
{
    boost::ptr_vector<mapnik::geometry_type> paths;
    mapnik::wkt_parser parse_wkt;
    std::string value("GEOMETRYCOLLECTION(MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20))),POINT(2 3),LINESTRING(2 3,3 4),MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20))),MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20))),MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20))))");
    if (!parse_wkt.parse(value, paths)) std::clog << "failed to parse\n";
    int iterations = 10000;
    typedef process_cpu_clock clock_type;
    process_real_cpu_clock::time_point start = process_real_cpu_clock::now();
    for (int i=0;i<iterations;++i) {
        parse_wkt.parse(value, paths);
    }
    clock_type::duration elapsed = process_real_cpu_clock::now() - start;
    std::clog << "elapsed: " << boost::chrono::duration_cast<milliseconds>(elapsed) << "\n";
}

int main( int, char*[] )
{
    return 0;
}
