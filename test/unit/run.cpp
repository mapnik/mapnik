#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <libxml/parser.h> // for xmlInitParser(), xmlCleanupParser()

int main (int argc, char* const argv[])
{
    int result = Catch::Session().run( argc, argv );
    if (!result) printf("\x1b[1;32m ✓ \x1b[0m\n");

    // only call this once, on exit
    // to make sure valgrind output is clean
    // http://xmlsoft.org/xmlmem.html
    xmlCleanupParser();

    return result;
}
