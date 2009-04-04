if HAVE_BOOST_PROGRAM_OPTIONS
if HAVE_SQLITE3
if HAVE_POSTGRESQL

bin_PROGRAMS = \
	pgsql2sqlite

pgsql2sqlite_SOURCES = \
	main.cpp      \
	pgsql2sqlite.hpp      \
	sqlite.cpp      \
	sqlite.hpp

pgsql2sqlite_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	${POSTGRESQL_LDFLAGS} \
	${SQLITE3_LDFLAGS} \
        ../../src/libmapnik.la 

pgsql2sqlite_DEPENDENCIES = \
        ../../src/libmapnik.la

pgsql2sqlite_CXXFLAGS = \
	-Wall \
	${PROFILING_CFLAGS} \
	${TRACING_CFLAGS} \
	${SQLITE3_CFLAGS} \
	${POSTGRESQL_CFLAGS} \
	-I../../include \
	-I../../plugins/input/postgis

endif
endif
endif

## File created by the gnome-build tools

