UNAME := $(shell uname)
LINK_FIX=LD_LIBRARY_PATH
ifeq ($(UNAME), Darwin)
	LINK_FIX=DYLD_LIBRARY_PATH
else
endif

all: mapnik

install:
	@python scons/scons.py --config=cache --implicit-cache --max-drift=1 install

mapnik:
	@python scons/scons.py --config=cache --implicit-cache --max-drift=1

clean:
	@python scons/scons.py -c --config=cache --implicit-cache --max-drift=1
	@if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi

distclean:
	if test -e ".sconf_temp/"; then rm -r ".sconf_temp/"; fi
	if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi
	if test -e "config.cache"; then rm "config.cache"; fi
	if test -e "config.py"; then mv "config.py" "config.py.backup"; fi

reset: distclean

uninstall:
	python scons/scons.py --config=cache --implicit-cache --max-drift=1 uninstall

test:
	./run_tests

test-local:
	@echo "*** Boostrapping local test environment..."
	@export ${LINK_FIX}=`pwd`/src:${${LINK_FIX}} && \
	export PYTHONPATH=`pwd`/bindings/python/:${PYTHONPATH} && \
	export MAPNIK_FONT_DIRECTORY=`pwd`/fonts/dejavu-fonts-ttf-2.33/ttf/ && \
	export MAPNIK_INPUT_PLUGINS_DIRECTORY=`pwd`/plugins/input/ && \
	make test

check: test-local

demo:
	@echo "*** Running rundemo.cpp…"
	cd demo/c++; ./rundemo `mapnik-config --prefix`/lib/mapnik

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
