#
# Name of Project
#
PROJECT (Annotations)

#
# Use XML from Qt but not GUI.
#
SET(QT_DONT_USE_QTGUI TRUE)

#
# QT include files
#
if(Qt5_FOUND)
    include_directories(${Qt5Core_INCLUDE_DIRS})
    #include_directories(${Qt5Network_INCLUDE_DIRS})
    include_directories(${Qt5Xml_INCLUDE_DIRS})
endif()

###INCLUDE(${QT_USE_FILE})

#
# Create a Library
#
ADD_LIBRARY(Annotations
Annotation.h
AnnotationAlignmentEnum.h
AnnotationAttributesDefaultTypeEnum.h
AnnotationBox.h
AnnotationColorBar.h
AnnotationColorBarNumericText.h
AnnotationColorBarPositionModeEnum.h
AnnotationColorBarSection.h
AnnotationCoordinate.h
AnnotationCoordinateSpaceEnum.h
AnnotationDistributeEnum.h
AnnotationFontAttributesInterface.h
AnnotationGroup.h
AnnotationGroupKey.h
AnnotationGroupTypeEnum.h
AnnotationGroupingModeEnum.h
AnnotationImage.h
AnnotationLine.h
AnnotationOneDimensionalShape.h
AnnotationOval.h
AnnotationPercentSizeText.h
AnnotationPointSizeText.h
AnnotationRedoUndoCommand.h
AnnotationRedoUndoCommandModeEnum.h
AnnotationEditingSelectionInformation.h
AnnotationSizingHandleTypeEnum.h
AnnotationSpatialModification.h
AnnotationSurfaceOffsetVectorTypeEnum.h
AnnotationText.h
AnnotationTextAlignHorizontalEnum.h
AnnotationTextAlignVerticalEnum.h
AnnotationTextConnectTypeEnum.h
AnnotationTextFontNameEnum.h
AnnotationTextFontPointSizeEnum.h
AnnotationTextFontSizeTypeEnum.h
AnnotationTextOrientationEnum.h
AnnotationTwoDimensionalShape.h
AnnotationTypeEnum.h
EventAnnotationAddToRemoveFromFile.h
EventAnnotationGroupGetWithKey.h
EventAnnotationGrouping.h

Annotation.cxx
AnnotationAlignmentEnum.cxx
AnnotationAttributesDefaultTypeEnum.cxx
AnnotationBox.cxx
AnnotationColorBar.cxx
AnnotationColorBarNumericText.cxx
AnnotationColorBarPositionModeEnum.cxx
AnnotationColorBarSection.cxx
AnnotationCoordinate.cxx
AnnotationCoordinateSpaceEnum.cxx
AnnotationDistributeEnum.cxx
AnnotationGroup.cxx
AnnotationGroupKey.cxx
AnnotationGroupTypeEnum.cxx
AnnotationGroupingModeEnum.cxx
AnnotationImage.cxx
AnnotationLine.cxx
AnnotationOneDimensionalShape.cxx
AnnotationOval.cxx
AnnotationPercentSizeText.cxx
AnnotationPointSizeText.cxx
AnnotationRedoUndoCommand.cxx
AnnotationRedoUndoCommandModeEnum.cxx
AnnotationEditingSelectionInformation.cxx
AnnotationSizingHandleTypeEnum.cxx
AnnotationSpatialModification.cxx
AnnotationSurfaceOffsetVectorTypeEnum.cxx
AnnotationText.cxx
AnnotationTextAlignHorizontalEnum.cxx
AnnotationTextAlignVerticalEnum.cxx
AnnotationTextConnectTypeEnum.cxx
AnnotationTextFontNameEnum.cxx
AnnotationTextFontPointSizeEnum.cxx
AnnotationTextFontSizeTypeEnum.cxx
AnnotationTextOrientationEnum.cxx
AnnotationTwoDimensionalShape.cxx
AnnotationTypeEnum.cxx
EventAnnotationAddToRemoveFromFile.cxx
EventAnnotationGroupGetWithKey.cxx
EventAnnotationGrouping.cxx
)

TARGET_LINK_LIBRARIES(Annotations ${CARET_QT5_LINK})

#
# Find headers
#
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/Common
${CMAKE_SOURCE_DIR}/Scenes
${CMAKE_SOURCE_DIR}/Nifti
)

