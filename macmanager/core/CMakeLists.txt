add_library(macmanager_core ${_MM_LIB_TYPE}
  # Add all core .cpp files here
  src/app.cpp
  src/file_manager.cpp
  # src/FileSystem.cpp
  # src/Process.cpp
)

target_include_directories(macmanager_core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_library(SQLITE3_LIBRARY sqlite3 REQUIRED)

target_link_libraries(macmanager_core
    PRIVATE ${SQLITE3_LIBRARY}
)

target_compile_features(macmanager_core PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(macmanager_core PRIVATE /W4)
else()
  target_compile_options(macmanager_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
