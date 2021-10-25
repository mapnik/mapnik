include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "mapnik")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://mapnik.org")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  \\.git/
  build/
  ".*~$"
  out/
  \\.vs/
  \\.vscode/
)
set(CPACK_VERBATIM_VARIABLES YES)

include(CPack)
