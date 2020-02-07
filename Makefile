OS := $(shell uname -s)

ifeq ($(JOBS),)
	JOBS:=1
endif

ifeq ($(HEAVY_JOBS),)
	HEAVY_JOBS:=1
endif

all: mapnik

install:
	$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 install

release:
	./scripts/publish_release.sh

test-release:
	./scripts/test_release.sh

src/json/libmapnik-json.a:
	# we first build memory intensive files with -j$(HEAVY_JOBS)
	$(PYTHON) scons/scons.py -j$(HEAVY_JOBS) \
		--config=cache --implicit-cache --max-drift=1 \
		src/renderer_common/render_group_symbolizer.os \
		src/renderer_common/render_markers_symbolizer.os \
		src/renderer_common/render_thunk_extractor.os \
		src/json/libmapnik-json.a \
		src/wkt/libmapnik-wkt.a \
		src/css/css_grammar_x3.os \
		src/css/css_color_grammar_x3.os \
		src/expression_grammar_x3.os \
		src/transform_expression_grammar_x3.os \
		src/image_filter_grammar_x3.os \
		src/marker_helpers.os \
		src/svg/svg_transform_parser.os \
		src/agg/process_line_symbolizer.os \
		plugins/input/geojson/geojson_datasource.os \
		src/svg/svg_path_parser.os \
		src/svg/svg_parser.os \
		src/svg/svg_points_parser.os \
		src/svg/svg_transform_parser.os \


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
	@find ./ -name "*.gcda" -exec rm {} \;
	@find ./ -name "*.gcno" -exec rm {} \;

distclean:
	if test -e "config.py"; then mv "config.py" "config.py.backup"; fi

reset: distclean

rebuild:
	make uninstall && make clean && time make && make install

uninstall:
	@$(PYTHON) scons/scons.py -j$(JOBS) --config=cache --implicit-cache --max-drift=1 uninstall

test/data-visual:
	./scripts/ensure_test_data.sh

test/data:
	./scripts/ensure_test_data.sh

test: ./test/data test/data-visual
	@./test/run

check: test

bench:
	./benchmark/run

demo:
	cd demo/c++; ./rundemo `mapnik-config --prefix`

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
