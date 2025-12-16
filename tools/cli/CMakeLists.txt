# tools/cli/CMakeLists.txt

add_executable(macmanager
  main.cpp
)

# Link the CLI against your Swift-safe bridge (which links core internally)
target_link_libraries(macmanager
    macmanager_core
    #link against the bridge when swift time
 # PRIVATE macmanager_bridge

)

# Optional: enforce same C++ standard
target_compile_features(macmanager PRIVATE cxx_std_20)

# Convenience: build+run
add_custom_target(run
    COMMAND macmanager
    DEPENDS macmanager
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
