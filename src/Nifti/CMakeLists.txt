#
# The NIFTI Project
#
project (Nifti)

#
# Need XML from Qt
#

#
# Add QT for includes
#
if(Qt5_FOUND)
    include_directories(${Qt5Core_INCLUDE_DIRS})
endif()
IF (QT4_FOUND)
    SET(QT_DONT_USE_QTGUI)
    INCLUDE(${QT_USE_FILE})
ENDIF ()


#
# Create the NIFTI library
#
ADD_LIBRARY(Nifti
ControlPoint3D.h
Matrix4x4.h
NiftiHeader.h
NiftiIO.h

ControlPoint3D.cxx
Matrix4x4.cxx
NiftiHeader.cxx
NiftiIO.cxx
)

TARGET_LINK_LIBRARIES(Nifti ${CARET_QT5_LINK})

#
# Find Headers
#
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/Nifti
${CMAKE_SOURCE_DIR}/FilesBase
${CMAKE_SOURCE_DIR}/Common
${CMAKE_SOURCE_DIR}/Palette
${CMAKE_SOURCE_DIR}/Cifti
${CMAKE_SOURCE_DIR}/Scenes
${CMAKE_SOURCE_DIR}/Xml
)
