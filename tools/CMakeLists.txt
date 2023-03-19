add_executable(range-analysis range_analysis.cpp)

llvm_map_components_to_libnames(
  RANGE_ANALYSIS_LIBS
  support core irreader passes
)

target_link_libraries(
  range-analysis
  PRIVATE ${RANGE_ANALYSIS_LIBS} RangeAnalysis vSSA
)
