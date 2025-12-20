add_library(macmanager_bridge ${_MM_LIB_TYPE}
  src/macmanager_bridge.cpp
)

target_include_directories(macmanager_bridge
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(macmanager_bridge
  PUBLIC macmanager_core
)

target_compile_features(macmanager_bridge PUBLIC cxx_std_20)
