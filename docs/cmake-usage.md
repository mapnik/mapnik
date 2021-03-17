# Usage with CMake
## Build
First clone mapnik from github and initialize submodules

```bash
git clone https://github.com/mapnik/mapnik.git
cd mapnik
git submodule update --init
```

Make sure that all dependencies are installed.

All available cmake options are listed at the top of [CMakeLists.txt](../CMakeLists.txt). 
Pass your options while configuring e.g.: `cmake -DBUILD_DEMO_VIEWER=OFF ..` to disable the build of the demo viewer application.

To quickstart open a console in the root mapnik dir and execute the following commands: (Pass all options and dependency dirs after `-DBUILD_TEST=OFF`)
```
> cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST=OFF
> cmake --build build --target install
```

## Usage

To use Mapnik in your project add the following lines to your CMakeLists.tzt.
```
find_package(mapnik CONFIG REQUIRED)
[...]
target_link_libraries(mytarget ... mapnik::headers mapnik::mapnik)
```

All mapnik executables and targets are exported within `MapnikTargets.cmake`. 

The plugin dir is available in the variable `MAPNIK_PLUGINS_DIR`. 
The font path is is available in the variable `MAPNIK_FONTS_DIR`. 
