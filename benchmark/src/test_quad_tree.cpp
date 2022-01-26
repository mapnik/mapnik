#include "bench_framework.hpp"
#include <mapnik/quad_tree.hpp>
#include <random>

using quad_tree_type = mapnik::quad_tree<std::size_t>;

class test : public benchmark::test_case
{
  public:
    test(mapnik::parameters const& params)
        : test_case(params)
    {}

    bool validate() const { return true; }

    bool operator()() const
    {
        std::random_device rd;
        std::default_random_engine engine(rd());
        std::uniform_int_distribution<int> uniform_dist(0, 2048);
        quad_tree_type tree(mapnik::box2d<double>(0, 0, 2048, 2048));
        // populate
        for (size_t i = 0; i < iterations_; ++i)
        {
            int cx = uniform_dist(engine);
            int cy = uniform_dist(engine);
            int sx = 0.2 * uniform_dist(engine);
            int sy = 0.2 * uniform_dist(engine);
            mapnik::box2d<double> box(cx - sx, cy - sy, cx + sx, cy + sy);
            tree.insert(i, box);
        }
        // bounding box query
        std::size_t count = 0;
        for (size_t i = 0; i < iterations_; ++i)
        {
            int cx = uniform_dist(engine);
            int cy = uniform_dist(engine);
            int sx = 0.4 * uniform_dist(engine);
            int sy = 0.4 * uniform_dist(engine);
            mapnik::box2d<double> box(cx - sx, cy - sy, cx + sx, cy + sy);
            auto itr = tree.query_in_box(box);
            auto end = tree.query_end();
            for (; itr != end; ++itr)
            {
                ++count;
            }
        }
        return true;
    }
};

BENCHMARK(test, "quad_tree creation")
