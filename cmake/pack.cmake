include(InstallRequiredSystemLibraries)
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
