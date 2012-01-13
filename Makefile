all: mapnik

install:
	python scons/scons.py install

mapnik:
	python scons/scons.py

clean:
	python scons/scons.py -c

reset:
	if test -e ".sconf_temp/"; then rm -r ".sconf_temp/"; fi
	if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi
	if test -e "config.cache"; then rm "config.cache"; fi

uninstall:
	python scons/scons.py uninstall

test:
	@python tests/run_tests.py -q

.PHONY: clean reset uninstall test install
