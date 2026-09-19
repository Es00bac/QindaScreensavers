# SPDX-License-Identifier: GPL-3.0-or-later
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Quick Qml)
add_library(circuit-reef-qt STATIC CircuitReefItem.cpp CircuitReefItem.hpp)
set_target_properties(circuit-reef-qt PROPERTIES AUTOMOC ON)
target_include_directories(circuit-reef-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(circuit-reef-qt PUBLIC reef-core Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml)
add_executable(circuit-reef-qt-preview preview.cpp)
target_link_libraries(circuit-reef-qt-preview PRIVATE circuit-reef-qt)

# An installable QML module for hosts that cannot link this library: the QindaQt
# screen locker draws its wallpaper from QML and links no screensaver.
if(REEF_BUILD_QML_MODULE)
  include(GNUInstallDirs)
  qt_standard_project_setup(REQUIRES 6.4)
  qt_add_library(circuit-reef-qml SHARED)
  target_link_libraries(circuit-reef-qml PRIVATE reef-core Qt6::Quick Qt6::Qml)
  target_include_directories(circuit-reef-qml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  set(REEF_QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt6/qml"
      CACHE PATH "Directory the QindaQt.CircuitReef QML module is installed into")
  qt_add_qml_module(circuit-reef-qml
    URI QindaQt.CircuitReef
    VERSION 1.0
    SOURCES CircuitReefItem.cpp CircuitReefItem.hpp
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qml/QindaQt/CircuitReef"
  )
  install(TARGETS circuit-reef-qml circuit-reef-qmlplugin
    LIBRARY DESTINATION "${REEF_QML_INSTALL_DIR}/QindaQt/CircuitReef"
    RUNTIME DESTINATION "${REEF_QML_INSTALL_DIR}/QindaQt/CircuitReef")
  install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/qml/QindaQt/CircuitReef/qmldir"
      "${CMAKE_CURRENT_BINARY_DIR}/qml/QindaQt/CircuitReef/circuit-reef-qml.qmltypes"
    DESTINATION "${REEF_QML_INSTALL_DIR}/QindaQt/CircuitReef")
endif()
