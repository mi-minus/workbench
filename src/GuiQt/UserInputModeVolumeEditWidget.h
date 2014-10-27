#ifndef __USER_INPUT_MODE_VOLUME_EDIT_WIDGET_H__
#define __USER_INPUT_MODE_VOLUME_EDIT_WIDGET_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2014 Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/


#include <QWidget>

#include "UserInputModeVolumeEdit.h"
#include "VolumeEditingModeEnum.h"

class QActionGroup;
class QDoubleSpinBox;
class QSpinBox;
class QToolButton;

namespace caret {

    class UserInputModeVolumeEdit;
    
    class UserInputModeVolumeEditWidget : public QWidget {
        
        Q_OBJECT

    public:
        UserInputModeVolumeEditWidget(UserInputModeVolumeEdit* inputModeVolumeEdit,
                                      const int32_t windowIndex,
                                      QWidget* parent = 0);
        
        virtual ~UserInputModeVolumeEditWidget();
        
        void updateWidget();
        
        void getEditingParameters(VolumeEditingModeEnum::Enum& editingModeOut,
                                  int32_t brushSizesOut[3],
                                  float& valueOut) const;
        
        VolumeEditingModeEnum::Enum getEditingMode() const;
        
        // ADD_NEW_METHODS_HERE

    private slots:
        void newFileActionTriggered();
        
        void lockFileActionTriggered();
        
        void undoActionTriggered();
        
        void redoActionTriggered();
        
        void resetActionTriggered();
        
        void xBrushSizeValueChanged(int);
        
        void yBrushSizeValueChanged(int);
        
        void zBrushSizeValueChanged(int);
        
        void voxelValueChanged(double);
        
        void editingModeActionTriggered(QAction*);
        
    private:
        UserInputModeVolumeEditWidget(const UserInputModeVolumeEditWidget&);

        UserInputModeVolumeEditWidget& operator=(const UserInputModeVolumeEditWidget&);
        
        QWidget* createSelectionToolBar();
        
        QWidget* createModeToolBar();
        
        UserInputModeVolumeEdit* m_inputModeVolumeEdit;
        
        const int32_t m_windowIndex;
        
        QActionGroup* m_volumeEditModeActionGroup;
        
        QToolButton* m_newFileToolButton;
        
        QSpinBox* m_xBrushSizeSpinBox;
        
        QSpinBox* m_yBrushSizeSpinBox;
        
        QSpinBox* m_zBrushSizeSpinBox;
        
        QDoubleSpinBox* m_voxelValueSpinBox;
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __USER_INPUT_MODE_VOLUME_EDIT_WIDGET_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __USER_INPUT_MODE_VOLUME_EDIT_WIDGET_DECLARE__

} // namespace
#endif  //__USER_INPUT_MODE_VOLUME_EDIT_WIDGET_H__