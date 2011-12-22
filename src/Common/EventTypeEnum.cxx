
/*LICENSE_START*/
/* 
 *  Copyright 1995-2011 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#define __EVENT_TYPE_ENUM_DECLARE__
#include "EventTypeEnum.h"
#undef __EVENT_TYPE_ENUM_DECLARE__

#include "CaretAssert.h"

using namespace caret;

/**
 * Constructor.
 *
 * @param enumValue
 *    An enumerated value.
 * @param name
 *    Name of enumberated value.
 */
EventTypeEnum::EventTypeEnum(const Enum enumValue,
                           const AString& name,
                           const AString& guiName)
{
    this->enumValue = enumValue;
    this->name = name;
    this->guiName = guiName;
}

/**
 * Destructor.
 */
EventTypeEnum::~EventTypeEnum()
{
}

/**
 * Initialize the enumerated metadata.
 */
void
EventTypeEnum::initialize()
{
    if (initializedFlag) {
        return;
    }
    initializedFlag = true;

    enumData.push_back(EventTypeEnum(EVENT_INVALID, 
                                     "EVENT_INVALID", 
                                     "Invalid Event"));
    
    enumData.push_back(EventTypeEnum(EVENT_BROWSER_TAB_DELETE, 
                                     "EVENT_BROWSER_TAB_DELETE", 
                                     "Delete a browser tab"));
    
    enumData.push_back(EventTypeEnum(EVENT_BROWSER_TAB_GET, 
                                    "EVENT_BROWSER_TAB_GET", 
                                    "Get a browser tab by number"));
    
    enumData.push_back(EventTypeEnum(EVENT_BROWSER_TAB_GET_ALL, 
                                     "EVENT_BROWSER_TAB_GET_ALL", 
                                     "Get ALL browser tabs"));
    
    enumData.push_back(EventTypeEnum(EVENT_BROWSER_TAB_NEW, 
                                     "EVENT_BROWSER_TAB_NEW", 
                                     "Create a browser tab"));
    
    enumData.push_back(EventTypeEnum(EVENT_BROWSER_WINDOW_CONTENT_GET, 
                                     "EVENT_BROWSER_WINDOW_CONTENT_GET", 
                                     "Get the content in a browser window"));
    
    enumData.push_back(EventTypeEnum(EVENT_BROWSER_WINDOW_NEW, 
                                     "EVENT_BROWSER_WINDOW_NEW", 
                                     "Create a new browser window"));
    
    enumData.push_back(EventTypeEnum(EVENT_CARET_MAPPABLE_DATA_FILES_GET, 
                                     "EVENT_CARET_MAPPABLE_DATA_FILES_GET", 
                                     "Get all Caret Mappable data files"));
    
    enumData.push_back(EventTypeEnum(EVENT_DATA_FILE_READ, 
                                    "EVENT_DATA_FILE_READ", 
                                    "Read a data file"));
    
    enumData.push_back(EventTypeEnum(EVENT_GET_NODE_DATA_FILES,
                                     "EVENT_GET_NODE_DATA_FILES",
                                     "Get node data files"));
    
    enumData.push_back(EventTypeEnum(EVENT_GRAPHICS_UPDATE_ALL_WINDOWS, 
                                     "EVENT_GRAPHICS_UPDATE_ALL_WINDOWS", 
                                     "Update all graphics windows"));
    
    enumData.push_back(EventTypeEnum(EVENT_GRAPHICS_UPDATE_ONE_WINDOW, 
                                     "EVENT_GRAPHICS_UPDATE_ONE_WINDOW", 
                                     "Update graphics in one window"));
    
    enumData.push_back(EventTypeEnum(EVENT_IDENTIFICATION_SYMBOL_REMOVAL, 
                                     "EVENT_IDENTIFICATION_SYMBOL_REMOVAL", 
                                     "Remove all identification symbols"));
    
    enumData.push_back(EventTypeEnum(EVENT_INFORMATION_TEXT_DISPLAY,
                                     "EVENT_INFORMATION_TEXT_DISPLAY",
                                     "Display text in information windows"));
    
    enumData.push_back(EventTypeEnum(EVENT_MAP_SCALAR_DATA_COLOR_MAPPING_EDITOR,
                                     "EVENT_MAP_SCALAR_DATA_COLOR_MAPPING_EDITOR",
                                     "Display map scalar data color mapping editor"));
    
    enumData.push_back(EventTypeEnum(EVENT_MODEL_DISPLAY_CONTROLLER_ADD, 
                                     "EVENT_MODEL_DISPLAY_CONTROLLER_ADD", 
                                     "Add a model display controller"));
    
    enumData.push_back(EventTypeEnum(EVENT_MODEL_DISPLAY_CONTROLLER_DELETE, 
                                    "EVENT_MODEL_DISPLAY_CONTROLLER_DELETE", 
                                    "Delete a model display controller"));
    
    enumData.push_back(EventTypeEnum(EVENT_MODEL_DISPLAY_CONTROLLER_GET_ALL, 
                                    "EVENT_MODEL_DISPLAY_CONTROLLER_GET_ALL", 
                                    "Get all model display controllers"));

    enumData.push_back(EventTypeEnum(EVENT_MODEL_DISPLAY_CONTROLLER_YOKING_GROUP_GET_ALL, 
                                     "EVENT_MODEL_DISPLAY_CONTROLLER_YOKING_GROUP_GET_ALL", 
                                     "Get all model display YOKING GROUP controllers"));
    
    enumData.push_back(EventTypeEnum(EVENT_SPEC_FILE_READ_DATA_FILES,
                                     "EVENT_SPEC_FILE_READ_DATA_FILES",
                                     "Read the selected data files in a spec file"));
    
    enumData.push_back(EventTypeEnum(EVENT_SURFACE_COLORING_INVALIDATE, 
                                     "EVENT_SURFACE_COLORING_INVALIDATE", 
                                     "Invalidate surface coloring"));
    
    enumData.push_back(EventTypeEnum(EVENT_SURFACES_GET, 
                                     "EVENT_SURFACES_GET", 
                                     "Get Surfaces"));
    
    enumData.push_back(EventTypeEnum(EVENT_USER_INTERFACE_UPDATE, 
                                     "EVENT_USER_INTERFACE_UPDATE", 
                                     "Update the user-interface"));
    
    enumData.push_back(EventTypeEnum(EVENT_PROGRESS_UPDATE, 
                                     "EVENT_PROGRESS_UPDATE", 
                                     "Update the progress amount, text, or finished status"));
    
    enumData.push_back(EventTypeEnum(EVENT_COUNT, 
                                    "EVENT_COUNT", 
                                    "Count of events"));
    
    CaretAssertMessage((enumData.size() == static_cast<uint64_t>(EVENT_COUNT + 1)),
                       ("Number of EventTypeEnum::Enum values is incorrect.\n"
                        "Have enumerated type been added?"));
}

