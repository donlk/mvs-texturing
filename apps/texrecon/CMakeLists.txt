file (GLOB HEADERS "*.h")
file (GLOB SOURCES "[^_]*.cpp")

set(BIN texrecon)

add_executable(${BIN} ${SOURCES})
target_link_libraries(${BIN} tex -lmve -lmve_util)
install(TARGETS ${BIN} RUNTIME DESTINATION bin)
