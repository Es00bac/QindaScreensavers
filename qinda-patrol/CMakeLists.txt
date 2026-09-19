cmake_minimum_required(VERSION 3.20)
project(QindaPatrol VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(PATROL_BUILD_DESKTOP "Build the Qt 6 desktop screensaver" ON)
option(PATROL_BUILD_QUICK "Build the optional Qt Quick embedding adapter" OFF)
option(PATROL_LAYER_SHELL "Use LayerShellQt (wlr-layer-shell) for per-output overlay placement on Wayland when available" ON)
include(CTest)
find_package(Threads REQUIRED)
add_library(patrol_core src/world.cpp src/metrics.cpp src/canvas.cpp src/assets.cpp src/renderer.cpp)
set_target_properties(patrol_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(patrol_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(patrol_core PUBLIC Threads::Threads)
target_compile_options(patrol_core PRIVATE -Wall -Wextra -Wpedantic)
add_executable(qinda-patrol-render src/render_main.cpp)
target_link_libraries(qinda-patrol-render PRIVATE patrol_core)
if(PATROL_BUILD_DESKTOP)
  find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui)
  add_executable(qinda-patrol src/main.cpp src/patrol_window.cpp)
  target_link_libraries(qinda-patrol PRIVATE patrol_core Qt6::Core Qt6::Gui)
  if(PATROL_LAYER_SHELL)
    find_package(LayerShellQt QUIET)
  endif()
  if(LayerShellQt_FOUND)
    # One overlay surface bound to each output: compositors that ignore the output hint of
    # xdg-shell fullscreen still place these correctly, and docks stay beneath them.
    target_link_libraries(qinda-patrol PRIVATE LayerShellQt::Interface)
    target_compile_definitions(qinda-patrol PRIVATE PATROL_HAVE_LAYER_SHELL=1)
    message(STATUS "LayerShellQt found: per-output overlay placement enabled on Wayland")
  else()
    message(STATUS "LayerShellQt not found: Wayland placement uses xdg-shell fullscreen only")
  endif()
  install(TARGETS qinda-patrol RUNTIME DESTINATION bin)
  install(FILES packaging/org.qindaqt.Patrol.desktop DESTINATION share/applications)
  install(FILES assets/qinda-patrol.svg DESTINATION share/icons/hicolor/scalable/apps)
endif()
if(PATROL_BUILD_QUICK)
  find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Qml)
  set(CMAKE_AUTOMOC ON)
  add_library(qinda-patrol-quick STATIC src/patrol_item.cpp src/patrol_item.hpp)
  target_link_libraries(qinda-patrol-quick PUBLIC patrol_core Qt6::Quick Qt6::Qml)
  target_include_directories(qinda-patrol-quick PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()

# An installable QML module, so a host that does not link this project can still
# show the scene: the QindaQt screen locker draws its wallpaper from QML and
# never links a screensaver. Separate from qinda-patrol-quick above, which stays
# the static library an application embeds directly.
option(PATROL_BUILD_QML_MODULE "Install the Qinda.Patrol QML module" OFF)
if(PATROL_BUILD_QML_MODULE)
  find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Qml)
  include(GNUInstallDirs)
  qt_standard_project_setup(REQUIRES 6.4)
  qt_add_library(qinda-patrol-qml SHARED)
  target_link_libraries(qinda-patrol-qml PRIVATE patrol_core Qt6::Quick Qt6::Qml)
  target_include_directories(qinda-patrol-qml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  set(PATROL_QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt6/qml"
      CACHE PATH "Directory the Qinda.Patrol QML module is installed into")
  qt_add_qml_module(qinda-patrol-qml
    URI Qinda.Patrol
    VERSION 1.0
    SOURCES src/patrol_item.cpp src/patrol_item.hpp
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qml/Qinda/Patrol"
  )
  install(TARGETS qinda-patrol-qml qinda-patrol-qmlplugin
    LIBRARY DESTINATION "${PATROL_QML_INSTALL_DIR}/Qinda/Patrol"
    RUNTIME DESTINATION "${PATROL_QML_INSTALL_DIR}/Qinda/Patrol")
  install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/qml/Qinda/Patrol/qmldir"
      "${CMAKE_CURRENT_BINARY_DIR}/qml/Qinda/Patrol/qinda-patrol-qml.qmltypes"
    DESTINATION "${PATROL_QML_INSTALL_DIR}/Qinda/Patrol")
endif()
if(BUILD_TESTING)
  add_executable(patrol-tests tests/test_main.cpp)
  target_link_libraries(patrol-tests PRIVATE patrol_core)
  add_test(NAME patrol-core COMMAND patrol-tests)
endif()
install(TARGETS qinda-patrol-render RUNTIME DESTINATION bin)
