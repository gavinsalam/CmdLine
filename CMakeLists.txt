cmake_minimum_required(VERSION 3.16)

project(CmdLine VERSION 3.4.1 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMDLINE_IS_TOP_LEVEL ON)
else()
  set(CMDLINE_IS_TOP_LEVEL OFF)
endif()

option(CMDLINE_BUILD_EXAMPLE "Build the CmdLine example program" ${CMDLINE_IS_TOP_LEVEL})
option(CMDLINE_BUILD_TESTS "Build the CmdLine unit tests" ${CMDLINE_IS_TOP_LEVEL})

check_cxx_source_compiles("
  #include <cxxabi.h>
  #include <cstdlib>

  int main() {
    int status = 0;
    char* name = abi::__cxa_demangle(\"i\", nullptr, nullptr, &status);
    std::free(name);
    return status;
  }
" CMDLINE_HAVE_CXA_DEMANGLE)

if(CMDLINE_HAVE_CXA_DEMANGLE)
  message(STATUS "CmdLine: abi::__cxa_demangle() found; enabling demangled type names")
else()
  message(STATUS "CmdLine: abi::__cxa_demangle() not found; using raw type names")
endif()

function(cmdline_configure_library target_name)
  target_compile_features(${target_name} PUBLIC cxx_std_17)
  if(CMDLINE_HAVE_CXA_DEMANGLE)
    target_compile_definitions(${target_name} PRIVATE __CMDLINE_ABI_DEMANGLE__)
  endif()
  target_include_directories(${target_name} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target_name} PRIVATE
      -pedantic
      -Wall
      -Wextra
      -Wsign-compare
      -Wshadow
    )
  endif()
endfunction()

add_library(CmdLineStatic STATIC
  CmdLine.cc
)
add_library(CmdLine::CmdLineStatic ALIAS CmdLineStatic)
set_target_properties(CmdLineStatic PROPERTIES
  OUTPUT_NAME CmdLine
  POSITION_INDEPENDENT_CODE ON
  EXPORT_NAME CmdLineStatic
)
cmdline_configure_library(CmdLineStatic)

add_library(CmdLineShared SHARED
  CmdLine.cc
)
add_library(CmdLine::CmdLine ALIAS CmdLineShared)
set_target_properties(CmdLineShared PROPERTIES
  OUTPUT_NAME CmdLine
  EXPORT_NAME CmdLine
)
cmdline_configure_library(CmdLineShared)

if(CMDLINE_BUILD_EXAMPLE)
  add_executable(example
    example.cc
  )
  target_link_libraries(example PRIVATE CmdLine::CmdLine)
endif()

if(CMDLINE_BUILD_TESTS)
  include(CTest)

  add_executable(unit-tests
    unit-tests.cc
  )
  target_link_libraries(unit-tests PRIVATE CmdLine::CmdLine)

  if(BUILD_TESTING)
    add_test(NAME unit-tests COMMAND unit-tests)
    if(TARGET example)
      add_test(NAME example-uses-option COMMAND example -i 2)
      add_test(NAME example-help COMMAND example -h)
    endif()
  endif()
endif()

install(TARGETS CmdLineStatic CmdLineShared
  EXPORT CmdLineTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES CmdLine.hh DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CmdLineConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CmdLineConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CmdLine"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/CmdLineConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(EXPORT CmdLineTargets
  FILE CmdLineTargets.cmake
  NAMESPACE CmdLine::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CmdLine"
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/CmdLineConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/CmdLineConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CmdLine"
)

export(EXPORT CmdLineTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/CmdLineTargets.cmake"
  NAMESPACE CmdLine::
)
