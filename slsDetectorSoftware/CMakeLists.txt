set(SOURCES
    src/DetectorImpl.cpp 
    src/Module.cpp 
    src/Detector.cpp
    src/CmdProxy.cpp
    src/CmdParser.cpp
)

add_library(slsDetectorObject OBJECT
    ${SOURCES}
)

target_include_directories(slsDetectorObject PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(slsDetectorObject 
    PUBLIC
        slsProjectOptions
        slsSupportStatic
        pthread
        rt
    PRIVATE
        slsProjectWarnings
)


set(PUBLICHEADERS
    include/sls/detectorData.h
    include/sls/Detector.h
    include/sls/Result.h
)

#Shared library
add_library(slsDetectorShared SHARED $<TARGET_OBJECTS:slsDetectorObject>)
target_link_libraries(slsDetectorShared PUBLIC slsDetectorObject)
set_target_properties(slsDetectorShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsDetector
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

#Static library
add_library(slsDetectorStatic STATIC $<TARGET_OBJECTS:slsDetectorObject>)
target_link_libraries(slsDetectorStatic PUBLIC slsDetectorObject)

set_target_properties(slsDetectorStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsDetectorStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)


# message("Detector LTO_AVAILABLE:" ${SLS_LTO_AVAILABLE})
if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET slsDetectorObject PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsDetectorStatic PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsDetectorShared PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

# Loop over list to generate command line binaries
set(bin_names "sls_detector_put" 
          "sls_detector_get" 
          "sls_detector_acquire" 
          "sls_detector_help")
set(cmd_name "PUT" "GET" "READOUT" "HELP")
list(LENGTH bin_names len1)
math(EXPR len2 "${len1} - 1")

foreach(val RANGE ${len2})
    list(GET bin_names ${val} val1)
    list(GET cmd_name ${val} val2)
    message(STATUS "${val1}  ${val2}")

    add_executable(${val1} src/CmdLineApp.cpp)
    target_link_libraries(${val1} 
        slsDetectorStatic
        pthread
        rt
    )
    set_target_properties(${val1} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMPILE_DEFINITIONS ${val2}=1
    )
    if(SLS_LTO_AVAILABLE)
        set_property(TARGET ${val1} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()
endforeach()


install(TARGETS ${bin_names} DESTINATION bin)


if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


install(TARGETS slsDetectorShared slsDetectorObject slsDetectorStatic
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
)
