cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES "${PROJECT_ROOT_DIR}/Libraries/LibM/math.cpp")
set(INCLUDE_DIRS "${PROJECT_ROOT_DIR}/Libraries/LibM" "${PROJECT_ROOT_DIR}/Libraries")
set(STATIC_LINK_LIBRARIES "")

add_library(m STATIC ${SOURCES})
target_include_directories(m PRIVATE ${INCLUDE_DIRS})
target_link_libraries(m PRIVATE ${STATIC_LINK_LIBRARIES})

install(
  TARGETS m
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

file(GLOB_RECURSE HEADERS ${PROJECT_ROOT_DIR}/Libraries/LibM/*.h)
install(FILES ${HEADERS} DESTINATION include)
