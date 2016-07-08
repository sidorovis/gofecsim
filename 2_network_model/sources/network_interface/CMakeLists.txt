if (ANDROID)
    include_directories("/usr/include/")
endif()
compile_project( ${module_name} "*.cpp;*.cc" "*.h" STATIC libraries)
