#ifndef MAPNIK_CSV_DATASOURCE_HPP
#define MAPNIK_CSV_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>

// stl
#include <vector>

class csv_datasource : public mapnik::datasource 
{
   public:
      csv_datasource(mapnik::parameters const& params, bool bind=true);
      virtual ~csv_datasource ();
      int type() const;
      static std::string name();
      mapnik::featureset_ptr features(mapnik::query const& q) const;
      mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
      mapnik::box2d<double> envelope() const;
      mapnik::layer_descriptor get_descriptor() const;
      void bind() const;
      template <typename T>
      void parse_csv(T& stream,
         std::string const& escape,
         std::string const& separator,
         std::string const& quote) const;
   private:
      mutable mapnik::layer_descriptor desc_;
      mutable mapnik::box2d<double> extent_;
      mutable std::string filename_;
      mutable std::string inline_string_;
      mutable unsigned file_length_;
      mutable int row_limit_;
      mutable std::vector<mapnik::feature_ptr> features_;
      mutable std::string escape_;
      mutable std::string separator_;
      mutable std::string quote_;
      mutable std::vector<std::string> headers_;
      mutable std::string manual_headers_;
      mutable bool strict_;
      mutable bool quiet_;
      mutable double filesize_max_;
};


#endif // MAPNIK_CSV_DATASOURCE_HPP
