
find_package(TIFF REQUIRED)

#Moench ZMQ
add_executable(moenchZmqProcess moenchZmqProcess.cpp ../tiffIO.cpp)
add_compile_definitions(moenchZmqProcess NEWZMQ INTERP)

#Moench04 ZMQ
add_executable(moench04ZmqProcess moenchZmqProcess.cpp ../tiffIO.cpp)
add_compile_definitions(moenchZmqProcess NEWZMQ INTERP MOENCH04)


#Both executables should have the same includes and output dirs
set(MOENCH_EXECUTABLES "moenchZmqProcess" "moench04ZmqProcess")

foreach(exe ${MOENCH_EXECUTABLES})
    #TODO! At a later stage clean up include dirs and have a proper lib
    target_include_directories(${exe} PRIVATE 
        ../ 
        ../dataStructures 
        ../interpolations
        ../../slsReceiverSoftware/include/
    )

    target_link_libraries(${exe} 
        PUBLIC
        slsSupportStatic
        ${ZeroMQ_LIBRARIES} 
        pthread 
        TIFF::TIFF

        PRIVATE
        slsProjectWarnings
    )


    set_target_properties(${exe}  PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if(SLS_LTO_AVAILABLE)
        set_property(TARGET ${exe} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()


endforeach(exe ${MOENCH_EXECUTABLES})
