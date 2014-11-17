project(gcviewer)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui)

if(WITH_OPENGL)
  find_package(OpenGL REQUIRED)
  find_package(Qt4 REQUIRED COMPONENTS QtOpenGL)
endif(WITH_OPENGL)

set (GCVIEWER_SOURCES
  src/GCViewerMW.cpp
  src/GC2DView.cpp
  src/GCThreadItem.cpp
  src/GCGraphicsView.cpp
  src/GCAbstractView.cpp
  src/GCModel.cpp
  src/FilamentSettingsDia.cpp
  src/GC3DViewSettingsDia.cpp
  src/main.cpp
  )

set (GCVIEWER_HEADERS
  src/GCViewerMW.h
  src/GC2DView.h
  src/GCGraphicsView.h
  src/GCAbstractView.h
  src/GCModel.h
  src/FilamentSettingsDia.h
  src/GC3DViewSettingsDia.h
  )

set (GCVIEWER_FORMS
  src/GCViewerMW.ui
  )

set (GCVIEWER_RESOURCES
  src/gcviewer.qrc
  )

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
  add_definitions(-DBUILD_3D)
  set (GCVIEWER_SOURCES ${GCVIEWER_SOURCES} src/GC3DView.cpp)
  set (GCVIEWER_SOURCES ${GCVIEWER_SOURCES} src/GCGLView.cpp)
  set (GCVIEWER_HEADERS ${GCVIEWER_HEADERS} src/GC3DView.h)
  set (GCVIEWER_HEADERS ${GCVIEWER_HEADERS} src/GCGLView.h)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

QT4_WRAP_CPP(GCVIEWER_HEADERS_MOC ${GCVIEWER_HEADERS})
QT4_WRAP_UI(GCVIEWER_FORMS_HEADERS ${GCVIEWER_FORMS})
QT4_ADD_RESOURCES(GCVIEWER_RESOURCES_RCC ${GCVIEWER_RESOURCES})
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR} src ${OPENGL_INCLUDE_DIR})
add_executable(gcviewer ${GCVIEWER_SOURCES} ${GCVIEWER_HEADERS_MOC} ${GCVIEWER_FORMS_HEADERS} ${GCVIEWER_RESOURCES_RCC})
target_link_libraries(gcviewer ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY})
install(TARGETS gcviewer RUNTIME DESTINATION bin)
