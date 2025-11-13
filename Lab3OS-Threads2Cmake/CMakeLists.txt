if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Lab3OS-Threads2Cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED) 

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Lab3OS-Threads2Cmake "syncronization.cpp" "marker.cpp" "marker.h" "head.h")

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

target_include_directories(Lab3OS-Threads2Cmake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Lab3OS-Threads2Cmake PRIVATE Threads::Threads)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Lab3OS-Threads2Cmake PROPERTY CXX_STANDARD 20)
endif()

add_executable(MarkerTests "syncronization.cpp" "test.cpp" "marker.h" "head.h" "marker.cpp")
target_compile_definitions(MarkerTests PRIVATE UNIT_TESTS)
target_include_directories(MarkerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MarkerTests PRIVATE gtest gtest_main Threads::Threads)

include(GoogleTest)	
enable_testing()
add_test(NAME MarkerTests COMMAND MarkerTests)