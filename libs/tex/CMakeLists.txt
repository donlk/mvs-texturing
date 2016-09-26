file (GLOB HEADERS "*.h")
file (GLOB SOURCES "[^_]*.cpp")

set(LIBRARY tex)
add_library(${LIBRARY} STATIC ${SOURCES})
target_link_libraries(${LIBRARY} mrf -lmve -lmve_util ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${TIFF_LIBRARIES})
install(TARGETS ${LIBRARY} ARCHIVE DESTINATION lib)
