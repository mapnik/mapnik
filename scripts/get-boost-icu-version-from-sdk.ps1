$ErrorActionPreference = 'Stop'
$boost_version='0'
$icu_version='0'
$libdir=$PSScriptRoot+"\..\mapnik-gyp\mapnik-sdk\lib"

#get boost and icu versions directly from the files in the SDK

#boost_python-vc140-mt-1_61.dll
$boost_version=(Get-ChildItem $libdir -Filter *boost*.dll)[0].BaseName.split("_")[-1]

#icuin56.dll
$icu_version=(Get-ChildItem $libdir -Filter icuin*.dll)[0].BaseName.split("icuin")[-1]

Write-Host "BOOST_VERSION" $boost_version
Write-Host "ICU_VERSION" $icu_version".1"
Write-Host "ICU_VERSION2" $icu_version"_1"

trap {
    "Error: $_"
    exit 1
}
