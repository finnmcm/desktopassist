# tools/cli/CMakeLists.txt

add_executable(macassistant_cli
  main.cpp
)

# Link the CLI against your Swift-safe bridge (which links core internally)
target_link_libraries(macassistant_cli
  PRIVATE macmanager_bridge
)

# Optional: enforce same C++ standard
target_compile_features(macassistant_cli PRIVATE cxx_std_20)

# Convenience: build+run
add_custom_target(run
  COMMAND $<TARGET_FILE:macassistant_cli>
  DEPENDS macassistant_cli
)