/**
 * Find the data for and enumerated value.
 * @param enumValue
 *     The enumerated value.
 * @return Pointer to data for this enumerated type
 * or NULL if no data for type or if type is invalid.
 */
const EventTypeEnum*
EventTypeEnum::findData(const Enum enumValue)
{
    if (initializedFlag == false) initialize();

    size_t num = enumData.size();
    for (size_t i = 0; i < num; i++) {
        const EventTypeEnum* d = &enumData[i];
        if (d->enumValue == enumValue) {
            return d;
        }
    }

    return NULL;
}

/**
 * Get a string representation of the enumerated type.
 * @param enumValue 
 *     Enumerated value.
 * @return 
 *     String representing enumerated value.
 */
AString 
EventTypeEnum::toName(Enum enumValue) {
    if (initializedFlag == false) initialize();
    
    const EventTypeEnum* enumInstance = findData(enumValue);
    return enumInstance->name;
}

/**
 * Get an enumerated value corresponding to its name.
 * @param name 
 *     Name of enumerated value.
 * @param isValidOut 
 *     If not NULL, it is set indicating that a
 *     enum value exists for the input name.
 * @return 
 *     Enumerated value.
 */
EventTypeEnum::Enum 
EventTypeEnum::fromName(const AString& name, bool* isValidOut)
{
    if (initializedFlag == false) initialize();
    
    bool validFlag = false;
    Enum enumValue = EVENT_INVALID;
    
    for (std::vector<EventTypeEnum>::iterator iter = enumData.begin();
         iter != enumData.end();
         iter++) {
        const EventTypeEnum& d = *iter;
        if (d.name == name) {
            enumValue = d.enumValue;
            validFlag = true;
            break;
        }
    }
    
    if (isValidOut != 0) {
        *isValidOut = validFlag;
    }
    else if (validFlag == false) {
        CaretAssertMessage(0, AString("Name " + name + "failed to match enumerated value for type EventTypeEnum"));
    }
    return enumValue;
}

/**
 * Get a GUI string representation of the enumerated type.
 * @param enumValue 
 *     Enumerated value.
 * @return 
 *     String representing enumerated value.
 */
AString 
EventTypeEnum::toGuiName(Enum enumValue) {
    if (initializedFlag == false) initialize();
    
    const EventTypeEnum* enumInstance = findData(enumValue);
    return enumInstance->guiName;
}

/**
 * Get an enumerated value corresponding to its GUI name.
 * @param s 
 *     Name of enumerated value.
 * @param isValidOut 
 *     If not NULL, it is set indicating that a
 *     enum value exists for the input name.
 * @return 
 *     Enumerated value.
 */
EventTypeEnum::Enum 
EventTypeEnum::fromGuiName(const AString& guiName, bool* isValidOut)
{
    if (initializedFlag == false) initialize();
    
    bool validFlag = false;
    Enum enumValue = EVENT_INVALID;
    
    for (std::vector<EventTypeEnum>::iterator iter = enumData.begin();
         iter != enumData.end();
         iter++) {
        const EventTypeEnum& d = *iter;
        if (d.guiName == guiName) {
            enumValue = d.enumValue;
            validFlag = true;
            break;
        }
    }
    
    if (isValidOut != 0) {
        *isValidOut = validFlag;
    }
    else if (validFlag == false) {
        CaretAssertMessage(0, AString("guiName " + guiName + "failed to match enumerated value for type EventTypeEnum"));
    }
    return enumValue;
}

/**
 * Get all of the enumerated type values.  The values can be used
 * as parameters to toXXX() methods to get associated metadata.
 *
 * @param allEnums
 *     A vector that is OUTPUT containing all of the enumerated values.
 */
void
EventTypeEnum::getAllEnums(std::vector<EventTypeEnum::Enum>& allEnums)
{
    if (initializedFlag == false) initialize();
    
    allEnums.clear();
    
    for (std::vector<EventTypeEnum>::iterator iter = enumData.begin();
         iter != enumData.end();
         iter++) {
        allEnums.push_back(iter->enumValue);
    }
}

