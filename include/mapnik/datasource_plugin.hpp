#ifndef DATASOURCE_PLUGIN_HPP
#define DATASOURCE_PLUGIN_HPP
#include <string>
#include <mapnik/config.hpp>
#include <mapnik/datasource.hpp>

namespace mapnik {
class MAPNIK_DECL datasource_plugin
{
  public:
    datasource_plugin() = default;
    virtual ~datasource_plugin() = default;
    virtual void after_load() const = 0;
    virtual void before_unload() const = 0;
    virtual const char* name() const = 0;
    virtual datasource_ptr create(parameters const& params) const = 0;
};
} // namespace mapnik

#define DATASOURCE_PLUGIN_DEF(classname, pluginname)                                                                   \
    class classname : public mapnik::datasource_plugin                                                                 \
    {                                                                                                                  \
      public:                                                                                                          \
        static constexpr const char* kName = #pluginname;                                                              \
        void after_load() const override;                                                                              \
        void before_unload() const override;                                                                           \
        const char* name() const override;                                                                             \
        mapnik::datasource_ptr create(mapnik::parameters const& params) const override;                                \
    };

#define DATASOURCE_PLUGIN_IMPL(classname, pluginclassname)                                                             \
    const char* classname::name() const                                                                                \
    {                                                                                                                  \
        return kName;                                                                                                  \
    }                                                                                                                  \
    mapnik::datasource_ptr classname::create(mapnik::parameters const& params) const                                   \
    {                                                                                                                  \
        return std::make_shared<pluginclassname>(params);                                                              \
    }

#define DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(classname)                                                                  \
    void classname::after_load() const {}
#define DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(classname)                                                               \
    void classname::before_unload() const {}

#ifndef MAPNIK_STATIC_PLUGINS
#define DATASOURCE_PLUGIN_EXPORT(classname)                                                                            \
    extern "C" MAPNIK_EXP classname plugin;                                                                            \
    classname plugin;
#else
#define DATASOURCE_PLUGIN_EXPORT(classname) // export classname
#endif

#endif
