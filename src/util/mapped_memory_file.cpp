#include <mapnik/mapped_memory_cache.hpp>
#include <mapnik/util/mapped_memory_file.hpp>
#include <mapnik/util/fs.hpp>
#ifdef _WIN32
#include <mapnik/util/utf_conv_win.hpp>
#endif

namespace mapnik {
namespace util {
mapped_memory_file::mapped_memory_file() {}

mapped_memory_file::mapped_memory_file(std::string const& file_name)
    : file_name_{file_name},
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
      file_()
#elif defined(_WIN32)
      file_(mapnik::utf8_to_utf16(file_name), std::ios::in | std::ios::binary)
#else
      file_(file_name.c_str(), std::ios::in | std::ios::binary)
#endif
{
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    auto const memory = mapnik::mapped_memory_cache::instance().find(file_name, true);

    if (memory.has_value())
    {
        mapped_region_ = *memory;
        file_.buffer(static_cast<char*>(mapped_region_->get_address()), mapped_region_->get_size());
    }
    else
    {
        throw std::runtime_error("could not create file mapping for " + file_name);
    }
#endif
}

mapped_memory_file::file_source_type& mapped_memory_file::file()
{
    return file_;
}

bool mapped_memory_file::is_open() const
{
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    return (file_.buffer().second > 0);
#else
    return file_.is_open();
#endif
}

void mapped_memory_file::skip(std::streampos bytes)
{
    file_.seekg(bytes, std::ios::cur);
}

void mapped_memory_file::deleteFile(std::string const& file_name)
{
#ifdef MAPNIK_MEMORY_MAPPED_FILE
    mapped_memory_cache::instance().remove(file_name);
#endif
    if (util::exists(file_name))
    {
        util::remove(file_name);
    }
}

mapped_memory_file::~mapped_memory_file() {}

} // namespace util
} // namespace mapnik
