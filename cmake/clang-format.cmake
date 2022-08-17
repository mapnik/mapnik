function(format_dir dir)
    file(GLOB_RECURSE sources 
        "${dir}/*.cpp"
        "${dir}/*.hpp"
    )
    execute_process(COMMAND clang-format -style=file -i ${sources})
endfunction()


format_dir(benchmark)
format_dir(demo)
format_dir(include)
format_dir(plugins)
format_dir(src)
format_dir(test)
format_dir(utils)
