cmake_minimum_required(VERSION 2.8.12)

project(LinqPP)

function(add_precompiled_header header cppFile targetSources)
  if(MSVC)
    set(precompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/pch.pch")

    set_source_files_properties(${cppFile}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${header}\" /Fp\"${precompiledBinary}\""
                                           OBJECT_OUTPUTS "${precompiledBinary}")
    set_source_files_properties(${${targetSources}}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${header}\" /Fp\"${precompiledBinary}\""
                                           OBJECT_DEPENDS "${precompiledBinary}")

    set(${targetSources} ${${targetSources}} ${cppFile} PARENT_SCOPE)
  endif(MSVC)
endfunction()

include_directories(inc)
include_directories(bandit)
add_subdirectory(src)
