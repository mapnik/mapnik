#include "bench_framework.hpp"

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/offset_converter.hpp>

struct fake_path
{
    using coord_type = std::tuple<double, double, unsigned>;
    using cont_type = std::vector<coord_type>;
    cont_type vertices_;
    cont_type::iterator itr_;

    fake_path(std::initializer_list<double> l)
        : fake_path(l.begin(), l.size()) {
    }

    fake_path(std::vector<double> const &v)
        : fake_path(v.begin(), v.size()) {
    }

    template <typename Itr>
    fake_path(Itr itr, size_t sz) {
        size_t num_coords = sz >> 1;
        vertices_.reserve(num_coords);

        for (size_t i = 0; i < num_coords; ++i) {
            double x = *itr++;
            double y = *itr++;
            unsigned cmd = (i == 0) ? mapnik::SEG_MOVETO : mapnik::SEG_LINETO;
            vertices_.push_back(std::make_tuple(x, y, cmd));
            if (i == num_coords - 1) cmd = mapnik::SEG_END;
            vertices_.push_back(std::make_tuple(x, y, cmd));
        }
        itr_ = vertices_.begin();
    }

    unsigned vertex(double *x, double *y) {
        if (itr_ == vertices_.end()) {
            return mapnik::SEG_END;
        }
        *x = std::get<0>(*itr_);
        *y = std::get<1>(*itr_);
        unsigned cmd = std::get<2>(*itr_);
        ++itr_;
        return cmd;
    }

    void rewind(unsigned) {
        itr_ = vertices_.begin();
    }
};

class test_offset : public benchmark::test_case
{
public:
    test_offset(mapnik::parameters const& params)
     : test_case(params) {}
    bool validate() const
    {
        return true;
    }
    bool operator()() const
    {
        std::vector<double> path;
        int mysize = 2500;
        int x1 = 0;
        path.reserve(mysize*2);
        for( int i = 0; i < mysize; i++ )
        {
            path.push_back( i );
            path.push_back( 0 );
        }
        fake_path fpath(path);
        for (std::size_t i=0;i<iterations_;++i) {
            mapnik::offset_converter<fake_path> off_path(fpath);
            off_path.set_offset(10);
            unsigned cmd;
            double x, y;
            while ((cmd = off_path.vertex(&x, &y)) != mapnik::SEG_END)
            {
                x1++;
            }
        }
        return x1 > 0;
    }
};


int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    {
        test_offset test_runner(params);
        run(test_runner,"offset_test");
    }
    return 0;
}
