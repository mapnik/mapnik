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

uninstall:
	python scons/scons.py uninstall

test:
	
	echo "...running python tests"
	python tests/run_tests.py

.PHONY: clean reset uninstall test install