file (GLOB HEADERS "*.h")
file (GLOB SOURCES "[^_]*.cpp")

set(LIBRARY tex)
add_library(${LIBRARY} STATIC ${SOURCES})
set_property(TARGET ${LIBRARY} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
add_dependencies(${LIBRARY} ext_rayint ext_mapmap)
target_link_libraries(${LIBRARY} ${TBB_LIBRARIES} -lmve -lmve_util tbb ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${TIFF_LIBRARIES})
install(TARGETS ${LIBRARY} ARCHIVE DESTINATION lib)
