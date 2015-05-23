
OS := $(shell uname -s)

PYTHON = python

ifeq ($(JOBS),)
	JOBS:=1
endif

all: mapnik

install:
	$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 install

python:
	if [[ ! -d ./bindings/python ]]; then git clone git@github.com:mapnik/python-mapnik.git --recursive ./bindings/python; else (cd bindings/python && git pull && git submodule update --init); fi;
	make
	python bindings/python/test/visual.py -q

src/json/libmapnik-json.a:
	# we first build memory intensive files with -j1
	$(PYTHON) scons/scons.py -j1 \
		--config=cache --implicit-cache --max-drift=1 \
		src/renderer_common/process_group_symbolizer.os \
		src/json/libmapnik-json.a \
		src/wkt/libmapnik-wkt.a \
		src/css_color_grammar.os \
		src/expression_grammar.os \
		src/transform_expression_grammar.os \
		src/image_filter_types.os \
		src/agg/process_markers_symbolizer.os \
		src/agg/process_group_symbolizer.os \
		src/grid/process_markers_symbolizer.os \
		src/grid/process_group_symbolizer.os \
		src/cairo/process_markers_symbolizer.os \
		src/cairo/process_group_symbolizer.os \

mapnik: src/json/libmapnik-json.a
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
	@find ./src/ -name "*.dylib" -exec rm {} \;
	@find ./src/ -name "*.so" -exec rm {} \;
	@find ./ -name "*.o" -exec rm {} \;
	@find ./src/ -name "*.a" -exec rm {} \;

distclean:
	if test -e "config.py"; then mv "config.py" "config.py.backup"; fi

reset: distclean

rebuild:
	make uninstall && make clean && time make && make install

uninstall:
	@$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 uninstall

test/data:
	git submodule update --init

test: ./test/data
	@./test/run

check: test

bench:
	./benchmark/run

demo:
	cd demo/c++; ./rundemo `mapnik-config --prefix`

pep8:
	# https://gist.github.com/1903033
	# gsed on osx
	@pep8 -r --select=W293 -q --filename=*.py `pwd`/tests/ | xargs gsed -i 's/^[ \r\t]*$$//'
	@pep8 -r --select=W391 -q --filename=*.py `pwd`/tests/ | xargs gsed -i -e :a -e '/^\n*$$/{$$d;N;ba' -e '}'
	@pep8 -r --select=W391 -q --filename=*.py `pwd`/tests/ | xargs ged -i '/./,/^$$/!d'

# note: pass --gen-suppressions=yes to create new suppression entries
grind:
	@source localize.sh && \
	    valgrind --suppressions=./test/unit/valgrind.supp --leak-check=full --log-fd=1 ./test/visual/run | grep definitely;
	@source localize.sh && \
	for FILE in test/standalone/*-bin; do \
		valgrind --suppressions=./test/unit/valgrind.supp --leak-check=full --log-fd=1 $${FILE} | grep definitely; \
	done
	@source localize.sh && \
	    valgrind --suppressions=./test/unit/valgrind.supp --leak-check=full --log-fd=1 ./test/unit/run | grep definitely;

render:
	@for FILE in tests/data/good_maps/*xml; do \
		nik2img.py $${FILE} /tmp/$$(basename $${FILE}).png; \
	done

.PHONY: install mapnik clean distclean reset uninstall test demo pep8 grind render
