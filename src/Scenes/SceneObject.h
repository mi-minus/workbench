#ifndef __SCENE_OBJECT__H_
#define __SCENE_OBJECT__H_

/*LICENSE_START*/
/*
 * Copyright 2012 Washington University,
 * All rights reserved.
 *
 * Connectome DB and Connectome Workbench are part of the integrated Connectome 
 * Informatics Platform.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of Washington University nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*LICENSE_END*/


#include "CaretObject.h"
#include "SceneObjectDataTypeEnum.h"
#include "XmlException.h"

namespace caret {

    class XmlWriter;
    
    class SceneObject : public CaretObject {
        
    public:
        virtual ~SceneObject();
        
        QString getName() const;
        
        SceneObjectDataTypeEnum::Enum getDataType() const;
        
        /**
         * Write the SceneObject to XML.
         * 
         * @param xmlWriter
         *    Writer that generates XML.
         * @throws
         *    XmlException if there is an error.
         */
        virtual void writeAsXML(XmlWriter& xmlWriter) const throw (XmlException) = 0;
        
        /** Attribute name for 'name' in XML */
        static const AString XML_ATTRIBUTE_NAME;
        
        /** Attribute name for 'data type' in XML */
        static const AString XML_ATTRIBUTE_DATA_TYPE;
        
    protected:
        SceneObject(const QString& name,
                    const SceneObjectDataTypeEnum::Enum dataType);
        
    private:
        SceneObject(const SceneObject&);

        SceneObject& operator=(const SceneObject&);
        
    public:
        virtual AString toString() const;

        // ADD_NEW_METHODS_HERE
        
    private:
        
        // ADD_NEW_MEMBERS_HERE

        /** Name of the item*/
        const QString m_name;
        
        /** Type of object */
        const SceneObjectDataTypeEnum::Enum m_dataType;
        
    };
    
#ifdef __SCENE_OBJECT_DECLARE__
    const AString SceneObject::XML_ATTRIBUTE_NAME = "name";
    const AString SceneObject::XML_ATTRIBUTE_DATA_TYPE = "dataType";
#endif // __SCENE_OBJECT_DECLARE__

} // namespace
#endif  //__SCENE_OBJECT__H_
