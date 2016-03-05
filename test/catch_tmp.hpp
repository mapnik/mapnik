#ifndef TEST_CATCH_TMP_HPP
#define TEST_CATCH_TMP_HPP

#include <boost/filesystem/convenience.hpp>

struct catch_temporary_path
{
    using path = boost::filesystem::path;

    static path base_dir;
    static bool keep_temporary_files;

    static path absolute(path const& p)
    {
        return boost::filesystem::absolute(p, base_dir);
    }

    static catch_temporary_path create_dir(std::string const& p)
    {
        auto abs = absolute(p);
        bool created = boost::filesystem::create_directory(abs);
        return catch_temporary_path(abs, created);
    }

    catch_temporary_path()
        : remove_(false) {}

    catch_temporary_path(char const* p, bool r = true)
        : path_(absolute(p)), remove_(r) {}

    catch_temporary_path(std::string const& p, bool r = true)
        : path_(absolute(p)), remove_(r) {}

    catch_temporary_path(path const& p, bool r = true)
        : path_(absolute(p)), remove_(r) {}

    catch_temporary_path(catch_temporary_path && other)
        : remove_(other.remove_)
    {
        path_.swap(other.path_);
        other.remove_ = false;
    }

    ~catch_temporary_path()
    {
        if (remove_ && !path_.empty() && !keep_temporary_files)
        {
            boost::system::error_code ec; // ignored
            boost::filesystem::remove(path_, ec); // nothrow
        }
    }

    catch_temporary_path & operator=(catch_temporary_path && other)
    {
        path_.swap(other.path_);
        std::swap(remove_, other.remove_);
        return *this;
    }

    path const* operator->() const
    {
        return &path_;
    }

    operator path const& () const
    {
        return path_;
    }

    operator std::string () const
    {
        return path_.string();
    }

private: // noncopyable
    catch_temporary_path(catch_temporary_path const& ) = delete;
    catch_temporary_path & operator=(catch_temporary_path const& ) = delete;
    path path_;
    bool remove_;
};

#endif // TEST_CATCH_TMP_HPP
