
OS := $(shell uname -s)

ifeq ($(JOBS),)
	JOBS:=1
	ifeq ($(OS),Linux)
		JOBS:=$(shell grep -c ^processor /proc/cpuinfo)
	endif
	ifeq ($(OS),Darwin)
		JOBS:=$(shell sysctl -n hw.ncpu)
	endif
endif

all: mapnik

install:
	python2 scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 install

mapnik:
	python2 scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1

clean:
	@python2 scons/scons.py -j$(JOBS) -c --config=cache --implicit-cache --max-drift=1
	@if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi
	@if test -e "config.log"; then rm  "config.log"; fi
	@if test -e ".sconf_temp/"; then rm -r ".sconf_temp/"; fi
	@find ./ -name "*.pyc" -exec rm {} \;
	@find ./ -name "*.os" -exec rm {} \;
	@find ./ -name "*.dylib" -exec rm {} \;
	@find ./ -name "*.so" -exec rm {} \;
	@find ./ -name "*.o" -exec rm {} \;
	@find ./ -name "*.pyc" -exec rm {} \;
	@if test -e "bindings/python/mapnik/paths.py"; then rm "bindings/python/mapnik/paths.py"; fi

distclean:
	@if test -e "config.cache"; then rm "config.cache"; fi
	if test -e "config.py"; then mv "config.py" "config.py.backup"; fi

reset: distclean

rebuild:
	make uninstall && make clean && time make && make install

uninstall:
	@python2 scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 uninstall

test:
	./run_tests

test-local:
	make test

test-visual:
	bash -c "source ./localize.sh && python2 tests/visual_tests/test.py -q"

test-python:
	bash -c "source ./localize.sh && python2 tests/run_tests.py -q"

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
