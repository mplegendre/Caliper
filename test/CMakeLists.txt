if (${CMAKE_CXX_COMPILER_ID} MATCHES Intel)
  # CMake does not have proper compiler feature support for Intel compilers :-/
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
else()
  set(CMAKE_C_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_STANDARD_REQUIRED ON) 

  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_C_STANDARD   99)
endif()

set(CALIPER_TEST_APPS
  cali-annotation-perftest
  cali-flush-perftest
  cali-test
  cali-throughput-pthread)

find_package(OpenMP)

foreach(app ${CALIPER_TEST_APPS})
  add_executable(${app} ${app}.cpp)
  target_link_libraries(${app} caliper-serial)
endforeach()

set(CALIPER_TEST_OMP_APPS
  cali-annotation-perftest
  cali-flush-perftest)

if (${OpenMP_FOUND})
  foreach (app ${CALIPER_TEST_OMP_APPS})
    target_compile_options(${app}
      PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(${app}
      ${OpenMP_CXX_FLAGS})
  endforeach()
endif()

target_link_libraries(cali-annotation-perftest
  caliper-tools-util)
target_link_libraries(cali-flush-perftest
  caliper-tools-util)
target_link_libraries(cali-throughput-pthread
  caliper-tools-util
  Threads::Threads)

add_subdirectory(ci_app_tests)
