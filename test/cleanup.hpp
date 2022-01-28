#ifndef TEST_MEMORY_CLEANUP
#define TEST_MEMORY_CLEANUP

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>

#if defined(HAVE_LIBXML2)
#include <libxml/parser.h>
#include <libxml/entities.h>
#include <libxml/globals.h>
#endif

#if defined(HAVE_CAIRO)
#include <cairo.h>
#endif

#include <unicode/uclean.h>

MAPNIK_DISABLE_WARNING_POP

namespace testing {

inline void run_cleanup()
{
    // only call this once, on exit
    // to make sure valgrind output is clean
    // http://xmlsoft.org/xmlmem.html
#if defined(HAVE_LIBXML2)
    xmlCleanupCharEncodingHandlers();
    xmlCleanupEncodingAliases();
    xmlCleanupGlobals();
    xmlCleanupParser();
    xmlCleanupThreads();
    xmlCleanupInputCallbacks();
    xmlCleanupOutputCallbacks();
    xmlCleanupMemory();
#endif

#if defined(HAVE_CAIRO)
    // http://cairographics.org/manual/cairo-Error-handling.html#cairo-debug-reset-static-data
    cairo_debug_reset_static_data();
#endif

    // http://icu-project.org/apiref/icu4c/uclean_8h.html#a93f27d0ddc7c196a1da864763f2d8920
    u_cleanup();
}

} // namespace testing

#endif
