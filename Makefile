UNAME := $(shell uname)
LINK_FIX=LD_LIBRARY_PATH
ifeq ($(UNAME), Darwin)
	LINK_FIX=DYLD_LIBRARY_PATH
else
endif

OS:=$(shell uname -s)

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

./deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

g: config.gypi mapnik.gyp ./deps/gyp
	deps/run_gyp mapnik.gyp --depth=. -Goutput_dir=.. --generator-output=./build/ -f make --no-duplicate-basename-check
	#export PATH=/Users/dane/projects/mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64/bin/:$$PATH && make -C build V=$(V) mapnik -j2
	export PATH=/Users/dane/projects/mapnik-packaging/osx/out/build-cpp03-libstdcpp-x86_64-macosx/bin/:$$PATH && make -C build V=1 -j2
	make test

install:
	python scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 install

mapnik:
	python scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1

clean:
	@python scons/scons.py -j$(JOBS) -c --config=cache --implicit-cache --max-drift=1
	@if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi
	@if test -e "config.log"; then rm  "config.log"; fi
	@if test -e ".sconf_temp/"; then rm -r ".sconf_temp/"; fi
	@find ./ -name "*.pyc" -exec rm {} \;
	@find ./ -name "*.os" -exec rm {} \;
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
	@python scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 uninstall

test:
	@ ./run_tests

test-local:
	@echo "*** Boostrapping local test environment..."
	@export ${LINK_FIX}=`pwd`/src:${${LINK_FIX}} && \
	export PATH=`pwd`/utils/mapnik-config/:${PATH} && \
	export PYTHONPATH=`pwd`/bindings/python/:${PYTHONPATH} && \
	export MAPNIK_FONT_DIRECTORY=`pwd`/fonts/dejavu-fonts-ttf-2.33/ttf/ && \
	export MAPNIK_INPUT_PLUGINS_DIRECTORY=`pwd`/plugins/input/ && \
	make test

bench:
	@export ${LINK_FIX}=`pwd`/src:${${LINK_FIX}} && \
	./benchmark/run

check: test-local

demo:
	@echo "*** Running rundemo.cpp…"
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
