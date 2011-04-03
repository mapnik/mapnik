all: mapnik

install: all
	python scons/scons.py install

mapnik:
	python scons/scons.py

clean:
	python scons/scons.py -c
	if test -e ".sconf_temp/"; then rm -r ".sconf_temp/"; fi
	if test -e ".sconsign.dblite"; then rm ".sconsign.dblite"; fi

check:
	cppcheck --enable=all -I include */*.cpp

uninstall:
	python scons/scons.py uninstall

test:
	
	echo "...running c++ tests"
	./tests/cpp_tests/font_registration_test
	
	echo "...running python tests"
	python tests/run_tests.py