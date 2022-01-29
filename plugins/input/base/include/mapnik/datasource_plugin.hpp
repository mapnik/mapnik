#ifndef DATASOURCE_PLUGIN_HPP
#define DATASOURCE_PLUGIN_HPP
#include <string>
#include <mapnik/datasource.hpp>
#include <mapnik/config.hpp>


namespace mapnik
{
    class MAPNIK_DECL datasource_plugin
    {
    public:
        datasource_plugin() {}
        virtual ~datasource_plugin() {}
        virtual void init_once() const = 0;
        virtual const std::string& name() const = 0;
        virtual datasource_ptr create(parameters const &params) const= 0;
        
    };
}

#define DATASOURCE_PLUGIN_DEF(classname, pluginname) class classname : public mapnik::datasource_plugin {\
public:\
    static constexpr const char* kName = #pluginname;\
    void init_once() const override;\
    const std::string& name() const override;\
    mapnik::datasource_ptr create(mapnik::parameters const &params) const override;\
};

#define DATASOURCE_PLUGIN_IMPL(classname, pluginclassname) const std::string& classname::name() const { return kName; } \
mapnik::datasource_ptr classname::create(mapnik::parameters const &params) const { return std::make_shared<pluginclassname>(params); }

#define DATASOURCE_PLUGIN_EMPTY_INIT(classname) void classname::init_once() const {}

#ifndef MAPNIK_STATIC_PLUGINS
#define DATASOURCE_PLUGIN_EXPORT(classname) extern "C" MAPNIK_EXP classname plugin; \
classname plugin;
#else
#define DATASOURCE_PLUGIN_EXPORT(classname) // export classname
#endif

#endif
