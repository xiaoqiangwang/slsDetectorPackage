cmake_policy(SET CMP0071 OLD)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCES
    slsDetectorPlotting/src/SlsQt1DPlot.cxx
    slsDetectorPlotting/src/SlsQt1DZoomer.cxx
    slsDetectorPlotting/src/SlsQt2DHist.cxx
    slsDetectorPlotting/src/SlsQt2DPlot.cxx
    slsDetectorPlotting/src/SlsQt2DPlotLayout.cxx
    src/qDetectorMain.cpp
    src/qDrawPlot.cpp
    src/qCloneWidget.cpp
    src/qTabMeasurement.cpp
    src/qTabDataOutput.cpp
    src/qTabPlot.cpp
    src/qTabAdvanced.cpp
    src/qTabSettings.cpp
    src/qTabDebugging.cpp
    src/qTabDeveloper.cpp
    src/qTabMessages.cpp
)

set(FORMS
    forms/form_detectormain.ui
    forms/form_tab_measurement.ui
    forms/form_tab_dataoutput.ui
    forms/form_tab_plot.ui
    forms/form_tab_advanced.ui
    forms/form_tab_settings.ui
    forms/form_tab_debugging.ui
    forms/form_tab_messages.ui 
)

qt5_wrap_ui(FORMS_H ${FORMS})

set(HEADERS
    slsDetectorPlotting/include/SlsQt1DPlot.h
    slsDetectorPlotting/include/SlsQt1DZoomer.h
    slsDetectorPlotting/include/SlsQt2DHist.h
    slsDetectorPlotting/include/SlsQt2DPlot.h
    slsDetectorPlotting/include/SlsQt2DPlotLayout.h
    slsDetectorPlotting/include/SlsQt2DZoomer.h
    include/qDefs.h
    include/qDetectorMain.h
    include/qDrawPlot.h
    include/qCloneWidget.h
    include/qTabMeasurement.h
    include/qTabDataOutput.h
    include/qTabPlot.h
    include/qTabAdvanced.h
    include/qTabSettings.h
    include/qTabDebugging.h
    include/qTabDeveloper.h
    include/qTabMessages.h
)
set(RESOURCES
    include/icons.qrc
)
qt5_add_resources(RESOURCES_SRCS ${RESOURCES})

#set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions(
    ${Qt5Widgets_DEFINITIONS} -DPRINT_LOG #-DVERBOSE 
)

add_executable(slsDetectorGui
    ${SOURCES}
    ${HEADERS}
    ${FORMS_H}
    ${RESOURCES_SRCS}
)

target_include_directories(slsDetectorGui PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/slsDetectorPlotting/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<BUILD_INTERFACE:${QT5Widgets_INCLUDE_DIRS}>"
 #   "$<BUILD_INTERFACE:${Qt5Gui_INCLUDE_DIRS}>"
 #   "$<BUILD_INTERFACE:${Qt5Core_INCLUDE_DIRS}>"
 #   "$<BUILD_INTERFACE:${Qt5OpenGL_INCLUDE_DIRS}>"
 #   "$<BUILD_INTERFACE:${Qt5Svg_INCLUDE_DIRS}>"
    "$<BUILD_INTERFACE:${Qt5Concurrent_INCLUDE_DIRS}>"  
    "$<BUILD_INTERFACE:${QWT_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

set(slsDetectorGui PUBLIC ${slsDetectorGui})

target_link_libraries(slsDetectorGui
    slsDetectorShared
    ${QWT_LIBRARIES}
    png
    z
   Qt5::Widgets
   Qt5::Gui
   Qt5::Core
   Qt5::OpenGL
   Qt5::Svg
   Qt5::Concurrent
)

set_target_properties(slsDetectorGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    # Instruct CMake to run moc automatically when needed
    AUTOMOC ON
    # Create code from a list of Qt designer ui files
    #AUTOUIC ON
)

install(TARGETS slsDetectorGui
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

