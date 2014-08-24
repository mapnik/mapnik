#include <vector>
#include <algorithm>
#include <string>
#include <mapnik/util/fs.hpp>
#include <boost/filesystem/convenience.hpp>

inline static bool set_working_dir(std::vector<std::string> args)
{
    std::vector<std::string>::iterator itr = std::find(args.begin(), args.end(), "-d");
    if (itr!=args.end())
    {
        unsigned dist = std::distance(args.begin(),itr);
        if (args.size() > dist+1)
        {
            std::string chdir = args.at(dist+1);
            bool exists = mapnik::util::exists( chdir );
            if (exists)
            {
                boost::filesystem::current_path(chdir);
                return true;
            }
        }
        return false;
    }
    return true;
}
