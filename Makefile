
OS := $(shell uname -s)

PYTHON = python

ifeq ($(JOBS),)
	JOBS:=1
endif

all: mapnik

install:
	$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 install

mapnik:
	# we first build memory intensive files with -j2
	$(PYTHON) scons/scons.py -j2 \
		--config=cache --implicit-cache --max-drift=1 \
		src/json/libmapnik-json.a \
		src/wkt/libmapnik-wkt.a \
		src/css_color_grammar.os \
		src/expression_grammar.os \
		src/transform_expression_grammar.os \
		src/image_filter_types.os \
		src/renderer_common/process_group_symbolizer.os \
		src/agg/process_markers_symbolizer.os \
		src/agg/process_group_symbolizer.os \
		src/grid/process_markers_symbolizer.os \
		src/grid/process_group_symbolizer.os \
		src/cairo/process_markers_symbolizer.os \
		src/cairo/process_group_symbolizer.os \
	# then install the rest with -j$(JOBS)
	$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1

clean:
	@$(PYTHON) scons/scons.py -j$(JOBS) -c --config=cache --implicit-cache --max-drift=1
	@if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi
	@if test -e "config.log"; then rm "config.log"; fi
	@if test -e "config.cache"; then rm "config.cache"; fi
	@if test -e ".sconf_temp/"; then rm -r ".sconf_temp/"; fi
	@find ./ -name "*.pyc" -exec rm {} \;
	@find ./ -name "*.os" -exec rm {} \;
	@find ./ -name "*.dylib" -exec rm {} \;
	@find ./ -name "*.so" -exec rm {} \;
	@find ./ -name "*.o" -exec rm {} \;
	@find ./ -name "*.a" -exec rm {} \;
	@find ./ -name "*.pyc" -exec rm {} \;
	@if test -e "bindings/python/mapnik/paths.py"; then rm "bindings/python/mapnik/paths.py"; fi

distclean:
	if test -e "config.py"; then mv "config.py" "config.py.backup"; fi

reset: distclean

rebuild:
	make uninstall && make clean && time make && make install

uninstall:
	@$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 uninstall

test:
	./run_tests

test-local:
	make test

test-visual:
	bash -c "source ./localize.sh && python tests/visual_tests/test.py -q"

test-python:
	bash -c "source ./localize.sh && python tests/run_tests.py -q"

test-cpp:
	./tests/cpp_tests/run

check: test-local

bench:
	./benchmark/run

demo:
	cd demo/c++; ./rundemo `mapnik-config --prefix`

pep8:
	# https://gist.github.com/1903033
	# gsed on osx
	@pep8 -r --select=W293 -q --filename=*.py `pwd`/tests/ | xargs gsed -i 's/^[ \r\t]*$//'
	@pep8 -r --select=W391 -q --filename=*.py `pwd`/tests/ | xargs gsed -i -e :a -e '/^\n*$/{$d;N;ba' -e '}'

grind:
	@for FILE in tests/cpp_tests/*-bin; do \
		valgrind --leak-check=full --log-fd=1 $${FILE} | grep definitely; \
	done

render:
	@for FILE in tests/data/good_maps/*xml; do \
		nik2img.py $${FILE} /tmp/$$(basename $${FILE}).png; \
	done

.PHONY: install mapnik clean distclean reset uninstall test demo pep8 grind render
