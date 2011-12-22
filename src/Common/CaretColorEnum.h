#ifndef __CARET_COLOR_ENUM__H_
#define __CARET_COLOR_ENUM__H_

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


#include <stdint.h>
#include <vector>
#include "AString.h"

namespace caret {

class CaretColorEnum {

public:
    /**
     * Enumerated values.
     */
    enum Enum {
        /** Special case for using surface colooring */
        SURFACE,
        /** AQUA */
        AQUA,
        /** Black */
        BLACK,
        /** Blue */
        BLUE,
        /** Fuchsia */
        FUCHSIA,
        /** Gray */
        GRAY,
        /** Green */
        GREEN,
        /** Lime */
        LIME,
        /** Maroon */
        MAROON,
        /** Navy Blue */
        NAVY,
        /** Olive */
        OLIVE,
        /** Purple */
        PURPLE,
        /** Red */
        RED,
        /** Silver */
        SILVER,
        /** Teal */
        TEAL,
        /** White */
        WHITE,
        /** Yellow */
        YELLOW
    };


    ~CaretColorEnum();

    static AString toName(Enum enumValue);
    
    static Enum fromName(const AString& name, bool* isValidOut);
    
    static AString toGuiName(Enum enumValue);
    
    static Enum fromGuiName(const AString& guiName, bool* isValidOut);
    
    static int32_t toIntegerCode(Enum enumValue);
    
    static Enum fromIntegerCode(const int32_t integerCode, bool* isValidOut);

    static void getAllEnums(std::vector<Enum>& allEnums,
                            const bool includeSurfaceColor = false);

    static void getAllNames(std::vector<AString>& allNames, const bool isSorted,
                            const bool includeSurfaceColor = false);

    static void getAllGuiNames(std::vector<AString>& allGuiNames, const bool isSorted,
                               const bool includeSurfaceColor = false);

    static const float* toRGB(Enum enumValue);
    
private:
    CaretColorEnum(const Enum enumValue, 
                   const AString& name,
                   const AString& guiName,
                   const float red,
                   const float green,
                   const float blue);

    static const CaretColorEnum* findData(const Enum enumValue);

    /** Holds all instance of enum values and associated metadata */
    static std::vector<CaretColorEnum> enumData;

    /** Initialize instances that contain the enum values and metadata */
    static void initialize();

    /** Indicates instance of enum values and metadata have been initialized */
    static bool initializedFlag;
    
    /** Auto generated integer codes */
    static int32_t integerCodeCounter;
    
    /** The enumerated type value for an instance */
    Enum enumValue;

    /** The integer code associated with an enumerated value */
    int32_t integerCode;

    /** The name, a text string that is identical to the enumerated value */
    AString name;
    
    /** A user-friendly name that is displayed in the GUI */
    AString guiName;
    
    /** RGB color components */
    float rgb[3];
};

#ifdef __CARET_COLOR_ENUM_DECLARE__
std::vector<CaretColorEnum> CaretColorEnum::enumData;
bool CaretColorEnum::initializedFlag = false;
int32_t CaretColorEnum::integerCodeCounter = 0; 
#endif // __CARET_COLOR_ENUM_DECLARE__

} // namespace
#endif  //__CARET_COLOR_ENUM__H_
