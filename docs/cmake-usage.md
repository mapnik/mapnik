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

To quickstart open a console in the root mapnik dir and execute the following commands: (Pass all options and dependency dirs after `-DBUILD_TESTING=OFF`)
```
> cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
> cmake --build build --target install
```

## Usage

To use Mapnik in your project add the following lines to your CMakeLists.tzt.
```
find_package(mapnik CONFIG REQUIRED)
[...]
target_link_libraries(mytarget ... mapnik::mapnik)
```
All targets: 
* `mapnik::core`: All compile definitions and headers.
* `mapnik::mapnik`: libmapnik. Has a public dependency on mapnik::core
* `mapnik::json`: json support for libmapnik.
* `mapnik::wkt`: wkt support for libmapnik.

All mapnik executables and targets are exported within `mapnikTargets.cmake`. 
The font path is is available in the variable `MAPNIK_FONTS_DIR`. 

The install location of the plugins might be configuration dependent. 
For each configuration there exists a variable `MAPNIK_PLUGINS_DIR_<CONFIGURATION>` where `<CONFIGURATION>` is one of `CMAKE_BUILD_TYPE` as upper case.
There is a function exported which you can use to query the plugin dir for known typical configurations:
```
mapnik_find_plugin_dir(MAPNIK_PLUGINS_DIR)
```
`MAPNIK_PLUGINS_DIR` will contain then the plugin dir. If a path could not be found, a warning is printed.

## Recommendations

If you target a specific platform, it is recommended to create a toolchain file and set all the options and library path that you would normally set via cmd line options.
If you are using a recent cmake version (>=3.20), it is recommended to use a CMakePreset instead. https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html


## CMakePreset example

If you are using CMakePresets and need to add vcpkg integration, just create a `CMakeUserPresets.json` file beside `CMakePresets.json. 
This could look like this:
```json
{
    "version": 2,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 20,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "vcpkg",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_TOOLCHAIN_FILE": "<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake"
            }
        }
        {
            "name": "vcpkg-x64-win-debug",
            "inherits": ["vcpkg", "windows-default-debug"]
        },
        {
            "name": "vcpkg-x64-win-release",
            "inherits": ["vcpkg", "windows-default-release"]
        }
    ]
}
```


If your libraries are not in the global search paths, you could add a own `CMakeUserPresets.json` with 

```json
{
    "version": 2,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 20,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "linux-clang-debug-own",
            "inherits": "linux-clang-debug",
            "cacheVariables": {
                "WebP_DIR": "/home/myuser/webp/cmake",
                "USE_CAIRO": "OFF",
                "CMAKE_INSTALL_PREFIX": "${sourceDir}/install"
            }
        }
    ]
}
```

Build with: 
```
$ cmake --preset <configure_preset_name>
$ cmake --build --preset <build_preset_name>
```
