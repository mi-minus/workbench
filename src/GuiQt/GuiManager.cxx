
/*LICENSE_START*/
/*
 *  Copyright (C) 2014  Washington University School of Medicine
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

#include <algorithm>
#include <cmath>
#include <list>

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>
#include <QPushButton>

#define __GUI_MANAGER_DEFINE__
#include "GuiManager.h"
#undef __GUI_MANAGER_DEFINE__

#include "AnnotationFile.h"
#include "Brain.h"
#include "BrainBrowserWindow.h"
#include "BrainOpenGL.h"
#include "BrainStructure.h"
#include "BrowserTabContent.h"
#include "BugReportDialog.h"
#include "CaretAssert.h"
#include "CaretLogger.h"
#include "CaretMappableDataFile.h"
#include "ChartingDataManager.h"
#include "CiftiConnectivityMatrixDataFileManager.h"
#include "CiftiFiberTrajectoryManager.h"
#include "CiftiConnectivityMatrixParcelFile.h"
#include "CiftiScalarDataSeriesFile.h"
#include "ClippingPlanesDialog.h"
#include "CursorDisplayScoped.h"
#include "CursorManager.h"
#include "CustomViewDialog.h"
#include "DataFileException.h"
#include "ElapsedTimer.h"
#include "EventAlertUser.h"
#include "EventAnnotationGetDrawnInWindow.h"
#include "EventBrowserTabGetAll.h"
#include "EventBrowserWindowNew.h"
#include "EventGraphicsUpdateAllWindows.h"
#include "EventGraphicsUpdateOneWindow.h"
#include "EventHelpViewerDisplay.h"
#include "EventIdentificationHighlightLocation.h"
#include "EventMacDockMenuUpdate.h"
#include "EventManager.h"
#include "EventMapYokingSelectMap.h"
#include "EventModelGetAll.h"
#include "EventOperatingSystemRequestOpenDataFile.h"
#include "EventOverlaySettingsEditorDialogRequest.h"
#include "EventPaletteColorMappingEditorDialogRequest.h"
#include "EventProgressUpdate.h"
#include "EventSurfaceColoringInvalidate.h"
#include "EventUpdateInformationWindows.h"
#include "EventUserInterfaceUpdate.h"
#include "FociPropertiesEditorDialog.h"
#include "GapsAndMarginsDialog.h"
#include "HelpViewerDialog.h"
#include "IdentifiedItemNode.h"
#include "IdentifiedItemVoxel.h"
#include "IdentificationManager.h"
#include "IdentificationStringBuilder.h"
#include "IdentifyBrainordinateDialog.h"
#include "ImageFile.h"
#include "ImageCaptureDialog.h"
#include "InformationDisplayDialog.h"
#include "OverlaySettingsEditorDialog.h"
#include "MacDockMenu.h"
#include "MovieDialog.h"
#include "PaletteColorMappingEditorDialog.h"
#include "PreferencesDialog.h"
#include "SceneAttributes.h"
#include "SceneClass.h"
#include "SceneClassArray.h"
#include "SceneDialog.h"
#include "SceneWindowGeometry.h"
#include "SelectionManager.h"
#include "SelectionItemChartMatrix.h"
#include "SelectionItemCiftiConnectivityMatrixRowColumn.h"
#include "SelectionItemSurfaceNode.h"
#include "SelectionItemSurfaceNodeIdentificationSymbol.h"
#include "SelectionItemVoxel.h"
#include "SelectionItemVoxelIdentificationSymbol.h"
#include "SessionManager.h"
#include "SpecFile.h"
#include "SpecFileManagementDialog.h"
#include "SurfacePropertiesEditorDialog.h"
#include "Surface.h"
#include "TileTabsConfigurationDialog.h"
#include "VolumeMappableInterface.h"
#include "WuQMessageBox.h"
//#include "WuQWebView.h"
#include "WuQtUtilities.h"

#include "CaretAssert.h"

using namespace caret;

/**
 * \defgroup GuiQt
 */
/**
 * \class caret::GuiManager
 * \brief Top level class for passing events to Gui widgets belonging to workbench window
 * \ingroup GuiQt
 */

/**
 * Constructor.
 * @param parent
 *   Parent of this object.
 */
GuiManager::GuiManager(QObject* parent)
: QObject(parent)
{
    /*
     * This constructor should be called only once.
     * When the first instance of GuiManager is created,
     * singletonGuiManager will be NULL.
     */
    CaretAssertMessage((GuiManager::singletonGuiManager == NULL),
                       "There should never be more than one instance of GuiManager.");
}

/**
 * Initialize the GUI manager.
 *
 * NOTE: This method is NOT called from the constructor.
 * If there are problems loading some of the images, there will
 * be calls to GuiManager::get() which results in recursive calls.
 */
void
GuiManager::initializeGuiManager()
{
    this->allowBrowserWindowsToCloseWithoutConfirmation = false;
    
    m_bugReportDialog = NULL;
    m_clippingPlanesDialog = NULL;
    m_customViewDialog = NULL;
    m_gapsAndMarginsDialog = NULL;
    this->imageCaptureDialog = NULL;
    this->movieDialog = NULL;
    m_informationDisplayDialog = NULL;
    m_identifyBrainordinateDialog = NULL;
    this->preferencesDialog = NULL;  
    this->connectomeDatabaseWebView = NULL;
    m_helpViewerDialog = NULL;
    m_paletteColorMappingEditor = NULL;
    this->sceneDialog = NULL;
    m_surfacePropertiesEditorDialog = NULL;
    m_tileTabsConfigurationDialog = NULL;
    
    this->cursorManager = new CursorManager();
    
    /*
     * Information window.
     */
    QIcon infoDisplayIcon;
    const bool infoDisplayIconValid =
    WuQtUtilities::loadIcon(":/ToolBar/info.png", 
                            infoDisplayIcon);
    

    m_informationDisplayDialogEnabledAction =
    WuQtUtilities::createAction("Information...",
                                "Enables display of the Information Window\n"
                                "when new information is available",
                                this,
                                this,
                                SLOT(showHideInfoWindowSelected(bool)));
    if (infoDisplayIconValid) {
        m_informationDisplayDialogEnabledAction->setIcon(infoDisplayIcon);
        m_informationDisplayDialogEnabledAction->setIconVisibleInMenu(false);
    }
    else {
        m_informationDisplayDialogEnabledAction->setIconText("Info");
    }
    
    m_informationDisplayDialogEnabledAction->blockSignals(true);
    m_informationDisplayDialogEnabledAction->setCheckable(true);
    m_informationDisplayDialogEnabledAction->setChecked(true);
    this->showHideInfoWindowSelected(m_informationDisplayDialogEnabledAction->isChecked());
    m_informationDisplayDialogEnabledAction->setIconText("Info"); 
    m_informationDisplayDialogEnabledAction->blockSignals(false);
    
    /*
     * Identify brainordinate window
     */
    QIcon identifyDisplayIcon;
    const bool identifyDisplayIconValid =
    WuQtUtilities::loadIcon(":/ToolBar/identify.png",
                            identifyDisplayIcon);
    m_identifyBrainordinateDialogEnabledAction =
    WuQtUtilities::createAction("Identify...",
                                "Enables display of the Identify Brainordinate Window",
                                this,
                                this,
                                SLOT(showIdentifyBrainordinateDialogActionToggled(bool)));
    if (identifyDisplayIconValid) {
        m_identifyBrainordinateDialogEnabledAction->setIcon(identifyDisplayIcon);
        m_identifyBrainordinateDialogEnabledAction->setIconVisibleInMenu(false);
    }
    else {
        m_identifyBrainordinateDialogEnabledAction->setIconText("ID");
    }
    
    m_identifyBrainordinateDialogEnabledAction->blockSignals(true);
    m_identifyBrainordinateDialogEnabledAction->setCheckable(true);
    m_identifyBrainordinateDialogEnabledAction->setChecked(false);
    m_identifyBrainordinateDialogEnabledAction->blockSignals(false);
    
    /*
     * Scene dialog action
     */
    m_sceneDialogDisplayAction = WuQtUtilities::createAction("Scenes...",
                                                             "Show/Hide the Scenes Window",
                                                             this,
                                                             this,
                                                             SLOT(sceneDialogDisplayActionToggled(bool)));
    QIcon clapBoardIcon;
    const bool clapBoardIconValid = WuQtUtilities::loadIcon(":/ToolBar/clapboard.png",
                                                            clapBoardIcon);
    if (clapBoardIconValid) {
        m_sceneDialogDisplayAction->setIcon(clapBoardIcon);
        m_sceneDialogDisplayAction->setIconVisibleInMenu(false);
    }
    else {
        m_sceneDialogDisplayAction->setIconText("Scenes");
    }
    m_sceneDialogDisplayAction->blockSignals(true);
    m_sceneDialogDisplayAction->setCheckable(true);
    m_sceneDialogDisplayAction->setChecked(false);
    m_sceneDialogDisplayAction->blockSignals(false);
    
    /*
     * Help dialog action
     */
    m_helpViewerDialogDisplayAction = WuQtUtilities::createAction("Workbench Help...",
                                                                  "Show/Hide the Help Window",
                                                                  QKeySequence::HelpContents,
                                                                  this,
                                                                  this,
                                                                  SLOT(showHelpDialogActionToggled(bool)));
    QIcon helpIcon;
    const bool helpIconValid = WuQtUtilities::loadIcon(":/ToolBar/help.png",
                                                       helpIcon);
    if (helpIconValid) {
        m_helpViewerDialogDisplayAction->setIcon(helpIcon);
        m_helpViewerDialogDisplayAction->setIconVisibleInMenu(false);
    }
    else {
        m_helpViewerDialogDisplayAction->setIconText("?");
    }
    m_helpViewerDialogDisplayAction->blockSignals(true);
    m_helpViewerDialogDisplayAction->setCheckable(true);
    m_helpViewerDialogDisplayAction->setChecked(false);
    m_helpViewerDialogDisplayAction->blockSignals(false);
    
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_ALERT_USER);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_ANNOTATION_GET_DRAWN_IN_WINDOW);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_BROWSER_WINDOW_NEW);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_HELP_VIEWER_DISPLAY);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_MAC_DOCK_MENU_UPDATE);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_OVERLAY_SETTINGS_EDITOR_SHOW);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_OPERATING_SYSTEM_REQUEST_OPEN_DATA_FILE);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_PALETTE_COLOR_MAPPING_EDITOR_SHOW);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_UPDATE_INFORMATION_WINDOWS);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_USER_INTERFACE_UPDATE);
}

/**
 * Destructor.
 */
GuiManager::~GuiManager()
{
    EventManager::get()->removeAllEventsFromListener(this);
    
    delete this->cursorManager;
    
    if (this->connectomeDatabaseWebView != NULL) {
        CaretAssertMessage(0, "Need to uncomment out line below if webkit is renabled");
        //delete this->connectomeDatabaseWebView;
    }
    
    FociPropertiesEditorDialog::deleteStaticMembers();
    
    for (std::set<QWidget*>::iterator iter = m_parentlessNonModalDialogs.begin();
         iter != m_parentlessNonModalDialogs.end();
         iter++) {
        QWidget* w = *iter;
        delete w;
    }
    m_parentlessNonModalDialogs.clear();
}

/**
 * Get the GUI Manager.
 */
GuiManager* 
GuiManager::get()
{
    CaretAssertMessage((GuiManager::singletonGuiManager != NULL),
                       "GuiManager::singletonGuiManager has not been created.  Must call GuiManager::createGuiManager()");

    return GuiManager::singletonGuiManager;
}

/*
 * Create the singleton GUI Manager.
 */
void 
GuiManager::createGuiManager()
{
    CaretAssertMessage((GuiManager::singletonGuiManager == NULL),
                       "GUI manager has already been created.");
    
    GuiManager::singletonGuiManager = new GuiManager();
    GuiManager::singletonGuiManager->initializeGuiManager();
    WuQtUtilities::sendListOfResourcesToCaretLogger();
}

/*
 * Delete the singleton GUI Manager.
 */
void 
GuiManager::deleteGuiManager()
{
    CaretAssertMessage((GuiManager::singletonGuiManager != NULL), 
                       "GUI manager does not exist, cannot delete it.");
    
    delete GuiManager::singletonGuiManager;
    GuiManager::singletonGuiManager = NULL;
}

/**
 * Beep to alert the user.
 */
void 
GuiManager::beep()
{
    QApplication::beep();
}

/**
 * @return The brain.
 */
Brain* 
GuiManager::getBrain() const
{
    return SessionManager::get()->getBrain(0);
}

/**
 * Get the Brain OpenGL for drawing with OpenGL.
 *
 * @return 
 *    Point to the brain.
 */
//BrainOpenGL* 
//GuiManager::getBrainOpenGL()
//{
//    if (this->brainOpenGL == NULL) {
//        this->brainOpenGL = BrainOpenGL::getBrainOpenGL();
//    }
//    
//    return this->brainOpenGL;
//}

/**
 * See if a brain browser window can be closed.  If there is only
 * one brain browser window, the user will be warned that any 
 * changes to files will be lost and the application will exit.
 * If there is more than one brain browser window open and the 
 * window being closed contains more than one tab, the user will
 * be warned.
 *
 * @param brainBrowserWindow
 *   Brain browser window that will be closed.
 * @param numberOfOpenTabs
 *   Number of tabs in window.
 * @return 
 *   True if window should be closed, else false.
 */
bool 
GuiManager::allowBrainBrowserWindowToClose(BrainBrowserWindow* brainBrowserWindow,
                                           const int32_t numberOfOpenTabs)
{
    bool isBrowserWindowAllowedToClose = false;

    if (this->allowBrowserWindowsToCloseWithoutConfirmation) {
        isBrowserWindowAllowedToClose = true;
    }
    else {
        if (this->getNumberOfOpenBrainBrowserWindows() > 1) {
            /*
             * Warn if multiple tabs in window
             */
            if (numberOfOpenTabs > 1) {
                QString tabMessage = QString::number(numberOfOpenTabs) + " tabs are open.";
                isBrowserWindowAllowedToClose =
                WuQMessageBox::warningCloseCancel(brainBrowserWindow, 
                                                  "Are you sure you want to close this window?", 
                                                  tabMessage);
            }
            else {
                isBrowserWindowAllowedToClose = true;
            }
        }
        else {
            isBrowserWindowAllowedToClose = this->exitProgram(brainBrowserWindow);
        }
    }
    
    if (isBrowserWindowAllowedToClose) {
        for (int32_t i = 0; i < static_cast<int32_t>(m_brainBrowserWindows.size()); i++) {
            if (m_brainBrowserWindows[i] == brainBrowserWindow) {
                m_brainBrowserWindows[i] = NULL; // must set to NULL BEFORE reparenting non-modal dialogs so they dont' see it
                this->reparentNonModalDialogs(brainBrowserWindow);
            }
        }
        
        EventManager::get()->sendEvent(EventMacDockMenuUpdate().getPointer());
    }
    
    return isBrowserWindowAllowedToClose;
}

/**
 * Get the number of brain browser windows.
 *
 * @return Number of brain browser windows that are valid.
 */
int32_t 
GuiManager::getNumberOfOpenBrainBrowserWindows() const
{
    int32_t numberOfWindows = 0;
    for (int32_t i = 0; i < static_cast<int32_t>(m_brainBrowserWindows.size()); i++) {
        if (m_brainBrowserWindows[i] != NULL) {
            numberOfWindows++;
        }
    }
    return numberOfWindows;
}


/**
 * Get all of the brain browser windows.
 *
 * @return 
 *   Vector containing all open brain browser windows.
 */
std::vector<BrainBrowserWindow*> 
GuiManager::getAllOpenBrainBrowserWindows() const
{ 
    std::vector<BrainBrowserWindow*> windows;
    
    int32_t numWindows = static_cast<int32_t>(m_brainBrowserWindows.size());
    for (int32_t i = 0; i < numWindows; i++) {
        if (m_brainBrowserWindows[i] != NULL) {
            windows.push_back(m_brainBrowserWindows[i]);
        }
    }
    
    return windows; 
}

/**
 * Get all of the brain browser window indices
 *
 * @return
 *   Vector containing all open brain browser window indices.
 */
std::vector<int32_t>
GuiManager::getAllOpenBrainBrowserWindowIndices() const
{
    std::vector<int32_t> windowIndices;
    
    int32_t numWindows = static_cast<int32_t>(m_brainBrowserWindows.size());
    for (int32_t i = 0; i < numWindows; i++) {
        if (m_brainBrowserWindows[i] != NULL) {
            windowIndices.push_back(i);
        }
    }
    
    return windowIndices;
}

/**
 * @return Return the active browser window.  If no browser window is active,
 * the browser window with the lowest index is returned.  If no browser
 * window is open (which likely should never occur), NULL is returned.
 *
 * To verify that the returned window was the active window, call its
 * "isActiveWindow()" method.
 */
BrainBrowserWindow* 
GuiManager::getActiveBrowserWindow() const
{
    BrainBrowserWindow* firstWindowFound = NULL;
    int32_t numWindows = static_cast<int32_t>(m_brainBrowserWindows.size());
    for (int32_t i = 0; i < numWindows; i++) {
        BrainBrowserWindow* bbw = m_brainBrowserWindows[i];
        if (bbw != NULL) {
            if (firstWindowFound == NULL) {
                firstWindowFound = bbw;
            }
            if (bbw->isActiveWindow()) {
                return bbw;
            }
        }
    }
    
    return firstWindowFound;
}


/**
 * Get the brain browser window with the given window index.
 * Note that as browser windows are opened or closed, a window's
 * index NEVER changes.  Thus, a NULL value may be returned for 
 * a window index referring to a window that was closed.
 *
 * @param browserWindowIndex
 *    Index of the window.
 * @return
 *    Pointer to window at given index or NULL in cases where
 *    the window was closed.
 */
BrainBrowserWindow* 
GuiManager::getBrowserWindowByWindowIndex(const int32_t browserWindowIndex)
{
    if (browserWindowIndex < static_cast<int32_t>(m_brainBrowserWindows.size())) {
        return m_brainBrowserWindows[browserWindowIndex];
    }
    return NULL;
}

/**
 * Create a new BrainBrowser Window.
 * @param parent
 *    Optional parent that is used only for window placement.
 * @param browserTabContent
 *    Optional tab for initial windwo tab.
 * @param createDefaultTabs
 *    If true, create the default tabs in the new window.
 */
BrainBrowserWindow*
GuiManager::newBrainBrowserWindow(QWidget* parent,
                                  BrowserTabContent* browserTabContent,
                                  const bool createDefaultTabs)
{
    /*
     * If no tabs can be created, do not create a new window.
     */
    EventBrowserTabGetAll getAllTabs;
    EventManager::get()->sendEvent(getAllTabs.getPointer());
    if (getAllTabs.getNumberOfBrowserTabs() == BrainConstants::MAXIMUM_NUMBER_OF_BROWSER_TABS) {
        return NULL;
    }
    
    int32_t windowIndex = -1;
    
    int32_t numWindows = static_cast<int32_t>(m_brainBrowserWindows.size());
    for (int32_t i = 0; i < numWindows; i++) {
        if (m_brainBrowserWindows[i] == NULL) {
            windowIndex = i;
            break;
        }
    }
    
    BrainBrowserWindow* bbw = NULL; 
    
    BrainBrowserWindow::CreateDefaultTabsMode tabsMode = (createDefaultTabs
                                                          ? BrainBrowserWindow::CREATE_DEFAULT_TABS_YES
                                                          : BrainBrowserWindow::CREATE_DEFAULT_TABS_NO);
    
    if (windowIndex < 0) {
        windowIndex = m_brainBrowserWindows.size();
        bbw = new BrainBrowserWindow(windowIndex, browserTabContent, tabsMode);
        m_brainBrowserWindows.push_back(bbw);
    }
    else {
        bbw = new BrainBrowserWindow(windowIndex, browserTabContent, tabsMode);
        m_brainBrowserWindows[windowIndex] = bbw;
    }
    
    if (parent != NULL) {
        WuQtUtilities::moveWindowToOffset(parent, bbw, 20, 20);
    }
    
    bbw->show();
    
    bbw->resetGraphicsWidgetMinimumSize();
    
    return bbw;
}

/**
 * Test for modified data files.
 *
 * @param testModifiedMode
 *    Mode of testing for modified files.
 * @param textMessageOut
 *    Message displayed at top of dialog.
 * @param modifiedFilesMessageOut
 *    If there are any modified files, this will contain information
 *    about the modified files.
 * @return
 *    True if there are modified files and the warning message is valid,
 *    else false.
 */
bool
GuiManager::testForModifiedFiles(const TestModifiedMode testModifiedMode,
                                 AString& textMessageOut,
                                 AString& modifiedFilesMessageOut) const
{
    textMessageOut.clear();
    modifiedFilesMessageOut.clear();
    
    /*
     * Exclude all
     *   Connectivity Files
     */
    std::vector<DataFileTypeEnum::Enum> dataFileTypesToExclude;
    dataFileTypesToExclude.push_back(DataFileTypeEnum::CONNECTIVITY_DENSE);
    dataFileTypesToExclude.push_back(DataFileTypeEnum::CONNECTIVITY_DENSE_DYNAMIC);
    dataFileTypesToExclude.push_back(DataFileTypeEnum::CONNECTIVITY_FIBER_ORIENTATIONS_TEMPORARY);
    dataFileTypesToExclude.push_back(DataFileTypeEnum::CONNECTIVITY_FIBER_TRAJECTORY_TEMPORARY);
    
    switch (testModifiedMode) {
        case TEST_FOR_MODIFIED_FILES_MODE_FOR_EXIT:
            break;
        case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_ADD:
            dataFileTypesToExclude.push_back(DataFileTypeEnum::SCENE);
            dataFileTypesToExclude.push_back(DataFileTypeEnum::SPECIFICATION);
            break;
        case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_SHOW:
            dataFileTypesToExclude.push_back(DataFileTypeEnum::SCENE);
            break;
    }
    
    /*
     * Are files modified?
     */
    std::vector<CaretDataFile*> allModifiedDataFiles;
    getBrain()->getAllModifiedFiles(dataFileTypesToExclude,
                                    allModifiedDataFiles);
    
    std::vector<CaretDataFile*> modifiedDataFiles;
    std::vector<CaretDataFile*> paletteModifiedDataFiles;
    
    for (std::vector<CaretDataFile*>::iterator allModFilesIter = allModifiedDataFiles.begin();
         allModFilesIter != allModifiedDataFiles.end();
         allModFilesIter++) {
        CaretDataFile* file = *allModFilesIter;
        CaretAssert(file);
        
        switch (testModifiedMode) {
            case TEST_FOR_MODIFIED_FILES_MODE_FOR_EXIT:
                modifiedDataFiles.push_back(file);
                break;
            case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_ADD:
            {
                /*
                 * Is modification just the palette color mapping?
                 */
                CaretMappableDataFile* mappableDataFile = dynamic_cast<CaretMappableDataFile*>(file);
                bool paletteOnlyModFlag = false;
                if (mappableDataFile != NULL) {
                    if (mappableDataFile->isModifiedPaletteColorMapping()) {
                        if ( ! mappableDataFile->isModifiedExcludingPaletteColorMapping()) {
                            paletteOnlyModFlag = true;
                        }
                    }
                }
                if (paletteOnlyModFlag) {
                    paletteModifiedDataFiles.push_back(file);
                }
                else {
                    modifiedDataFiles.push_back(file);
                }
                
            }
                break;
            case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_SHOW:
                modifiedDataFiles.push_back(file);
                break;
        }
    }
    
    const int32_t modFileCount = static_cast<int32_t>(modifiedDataFiles.size());
    const int32_t paletteModFileCount = static_cast<int32_t>(paletteModifiedDataFiles.size());
    
    /*
     * Are there scene annotations ?
     */
    const CaretDataFile* sceneAnnotationFile = getBrain()->getSceneAnnotationFile();
    bool sceneAnnotationsModifiedFlag = sceneAnnotationFile->isModified();
    switch (testModifiedMode) {
        case TEST_FOR_MODIFIED_FILES_MODE_FOR_EXIT:
            break;
        case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_ADD:
            /*
             * Do not need to notify about modified scene annotations
             * since scene annotations are saved to the scene
             */
            sceneAnnotationsModifiedFlag = false;
            break;
        case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_SHOW:
            break;
    }
    
    if ((modFileCount > 0)
        || sceneAnnotationsModifiedFlag
        || paletteModFileCount) {
        /*
         * Display dialog allowing user to save files (goes to Save/Manage
         * Files dialog), exit without saving, or cancel.
         */
        switch (testModifiedMode) {
            case TEST_FOR_MODIFIED_FILES_MODE_FOR_EXIT:
                textMessageOut = "Do you want to save changes you made to these files?";
                break;
            case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_ADD:
                textMessageOut = "Do you want to continue creating the scene?";
                break;
            case TEST_FOR_MODIFIED_FILES_MODE_FOR_SCENE_SHOW:
                textMessageOut = "Do you want to continue showing the scene?";
                break;
        }
        
        AString infoTextMsg;
        if (modFileCount > 0) {
            infoTextMsg.appendWithNewLine("Changes to these files will be lost if you don't save them:\n");
            for (std::vector<CaretDataFile*>::iterator iter = modifiedDataFiles.begin();
                 iter != modifiedDataFiles.end();
                 iter++) {
                const CaretDataFile* cdf = *iter;
                infoTextMsg.appendWithNewLine("   "
                                              + cdf->getFileNameNoPath());
            }
            infoTextMsg.append("\n");
        }
        if (paletteModFileCount > 0) {
            infoTextMsg.appendWithNewLine("These file(s) contain modified palette color mapping.  It is not "
                                          "necessary to save these file(s) if the save palette color mapping "
                                          "option is selected on the scene creation dialog:\n");
            for (std::vector<CaretDataFile*>::iterator iter = paletteModifiedDataFiles.begin();
                 iter != paletteModifiedDataFiles.end();
                 iter++) {
                const CaretDataFile* cdf = *iter;
                infoTextMsg.appendWithNewLine("   "
                                              + cdf->getFileNameNoPath());
            }
            infoTextMsg.append("\n");
        }
        
        if (sceneAnnotationsModifiedFlag) {
            infoTextMsg.appendWithNewLine("Scene annotations are modified.");
        }
        
        modifiedFilesMessageOut = infoTextMsg;
        return true;
    }
    
    return false;
}

/**
 * Exit the program.
 * @param
 *    Parent over which dialogs are displayed for saving/verifying.
 * return 
 *    true if application should exit, else false.
 */
bool 
GuiManager::exitProgram(BrainBrowserWindow* parent)
{
    bool okToExit = false;
    
    AString textMessage;
    AString modifiedFilesMessage;
    if (testForModifiedFiles(TEST_FOR_MODIFIED_FILES_MODE_FOR_EXIT,
                             textMessage,
                             modifiedFilesMessage)) {
        modifiedFilesMessage.appendWithNewLine("");
        
        QMessageBox quitDialog(QMessageBox::Warning,
                               "Exit Workbench",
                               textMessage,
                               QMessageBox::NoButton,
                               parent);
        quitDialog.setInformativeText(modifiedFilesMessage);
        
        QPushButton* saveButton = quitDialog.addButton("Save...", QMessageBox::AcceptRole);
        saveButton->setToolTip("Display manage files window to save files");
        
        QPushButton* dontSaveButton = quitDialog.addButton("Don't Save", QMessageBox::DestructiveRole);
        dontSaveButton->setToolTip("Do not save changes and exit.");
        
        QPushButton* cancelButton = quitDialog.addButton("Cancel", QMessageBox::RejectRole);
        
        quitDialog.setDefaultButton(saveButton);
        quitDialog.setEscapeButton(cancelButton);
        
        quitDialog.exec();
        const QAbstractButton* clickedButton = quitDialog.clickedButton();
        if (clickedButton == saveButton) {
            if (SpecFileManagementDialog::runSaveFilesDialogWhileQuittingWorkbench(this->getBrain(),
                                                                                   parent)) {
                okToExit = true;
                
            }
        }
        else if (clickedButton == dontSaveButton) {
            okToExit = true;
        }
        else if (clickedButton == cancelButton) {
            /* nothing */
        }
        else {
            CaretAssert(0);
        }
    }
    else {
        const AString textMsg("Exit Workbench?");

        QMessageBox quitDialog(QMessageBox::Warning,
                               "Exit Workbench",
                               textMsg,
                               QMessageBox::NoButton,
                               parent);
        if (SceneDialog::isInformUserAboutScenesOnExit()) {
            const AString infoTextMsg("<html>Would you like to save your Workbench windows in scene file "
                                      "so you can easily pick up where you left off?"
                                      "<p>"
                                      "Click the <B>Show Details</B> button for "
                                      "more information.</html>");
            const AString detailTextMsg("Scenes allow one to regenerate exactly what is displayed in "
                                        "Workbench.  This can be useful in these and other situations:"
                                        "\n\n"
                                        " * During manuscript preparation to restore Workbench to match "
                                        "a previously generated figure (image capture)."
                                        "\n\n"
                                        " * When returning to this dataset for further analysis."
                                        "\n\n"
                                        " * When sharing data sets with others to provide a particular "
                                        "view of a surface/volume with desired data (overlay and feature) "
                                        "selections.");
            quitDialog.setInformativeText(infoTextMsg);
            quitDialog.setDetailedText(detailTextMsg);
        }
        
        QPushButton* exitButton = quitDialog.addButton("Exit",
                                                       QMessageBox::AcceptRole);
        
        QPushButton* cancelButton = quitDialog.addButton("Cancel",
                                                         QMessageBox::RejectRole);
        
        quitDialog.setDefaultButton(exitButton);
        quitDialog.setEscapeButton(cancelButton);
        
        quitDialog.exec();
        const QAbstractButton* clickedButton = quitDialog.clickedButton();
        if (clickedButton  == exitButton) {
            okToExit = true;
        }
        else if (clickedButton == cancelButton) {
            /* Nothing */
        }
        else {
            CaretAssert(0);
        }
    }
    
    if (okToExit) {
        std::vector<BrainBrowserWindow*> bws = this->getAllOpenBrainBrowserWindows();
        for (int i = 0; i < static_cast<int>(bws.size()); i++) {
            bws[i]->deleteLater();
        }
        
        QCoreApplication::instance()->quit();
    }    
    
    return okToExit;
}

/**
 * Show the Open Spec File Dialog with the given spec file.
 *
 * @param specFile
 *    SpecFile displayed in the dialog.
 * @param browserWindow
 *    Window on which dialog is displayed.
 * @return
 *    True if user opened spec file, else false.
 */
bool
GuiManager::processShowOpenSpecFileDialog(SpecFile* specFile,
                                          BrainBrowserWindow* browserWindow)
{
    return SpecFileManagementDialog::runOpenSpecFileDialog(getBrain(),
                                                           specFile,
                                                           browserWindow);
}

/**
 * Show the Save/Manage Files Dialog.
 *
 * @param browserWindow
 *    Window on which dialog is displayed.
 */
void
GuiManager::processShowSaveManageFilesDialog(BrainBrowserWindow* browserWindow)
{
    SpecFileManagementDialog::runManageFilesDialog(getBrain(),
                                                   browserWindow);
}

/**
 * Get the model displayed in the given browser window.
 * @param browserWindowIndex
 *    Index of browser window.
 * @return
 *    Model in the browser window.  May be NULL if no data loaded.
 */
Model*
GuiManager::getModelInBrowserWindow(const int32_t browserWindowIndex)
{
    BrowserTabContent* browserTabContent = getBrowserTabContentForBrowserWindow(browserWindowIndex,
                                                                                true);
    Model* model = NULL;
    if (browserTabContent != NULL) {
        model = browserTabContent->getModelForDisplay();
    }
    return model;
}


/**
 * Get the browser tab content in a browser window.
 * @param browserWindowIndex
 *    Index of browser window.
 * @param allowInvalidBrowserWindowIndex
 *    In some instance, such as GUI construction or destruction, the window is not
 *    fully created or deleted, thus "this->brainBrowserWindows" is invalid for
 *    the given index.  If this parameter is true, NULL will be 
 *    returned in this case.
 * @return
 *    Browser tab content in the browser window.  Value may be NULL
 *    is allowInvalidBrowserWindowIndex is true
 */
BrowserTabContent* 
GuiManager::getBrowserTabContentForBrowserWindow(const int32_t browserWindowIndex,
                                                 const bool allowInvalidBrowserWindowIndex)
{
    if (allowInvalidBrowserWindowIndex) {
        if (browserWindowIndex >= static_cast<int32_t>(m_brainBrowserWindows.size())) {
            return NULL;
        }
    }
    
    CaretAssertVectorIndex(m_brainBrowserWindows, browserWindowIndex);
    BrainBrowserWindow* browserWindow = m_brainBrowserWindows[browserWindowIndex];
    if (allowInvalidBrowserWindowIndex) {
        if (browserWindow == NULL) {
            return NULL;
        }
    }
    CaretAssert(browserWindow);
    
    BrowserTabContent* tabContent = browserWindow->getBrowserTabContent();
    return tabContent;
}

/**
 * Called when bring all windows to front is selected.
 */
void 
GuiManager::processBringAllWindowsToFront()
{
    for (int32_t i = 0; i < static_cast<int32_t>(m_brainBrowserWindows.size()); i++) {
        if (m_brainBrowserWindows[i] != NULL) {
            m_brainBrowserWindows[i]->show();
            m_brainBrowserWindows[i]->activateWindow();
        }
    }
    
    for (std::set<QWidget*>::iterator iter = this->nonModalDialogs.begin();
         iter != this->nonModalDialogs.end();
         iter++) {
        QWidget* w = *iter;
        if (w->isVisible()) {
            w->raise();
        }
    }
    
    for (std::set<QWidget*>::iterator iter = m_parentlessNonModalDialogs.begin();
         iter != m_parentlessNonModalDialogs.end();
         iter++) {
        QWidget* w = *iter;
        if (w->isVisible()) {
            w->raise();
        }
    }
}

/**
 * Called to tile the windows (arrange browser windows in a grid).
 */
void GuiManager::processTileWindows()
{
    std::vector<BrainBrowserWindow*> windows = getAllOpenBrainBrowserWindows();
    const int32_t numWindows = static_cast<int32_t>(windows.size());
    if (numWindows <= 1) {
        return;
    }
    
    QDesktopWidget* dw = QApplication::desktop();
    const int32_t numScreens = dw->screenCount();
    const int32_t windowsPerScreen = std::max(numWindows / numScreens,
                                              1);
    
    /**
     * Determine the number of rows and columns for the montage.
     * Since screen width typically exceeds height, always have
     * columns greater than or equal to rows.
     */
    int32_t numRows = (int)std::sqrt((double)windowsPerScreen);
    int32_t numCols = numRows;
    int32_t row2 = numRows * numRows;
    if (row2 < numWindows) {
        numCols++;
    }
    if ((numRows * numCols) < numWindows) {
        numRows++;
    }
    
    AString windowInfo("Tiled Windows");
    
    /*
     * Arrange models left-to-right and top-to-bottom.
     * Note that origin is top-left corner.
     */
    int32_t windowIndex = 0;
    for (int32_t iScreen = 0; iScreen < numScreens; iScreen++) {
        const QRect rect = dw->availableGeometry(iScreen);
        const int screenX = rect.x();
        const int screenY = rect.y();
        const int screenWidth = rect.width();
        const int screenHeight = rect.height();
        
        const int32_t windowWidth = screenWidth / numCols;
        const int32_t windowHeight = screenHeight / numRows;
        int32_t windowX = 0;
        int32_t windowY = screenY;
        
        for (int32_t iRow = 0; iRow < numRows; iRow++) {
            windowX = screenX;
            for (int32_t iCol = 0; iCol < numCols; iCol++) {
                windows[windowIndex]->setGeometry(windowX,
                                                  windowY,
                                                  windowWidth,
                                                  windowHeight);
                
                windowX += windowWidth;
                
                QString info = ("    Window: "
                                + windows[windowIndex]->windowTitle()
                                + " screen/x/y/w/h ("
                                + QString::number(iScreen)
                                + ", "
                                + QString::number(windowX)
                                + ", "
                                + QString::number(windowY)
                                + ", "
                                + QString::number(windowWidth)
                                + ", "
                                + QString::number(windowHeight)
                                + ")");
                windowInfo.appendWithNewLine(info);
                
                windowIndex++;
                if (windowIndex >= numWindows) {
                    /*
                     * No more windows to place.
                     * Containing loops would cause a crash.
                     */
                    CaretLogFine(windowInfo);
                    return;
                }
            }
            
            windowY += windowHeight;
        }
    }

    CaretLogFine(windowInfo);
}

/**
 * Receive events from the event manager.
 * 
 * @param event
 *   Event sent by event manager.
 */
void 
GuiManager::receiveEvent(Event* event)
{
    if (event->getEventType() == EventTypeEnum::EVENT_ALERT_USER) {
        EventAlertUser* alertUserEvent = dynamic_cast<EventAlertUser*>(event);
        const AString message = alertUserEvent->getMessage();
        
        BrainBrowserWindow* bbw = getActiveBrowserWindow();
        CaretAssert(bbw);
        
        WuQMessageBox::errorOk(bbw, message);
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_ANNOTATION_GET_DRAWN_IN_WINDOW) {
        EventAnnotationGetDrawnInWindow* annGetEvent = dynamic_cast<EventAnnotationGetDrawnInWindow*>(event);
        CaretAssert(annGetEvent);
        
        const int32_t windowIndex = annGetEvent->getWindowIndex();
        
        Brain* brain = getBrain();
        std::vector<AnnotationFile*> allAnnotationFiles;
        brain->getAllAnnotationFilesIncludingSceneAnnotationFile(allAnnotationFiles);
        
        /*
         * Clear "drawn in window status" for all annotations
         */
        for (std::vector<AnnotationFile*>::iterator fileIter = allAnnotationFiles.begin();
             fileIter != allAnnotationFiles.end();
             fileIter++) {
            (*fileIter)->clearAllAnnotationsDrawnInWindowStatus();
        }
        
        /*
         * Draw the given window
         */
        EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(windowIndex).getPointer());
        
        /*
         * Find annotations that were drawn in the given window.
         */
        for (std::vector<AnnotationFile*>::iterator fileIter = allAnnotationFiles.begin();
             fileIter != allAnnotationFiles.end();
             fileIter++) {
            std::vector<Annotation*> annotations;
            (*fileIter)->getAllAnnotationWithDrawnInWindowStatusSet(windowIndex,
                                                                    annotations);
            annGetEvent->addAnnotations(annotations);
        }
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_BROWSER_WINDOW_NEW) {
        EventBrowserWindowNew* eventNewBrowser =
            dynamic_cast<EventBrowserWindowNew*>(event);
        CaretAssert(eventNewBrowser);
        
        BrainBrowserWindow* bbw = this->newBrainBrowserWindow(eventNewBrowser->getParent(), 
                                                              eventNewBrowser->getBrowserTabContent(),
                                                              true);
        if (bbw == NULL) {
            eventNewBrowser->setErrorMessage("Workench is exhausted.  It cannot create any more windows.");
            eventNewBrowser->setEventProcessed();
            return;
        }
        
        eventNewBrowser->setBrowserWindowCreated(bbw);
        eventNewBrowser->setEventProcessed();
        
        /*
         * Initialize the size of the window
         */
        const int w = bbw->width();
        const int preferredMaxHeight = (WuQtUtilities::isSmallDisplay()
                                        ? 550
                                        : 850);
        const int h = std::min(bbw->height(), 
                               preferredMaxHeight);
        bbw->resize(w, h);
        
        EventManager::get()->sendEvent(EventMacDockMenuUpdate().getPointer());
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_MAC_DOCK_MENU_UPDATE) {
        EventMacDockMenuUpdate* macDockMenuEvent = dynamic_cast<EventMacDockMenuUpdate*>(event);
        CaretAssert(event);
        
        MacDockMenu::createUpdateMacDockMenu();
        
        macDockMenuEvent->setEventProcessed();
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_UPDATE_INFORMATION_WINDOWS) {
        EventUpdateInformationWindows* infoEvent =
        dynamic_cast<EventUpdateInformationWindows*>(event);
        CaretAssert(infoEvent);
        
        bool showInfoDialog = infoEvent->isImportant();
        
        if (showInfoDialog) {
            this->processShowInformationDisplayDialog(false);
        }
        
        infoEvent->setEventProcessed();
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_OVERLAY_SETTINGS_EDITOR_SHOW) {
        EventOverlaySettingsEditorDialogRequest* mapEditEvent =
        dynamic_cast<EventOverlaySettingsEditorDialogRequest*>(event);
        CaretAssert(mapEditEvent);
        
        const EventOverlaySettingsEditorDialogRequest::Mode mode = mapEditEvent->getMode();
        const int browserWindowIndex = mapEditEvent->getBrowserWindowIndex();
        CaretAssertVectorIndex(m_brainBrowserWindows, browserWindowIndex);
        BrainBrowserWindow* browserWindow = m_brainBrowserWindows[browserWindowIndex];
        CaretAssert(browserWindow);
        Overlay* overlay = mapEditEvent->getOverlay();
        
        switch (mode) {
            case EventOverlaySettingsEditorDialogRequest::MODE_OVERLAY_MAP_CHANGED:
            {
                for (std::set<OverlaySettingsEditorDialog*>::iterator overlayEditorIter = m_overlaySettingsEditors.begin();
                     overlayEditorIter != m_overlaySettingsEditors.end();
                     overlayEditorIter++) {
                    OverlaySettingsEditorDialog* med = *overlayEditorIter;
                    med->updateIfThisOverlayIsInDialog(overlay);
                }
            }
                break;
            case EventOverlaySettingsEditorDialogRequest::MODE_SHOW_EDITOR:
            {
                OverlaySettingsEditorDialog* overlayEditor = NULL;
                for (std::set<OverlaySettingsEditorDialog*>::iterator overlayEditorIter = m_overlaySettingsEditors.begin();
                     overlayEditorIter != m_overlaySettingsEditors.end();
                     overlayEditorIter++) {
                    OverlaySettingsEditorDialog* med = *overlayEditorIter;
                    if (med->isDoNotReplaceSelected() == false) {
                        overlayEditor = med;
                        break;
                    }
                }
                
                bool placeInDefaultLocation = false;
                if (overlayEditor == NULL) {
                    overlayEditor = new OverlaySettingsEditorDialog(browserWindow);
                    m_overlaySettingsEditors.insert(overlayEditor);
                    this->addNonModalDialog(overlayEditor);
                    placeInDefaultLocation = true;
                }
                else {
                    if (overlayEditor->isHidden()) {
                        placeInDefaultLocation = true;
                    }
                    
                    /*
                     * Is the overlay editor requested for a window
                     * that is not the parent of this overlay editor?
                     */
                    if (browserWindow != overlayEditor->parent()) {
                        /*
                         * Switch the parent of the overlay editor.
                         * On Linux, this will cause the overlay editor 
                         * to be "brought to the front" only when its
                         * parent is "brought to the front".
                         * 
                         * The position must be preserved.
                         */
                        const QPoint globalPos = overlayEditor->pos();
                        overlayEditor->setParent(browserWindow,
                                                 overlayEditor->windowFlags());
                        overlayEditor->move(globalPos);
                    }
                }
                
                overlayEditor->updateDialogContent(overlay);
                overlayEditor->show();
                overlayEditor->raise();
                overlayEditor->activateWindow();
                if (placeInDefaultLocation) {
                    WuQtUtilities::moveWindowToSideOfParent(browserWindow,
                                                            overlayEditor);
                }
            }
            case EventOverlaySettingsEditorDialogRequest::MODE_UPDATE_ALL:
                for (std::set<OverlaySettingsEditorDialog*>::iterator overlayEditorIter = m_overlaySettingsEditors.begin();
                     overlayEditorIter != m_overlaySettingsEditors.end();
                     overlayEditorIter++) {
                    OverlaySettingsEditorDialog* med = *overlayEditorIter;
                    med->updateDialog();
                }
                break;
        }
        mapEditEvent->setEventProcessed();
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_OPERATING_SYSTEM_REQUEST_OPEN_DATA_FILE) {
        EventOperatingSystemRequestOpenDataFile* openFileEvent =
        dynamic_cast<EventOperatingSystemRequestOpenDataFile*>(event);
        CaretAssert(openFileEvent);
        
        BrainBrowserWindow* bbw = getActiveBrowserWindow();
        if (bbw != NULL) {
            std::vector<AString> filenamesVector;
            std::vector<DataFileTypeEnum::Enum> dataFileTypeVectorNotUsed;
            filenamesVector.push_back(openFileEvent->getDataFileName());
            bbw->loadFiles(bbw,
                           filenamesVector,
                           dataFileTypeVectorNotUsed,
                           BrainBrowserWindow::LOAD_SPEC_FILE_WITH_DIALOG,
                           "",
                           "");
        }
        else {
            /*
             * Browser window has not yet been created.
             * After it is created, the file will be opened.
             */
            m_nameOfDataFileToOpenAfterStartup = openFileEvent->getDataFileName();
        }
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_PALETTE_COLOR_MAPPING_EDITOR_SHOW) {
        EventPaletteColorMappingEditorDialogRequest* paletteEditEvent =
        dynamic_cast<EventPaletteColorMappingEditorDialogRequest*>(event);
        CaretAssert(paletteEditEvent);

        BrainBrowserWindow* browserWindow = m_brainBrowserWindows[paletteEditEvent->getBrowserWindowIndex()];
        CaretAssert(browserWindow);
        
        int32_t browserTabIndex = -1;
        BrowserTabContent* tabContent = browserWindow->getBrowserTabContent();
        if (tabContent != NULL) {
            browserTabIndex = tabContent->getTabNumber();
        }

        bool placeInDefaultLocation = false;
        if (m_paletteColorMappingEditor == NULL) {
            m_paletteColorMappingEditor = new PaletteColorMappingEditorDialog(browserWindow);
            addNonModalDialog(m_paletteColorMappingEditor);
            placeInDefaultLocation = true;
        }
        else if (m_paletteColorMappingEditor->isHidden()) {
            placeInDefaultLocation = true;
        }
        
        m_paletteColorMappingEditor->updateDialogContent(paletteEditEvent->getCaretMappableDataFile(),
                                                         paletteEditEvent->getMapIndex(),
                                                         browserTabIndex);
        m_paletteColorMappingEditor->show();
        m_paletteColorMappingEditor->raise();
        m_paletteColorMappingEditor->activateWindow();
        if (placeInDefaultLocation) {
            WuQtUtilities::moveWindowToSideOfParent(browserWindow,
                                                    m_paletteColorMappingEditor);
        }
        
        paletteEditEvent->setEventProcessed();
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_HELP_VIEWER_DISPLAY) {
        EventHelpViewerDisplay* helpEvent = dynamic_cast<EventHelpViewerDisplay*>(event);
        CaretAssert(helpEvent);
        
        showHideHelpDialog(true,
                           helpEvent->getBrainBrowserWindow());
        m_helpViewerDialog->showHelpPageWithName(helpEvent->getHelpPageName());
    }
}

/**
 * Remove the tab content from all browser windows except for the given
 * browser windows, close the other browser windows, and then return
 * the tab content.
 *
 * @param browserWindow
 *    Browser window that gets tab content from all other windows.
 * @param tabContents
 *    Tab content from all other windows.
 */
void 
GuiManager::closeOtherWindowsAndReturnTheirTabContent(BrainBrowserWindow* browserWindow,
                                                      std::vector<BrowserTabContent*>& tabContents)
{
    tabContents.clear();
    
    const int32_t numWindows = m_brainBrowserWindows.size();
    for (int32_t i = 0; i < numWindows; i++) {
        BrainBrowserWindow* bbw = m_brainBrowserWindows[i];
        if (bbw != NULL) {
            if (bbw != browserWindow) {
                std::vector<BrowserTabContent*> tabs;
                bbw->removeAndReturnAllTabs(tabs);
                tabContents.insert(tabContents.end(), 
                                   tabs.begin(), 
                                   tabs.end());
                this->allowBrowserWindowsToCloseWithoutConfirmation = true;
                bbw->close();
                
                /*
                 * Should delete the windows that were closed!
                 * When a window is closed, Qt uses 'deleteLater'
                 * but we need them deleted now so that event listeners
                 * are shut down since the closed windows no longer
                 * have any content.
                 */
                QCoreApplication::sendPostedEvents(0,  QEvent::DeferredDelete);
                
                this->allowBrowserWindowsToCloseWithoutConfirmation = false;
            }
        }
    }
    
}

/**
 * Close all but the given window.
 * @param browserWindow
 *    Window that is NOT closed.
 */
void 
GuiManager::closeAllOtherWindows(BrainBrowserWindow* browserWindow)
{
    const int32_t numWindows = m_brainBrowserWindows.size();
    for (int32_t i = 0; i < numWindows; i++) {
        BrainBrowserWindow* bbw = m_brainBrowserWindows[i];
        if (bbw != NULL) {
            if (bbw != browserWindow) {
                this->allowBrowserWindowsToCloseWithoutConfirmation = true;
                bbw->close();
                
                /*
                 * Should delete the windows that were closed!
                 * When a window is closed, Qt uses 'deleteLater'
                 * but we need them deleted now so that event listeners
                 * are shut down since the closed windows no longer
                 * have any content.
                 */
                QCoreApplication::sendPostedEvents(0,  QEvent::DeferredDelete);
                
                this->allowBrowserWindowsToCloseWithoutConfirmation = false;
            }
        }
    }
}

/** 
 * Reparent non-modal dialogs that may need to be reparented if the 
 * original parent, a BrainBrowserWindow is closed in which case the
 * dialog is reparented to a different BrainBrowserWindow.
 *
 * @param closingBrainBrowserWindow
 *    Browser window that is closing.
 */
void 
GuiManager::reparentNonModalDialogs(BrainBrowserWindow* closingBrainBrowserWindow)
{
    BrainBrowserWindow* firstBrainBrowserWindow = NULL;
    
    for (int32_t i = 0; i < static_cast<int32_t>(m_brainBrowserWindows.size()); i++) {
        if (m_brainBrowserWindows[i] != NULL) {
            if (m_brainBrowserWindows[i] != closingBrainBrowserWindow) {
                firstBrainBrowserWindow = m_brainBrowserWindows[i];
                break;
            }
        }
    }
    
    if (firstBrainBrowserWindow != NULL) {
        for (std::set<QWidget*>::iterator iter = this->nonModalDialogs.begin();
                 iter != this->nonModalDialogs.end();
                 iter++) {
            QWidget* d = *iter;
            if (d->parent() == closingBrainBrowserWindow) {
                const bool wasVisible = d->isVisible();
                const QPoint globalPos = d->pos();
                d->setParent(firstBrainBrowserWindow, d->windowFlags());
                d->move(globalPos);
                if (wasVisible) {
                    d->show();
                }
                else {
                    d->hide();
                }
            }
            
            /*
             * Update any dialogs that are WuQ non modal dialogs.
             */
            WuQDialogNonModal* wuqNonModalDialog = dynamic_cast<WuQDialogNonModal*>(d);
            if (wuqNonModalDialog != NULL) {
                wuqNonModalDialog->updateDialog();
            }
        }
    }
}

/**
 * Show the surface properties editor dialog.
 * @param browserWindow
 *    Browser window on which dialog is displayed.
 */
void
GuiManager::processShowSurfacePropertiesEditorDialog(BrainBrowserWindow* browserWindow)
{
    bool wasCreatedFlag = false;
    
    if (this->m_surfacePropertiesEditorDialog == NULL) {
        m_surfacePropertiesEditorDialog = new SurfacePropertiesEditorDialog(browserWindow);
        this->addNonModalDialog(m_surfacePropertiesEditorDialog);
        m_surfacePropertiesEditorDialog->setSaveWindowPositionForNextTime(true);
        wasCreatedFlag = true;
    }
    m_surfacePropertiesEditorDialog->setVisible(true);
    m_surfacePropertiesEditorDialog->show();
    m_surfacePropertiesEditorDialog->activateWindow();
    
    if (wasCreatedFlag) {
        WuQtUtilities::moveWindowToSideOfParent(browserWindow,
                                                m_surfacePropertiesEditorDialog);
    }
}

/**
 * @return The action for showing/hiding the scene dialog.
 */
QAction*
GuiManager::getSceneDialogDisplayAction()
{
    return m_sceneDialogDisplayAction;
}

/**
 * Gets called when the scene dialog action is toggled.
 *
 * @param status
 *    New status (true display dialog, false hide it).
 */
void
GuiManager::sceneDialogDisplayActionToggled(bool status)
{
    showHideSceneDialog(status,
                        NULL);
}

/**
 * Gets called by the scene dialog when the scene dialog is closed.
 * Ensures that the scene dialog display action status remains
 * synchronized with the displayed status of the scene dialog.
 */
void
GuiManager::sceneDialogWasClosed()
{
    m_sceneDialogDisplayAction->blockSignals(true);
    m_sceneDialogDisplayAction->setChecked(false);
    m_sceneDialogDisplayAction->blockSignals(false);
}

/**
 * Show or hide the scene dialog.
 *
 * @param status
 *     True means show, false means hide.
 * @param parentBrainBrowserWindow
 *     If this is not NULL, and the scene dialog needs to be created,
 *     use this window as the parent and place the dialog next to this
 *     window.
 */
void
GuiManager::showHideSceneDialog(const bool status,
                                BrainBrowserWindow* parentBrainBrowserWindow)
{
    bool dialogWasCreated = false;
    
    QWidget* moveWindowParent = parentBrainBrowserWindow;
    
    if (status) {
        if (this->sceneDialog == NULL) {
            BrainBrowserWindow* sceneDialogParent = parentBrainBrowserWindow;
            if (sceneDialogParent == NULL) {
                sceneDialogParent = getActiveBrowserWindow();
            }
            
            this->sceneDialog = new SceneDialog(sceneDialogParent);
            this->addNonModalDialog(this->sceneDialog);
            QObject::connect(this->sceneDialog, SIGNAL(dialogWasClosed()),
                             this, SLOT(sceneDialogWasClosed()));
            
            dialogWasCreated = true;
            
            /*
             * If there was no parent dialog for placement of the scene
             * dialog and there is only one browser window, use the browser 
             * for placement of the scene dialog.
             */
            if (moveWindowParent == NULL) {
                if (getAllOpenBrainBrowserWindows().size() == 1) {
                    moveWindowParent = sceneDialogParent;
                }
            }
        }
        
        this->sceneDialog->show();
        this->sceneDialog->activateWindow();
    }
    else {
        this->sceneDialog->close();
    }
 
    if (dialogWasCreated) {
        if (moveWindowParent != NULL) {
            WuQtUtilities::moveWindowToSideOfParent(moveWindowParent,
                                                    this->sceneDialog);
        }
    }
    
    m_sceneDialogDisplayAction->blockSignals(true);
    m_sceneDialogDisplayAction->setChecked(status);
    m_sceneDialogDisplayAction->blockSignals(false);
}


/**
 * Show the scene dialog.  If dialog needs to be created, use the
 * given window as the parent.
 * @param browserWindowIn
 *    Parent of scene dialog if it needs to be created.
 */
void 
GuiManager::processShowSceneDialog(BrainBrowserWindow* browserWindowIn)
{
    showHideSceneDialog(true,
                        browserWindowIn);
}

/**
 * Show the scene dialog and load the given scene from the given scene file.
 * If displaying the scene had an error, the scene dialog will remain open.
 * Otherwise, the scene dialog is closed.
 *
 * @param browserWindow
 *    Parent of scene dialog if it needs to be created.
 * @param sceneFile
 *    Scene File that contains the scene.
 * @param scene
 *    Scene that is displayed.
 */
void
GuiManager::processShowSceneDialogAndScene(BrainBrowserWindow* browserWindow,
                                           SceneFile* sceneFile,
                                           Scene* scene)
{
    showHideSceneDialog(true,
                        browserWindow);
    
    const bool sceneWasDisplayed = this->sceneDialog->displayScene(sceneFile,
                                                                   scene);
    if (sceneWasDisplayed) {
        showHideSceneDialog(false,
                            NULL);
    }
}

/**
 * Show the Workbench Bug Report Dialog.
 *
 * @param browserWindow
 *    Parent of dialog if it needs to be created.
 * @param openGLInformation
 *    Information about OpenGL.
 */
void
GuiManager::processShowBugReportDialog(BrainBrowserWindow* browserWindow,
                                       const AString& openGLInformation)
{
    if (m_bugReportDialog == NULL) {
        m_bugReportDialog = new BugReportDialog(browserWindow,
                                                openGLInformation);
        this->addNonModalDialog(m_bugReportDialog);
    }
    
    m_bugReportDialog->setVisible(true);
    m_bugReportDialog->show();
    m_bugReportDialog->activateWindow();
}

/**
 * @return Action for display of help viewer.
 */
QAction*
GuiManager::getHelpViewerDialogDisplayAction()
{
    return m_helpViewerDialogDisplayAction;
}

/**
 * Show or hide the help dialog.
 *
 * @param status
 *     True means show, false means hide.
 * @param parentBrainBrowserWindow
 *     If this is not NULL, and the help dialog needs to be created,
 *     use this window as the parent and place the dialog next to this
 *     window.
 */
void
GuiManager::showHideHelpDialog(const bool status,
                        BrainBrowserWindow* parentBrainBrowserWindow)
{
    bool dialogWasCreated = false;
    
    QWidget* moveWindowParent = parentBrainBrowserWindow;
    
    if (status) {
        if (m_helpViewerDialog == NULL) {
            BrainBrowserWindow* helpDialogParent = parentBrainBrowserWindow;
            if (helpDialogParent == NULL) {
                helpDialogParent = getActiveBrowserWindow();
            }
            
            m_helpViewerDialog = new HelpViewerDialog(helpDialogParent);
            this->addNonModalDialog(m_helpViewerDialog);
            QObject::connect(m_helpViewerDialog, SIGNAL(dialogWasClosed()),
                             this, SLOT(helpDialogWasClosed()));
            
            dialogWasCreated = true;
            
            /*
             * If there was no parent dialog for placement of the help
             * dialog and there is only one browser window, use the browser
             * for placement of the help dialog.
             */
            if (moveWindowParent == NULL) {
                if (getAllOpenBrainBrowserWindows().size() == 1) {
                    moveWindowParent = helpDialogParent;
                }
            }
        }
        
        m_helpViewerDialog->show();
        m_helpViewerDialog->activateWindow();
    }
    else {
        m_helpViewerDialog->close();
    }
    
    if (dialogWasCreated) {
        if (moveWindowParent != NULL) {
            WuQtUtilities::moveWindowToSideOfParent(moveWindowParent,
                                                    m_helpViewerDialog);
        }
    }
    
    m_helpViewerDialogDisplayAction->blockSignals(true);
    m_helpViewerDialogDisplayAction->setChecked(status);
    m_helpViewerDialogDisplayAction->blockSignals(false);    
}

/**
 * Gets called by the help dialog when the help dialog is closed.
 * Ensures that the help dialog display action status remains
 * synchronized with the displayed status of the help dialog.
 */

void
GuiManager::helpDialogWasClosed()
{
    m_helpViewerDialogDisplayAction->blockSignals(true);
    m_helpViewerDialogDisplayAction->setChecked(false);
    m_helpViewerDialogDisplayAction->blockSignals(false);
}

/**
 * Called when show help action is triggered
 */
void
GuiManager::showHelpDialogActionToggled(bool status)
{
    showHideHelpDialog(status, NULL);
}

/**
 * @return The action that indicates the enabled status
 * for display of the information window.
 */
QAction* 
GuiManager::getInformationDisplayDialogEnabledAction()
{
    return m_informationDisplayDialogEnabledAction;
}

/**
 * @return The action that indicates the enabled status
 * for display of the identify brainordinate dialog.
 */
QAction*
GuiManager::getIdentifyBrainordinateDialogDisplayAction()
{
    return m_identifyBrainordinateDialogEnabledAction;
}


/**
 * Show the information window.
 */
void 
GuiManager::processShowInformationWindow()
{
    this->processShowInformationDisplayDialog(true);
}

void 
GuiManager::showHideInfoWindowSelected(bool status)
{
    QString text("Show Information Window");
    if (status) {
        text = "Hide Information Window";
        if (m_informationDisplayDialogEnabledAction->signalsBlocked() == false) {
            this->processShowInformationDisplayDialog(true);
        }
    }
    
    text += ("\n\n"
             "When this button is 'on', the information window\n"
             "is automatically displayed when an identification\n"
             "operation (mouse click over surface or volume slice)\n"
             "is performed.  ");
    m_informationDisplayDialogEnabledAction->setToolTip(text);
}


/**
 * Show the information display window.
 * @param forceDisplayOfDialog
 *   If true, the window will be displayed even if its display
 *   enabled status is off.
 */
void 
GuiManager::processShowInformationDisplayDialog(const bool forceDisplayOfDialog)
{
    if (m_informationDisplayDialog == NULL) {
        std::vector<BrainBrowserWindow*> bbws = this->getAllOpenBrainBrowserWindows();
        if (bbws.empty() == false) {
            BrainBrowserWindow* parentWindow = bbws[0];
#ifdef CARET_OS_MACOSX
            m_informationDisplayDialog = new InformationDisplayDialog(parentWindow);
            this->addNonModalDialog(m_informationDisplayDialog);
#else // CARET_OS_MACOSX
            m_informationDisplayDialog = new InformationDisplayDialog(NULL);
            addParentLessNonModalDialog(m_informationDisplayDialog);
#endif // CARET_OS_MACOSX
            m_informationDisplayDialog->resize(600, 200);
            m_informationDisplayDialog->setSaveWindowPositionForNextTime(true);
            WuQtUtilities::moveWindowToSideOfParent(parentWindow,
                                                    m_informationDisplayDialog);
        }
    }
    
    if (forceDisplayOfDialog
        || m_informationDisplayDialogEnabledAction->isChecked()) {
        if (m_informationDisplayDialog != NULL) {
            m_informationDisplayDialog->setVisible(true);
            m_informationDisplayDialog->show();
            m_informationDisplayDialog->activateWindow();
        }
    }
}

/**
 * Show/hide the identify dialog.
 *
 * @param status
 *     Status (true/false) to display dialog.
 */
void
GuiManager::showIdentifyBrainordinateDialogActionToggled(bool status)
{
    showHideIdentfyBrainordinateDialog(status,
                          NULL);
}

/**
 * Show or hide the identify brainordinate dialog.
 *
 * @param status
 *     True means show, false means hide.
 * @param parentBrainBrowserWindow
 *     If this is not NULL, and the help dialog needs to be created,
 *     use this window as the parent and place the dialog next to this
 *     window.
 */
void
GuiManager::showHideIdentfyBrainordinateDialog(const bool status,
                                  BrainBrowserWindow* parentBrainBrowserWindow)
{
    bool dialogWasCreated = false;
    
    QWidget* moveWindowParent = parentBrainBrowserWindow;
    
    if (status) {
        if (m_identifyBrainordinateDialog == NULL) {
            BrainBrowserWindow* idDialogParent = parentBrainBrowserWindow;
            if (idDialogParent == NULL) {
                idDialogParent = getActiveBrowserWindow();
            }
            
            m_identifyBrainordinateDialog = new IdentifyBrainordinateDialog(idDialogParent);
            //m_identifyBrainordinateDialog->setSaveWindowPositionForNextTime(true);
            this->addNonModalDialog(m_identifyBrainordinateDialog);
            QObject::connect(m_identifyBrainordinateDialog, SIGNAL(dialogWasClosed()),
                             this, SLOT(identifyBrainordinateDialogWasClosed()));
            
            dialogWasCreated = true;
            
            /*
             * If there was no parent dialog for placement of the help
             * dialog and there is only one browser window, use the browser
             * for placement of the help dialog.
             */
            if (moveWindowParent == NULL) {
                if (getAllOpenBrainBrowserWindows().size() == 1) {
                    moveWindowParent = idDialogParent;
                }
            }
        }
        
        m_identifyBrainordinateDialog->show();
        m_identifyBrainordinateDialog->activateWindow();
    }
    else {
        m_identifyBrainordinateDialog->close();
    }
    
    if (dialogWasCreated) {
        if (moveWindowParent != NULL) {
            WuQtUtilities::moveWindowToSideOfParent(moveWindowParent,
                                                    m_identifyBrainordinateDialog);
        }
    }
    
    m_identifyBrainordinateDialogEnabledAction->blockSignals(true);
    m_identifyBrainordinateDialogEnabledAction->setChecked(status);
    m_identifyBrainordinateDialogEnabledAction->blockSignals(false);
}

/**
 * Gets called when the identify dialog is closed.
 */
void
GuiManager::identifyBrainordinateDialogWasClosed()
{
    m_identifyBrainordinateDialogEnabledAction->blockSignals(true);
    m_identifyBrainordinateDialogEnabledAction->setChecked(false);
    m_identifyBrainordinateDialogEnabledAction->blockSignals(false);
}

/**
 * Add a non-modal dialog so that it may be reparented.
 *
 * @param dialog
 *    The dialog.
 */
void
GuiManager::addNonModalDialog(QWidget* dialog)
{
    CaretAssert(dialog);
    this->nonModalDialogs.insert(dialog);
}

/**
 * Add a parent-less dialog so that it may be deleted when 
 * the application is exited.
 *
 * @param dialog
 *    The dialog.
 */
void
GuiManager::addParentLessNonModalDialog(QWidget* dialog)
{
    CaretAssert(dialog);
    m_parentlessNonModalDialogs.insert(dialog);
}


/**
 * Show the clipping planes dialog.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void
GuiManager::processShowClippingPlanesDialog(BrainBrowserWindow* browserWindow)
{
    if (m_clippingPlanesDialog == NULL) {
        m_clippingPlanesDialog = new ClippingPlanesDialog(browserWindow);
        this->addNonModalDialog(m_clippingPlanesDialog);
    }
    
    const int32_t browserWindowIndex = browserWindow->getBrowserWindowIndex();
    m_clippingPlanesDialog->updateContent(browserWindowIndex);
    m_clippingPlanesDialog->setVisible(true);
    m_clippingPlanesDialog->show();
    m_clippingPlanesDialog->activateWindow();
}


/**
 * Show the custom view dialog.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void
GuiManager::processShowCustomViewDialog(BrainBrowserWindow* browserWindow)
{
    if (m_customViewDialog == NULL) {
        m_customViewDialog = new CustomViewDialog(browserWindow);
        this->addNonModalDialog(m_customViewDialog);
    }
    
    const int32_t browserWindowIndex = browserWindow->getBrowserWindowIndex();
    m_customViewDialog->updateContent(browserWindowIndex);
    m_customViewDialog->setVisible(true);
    m_customViewDialog->show();
    m_customViewDialog->activateWindow();
    
}

/**
 * Show the tile tabs configuration dialog.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void
GuiManager::processShowTileTabsConfigurationDialog(caret::BrainBrowserWindow *browserWindow)
{
    if (m_tileTabsConfigurationDialog == NULL) {
        m_tileTabsConfigurationDialog = new TileTabsConfigurationDialog(browserWindow);
        this->addNonModalDialog(m_tileTabsConfigurationDialog);
    }
    
    m_tileTabsConfigurationDialog->updateDialogWithSelectedTileTabsFromWindow(browserWindow);
    m_tileTabsConfigurationDialog->setVisible(true);
    m_tileTabsConfigurationDialog->show();
    m_tileTabsConfigurationDialog->activateWindow();
}

/**
 * Show the image capture window.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void 
GuiManager::processShowImageCaptureDialog(BrainBrowserWindow* browserWindow)
{
    if (this->imageCaptureDialog == NULL) {
        this->imageCaptureDialog = new ImageCaptureDialog(browserWindow);
        this->addNonModalDialog(this->imageCaptureDialog);
    }
    this->imageCaptureDialog->updateDialog();
    this->imageCaptureDialog->setBrowserWindowIndex(browserWindow->getBrowserWindowIndex());
    this->imageCaptureDialog->setVisible(true);
    this->imageCaptureDialog->show();
    this->imageCaptureDialog->activateWindow();
}

/**
 * Show the gaps and margins window.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void
GuiManager::processShowGapsAndMarginsDialog(BrainBrowserWindow* browserWindow)
{
    if (m_gapsAndMarginsDialog == NULL) {
        m_gapsAndMarginsDialog = new GapsAndMarginsDialog(browserWindow);
        this->addNonModalDialog(m_gapsAndMarginsDialog);
    }
    m_gapsAndMarginsDialog->updateDialog();
    //m_tabMarginsDialog->setBrowserWindowIndex(browserWindow->getBrowserWindowIndex());
    m_gapsAndMarginsDialog->setVisible(true);
    m_gapsAndMarginsDialog->show();
    m_gapsAndMarginsDialog->activateWindow();
}

/**
 * Show the record movie window.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void 
GuiManager::processShowMovieDialog(BrainBrowserWindow* browserWindow)
{
    if (this->movieDialog == NULL) {
        this->movieDialog = new MovieDialog(browserWindow);
        this->addNonModalDialog(this->movieDialog);
    }
    //this->movieDialog->updateDialog();
    //this->movieDialog->setBrowserWindowIndex(browserWindow->getBrowserWindowIndex());
    this->movieDialog->setVisible(true);
    this->movieDialog->show();
    this->movieDialog->activateWindow();
}



/**
 * Show the preferences window.
 * @param browserWindow
 *    Window on which dialog was requested.
 */
void 
GuiManager::processShowPreferencesDialog(BrainBrowserWindow* browserWindow)
{
    if (this->preferencesDialog == NULL) {
        this->preferencesDialog = new PreferencesDialog(browserWindow);
        this->addNonModalDialog(this->preferencesDialog);
    }
    this->preferencesDialog->updateDialog();
    this->preferencesDialog->setVisible(true);
    this->preferencesDialog->show();
    this->preferencesDialog->activateWindow();
}

/**
 * Show the allen database web view.
 * @param browserWindow
 *    If the web view needs to be created, use this as parent.
 */
void 
GuiManager::processShowAllenDataBaseWebView(BrainBrowserWindow* browserWindow)
{
    WuQMessageBox::informationOk(browserWindow, 
                                 "Allen Database connection not yet implemented");
}

/**
 * Show the connectome database web view.
 * @param browserWindow
 *    If the web view needs to be created, use this as parent.
 */
void 
GuiManager::processShowConnectomeDataBaseWebView(BrainBrowserWindow* /*browserWindow*/)
{
    CaretLogSevere("Webkit is disabled !!!");
// no webkit
//    if (this->connectomeDatabaseWebView == NULL) {
//        this->connectomeDatabaseWebView = new WuQWebView();
//        this->connectomeDatabaseWebView->load(QUrl("https://db.humanconnectome.org/"));
//        this->addNonModalDialog(this->connectomeDatabaseWebView);
//    }
//    this->connectomeDatabaseWebView->show();
}

/**
 * sets animation start time for Time Course Dialogs
 */
void GuiManager::updateAnimationStartTime(double /*value*/)
{
       
}

/**
 * @return The cursor manager.
 */
const 
CursorManager* 
GuiManager::getCursorManager() const
{
    return this->cursorManager;
}

/**
 * Create a scene for an instance of a class.
 *
 * @param sceneAttributes
 *    Attributes for the scene.  Scenes may be of different types
 *    (full, generic, etc) and the attributes should be checked when
 *    saving the scene.
 *
 * @return Pointer to SceneClass object representing the state of 
 *    this object.  Under some circumstances a NULL pointer may be
 *    returned.  Caller will take ownership of returned object.
 */
SceneClass* 
GuiManager::saveToScene(const SceneAttributes* sceneAttributes,
                        const AString& instanceName)
{
    SceneClass* sceneClass = new SceneClass(instanceName,
                                            "GuiManager",
                                            1);
    
    /*
     * Save session manager (brain, etc)
     */
    sceneClass->addClass(SessionManager::get()->saveToScene(sceneAttributes, 
                                                            "m_sessionManager"));
    

    /*
     * Save windows
     */
    std::vector<SceneClass*> browserWindowClasses;
    const int32_t numBrowserWindows = static_cast<int32_t>(m_brainBrowserWindows.size());
    for (int32_t i = 0; i < numBrowserWindows; i++) {
        BrainBrowserWindow* bbw = m_brainBrowserWindows[i];
        if (bbw != NULL) {
            browserWindowClasses.push_back(bbw->saveToScene(sceneAttributes,
                                                  "m_brainBrowserWindows"));
        }
    }
    SceneClassArray* browserWindowArray = new SceneClassArray("m_brainBrowserWindows",
                                                              browserWindowClasses);
    sceneClass->addChild(browserWindowArray);

    /*
     * Save information window
     */
    if (m_informationDisplayDialog != NULL) {
        sceneClass->addClass(m_informationDisplayDialog->saveToScene(sceneAttributes,
                                                                     "m_informationDisplayDialog"));
    }
    
    /*
     * Save surface properties window
     */
    if (m_surfacePropertiesEditorDialog != NULL) {
        sceneClass->addClass(m_surfacePropertiesEditorDialog->saveToScene(sceneAttributes,
                                                                          "m_surfacePropertiesEditorDialog"));
    }
    
    switch (sceneAttributes->getSceneType()) {
        case SceneTypeEnum::SCENE_TYPE_FULL:
            break;
        case SceneTypeEnum::SCENE_TYPE_GENERIC:
            break;
    }    
    
    return sceneClass;
}

/**
 * Restore the state of an instance of a class.
 * 
 * @param sceneAttributes
 *    Attributes for the scene.  Scenes may be of different types
 *    (full, generic, etc) and the attributes should be checked when
 *    restoring the scene.
 *
 * @param sceneClass
 *     SceneClass containing the state that was previously 
 *     saved and should be restored.
 */
void 
GuiManager::restoreFromScene(const SceneAttributes* sceneAttributes,
                             const SceneClass* sceneClass)
{
    const int64_t eventCountAtStart = EventManager::get()->getEventIssuedCounter();
    
    ElapsedTimer sceneRestoreTimer;
    sceneRestoreTimer.start();
    
    if (sceneClass == NULL) {
        return;
    }
    
    switch (sceneAttributes->getSceneType()) {
        case SceneTypeEnum::SCENE_TYPE_FULL:
            break;
        case SceneTypeEnum::SCENE_TYPE_GENERIC:
            break;
    }
    
    /*
     * Invalid first window position used when 
     * positioning other windows
     */
    SceneWindowGeometry::setFirstBrowserWindowCoordinatesInvalid();
        
    /*
     * Close all but one window and remove its tabs
     */
    BrainBrowserWindow* firstBrowserWindow = getActiveBrowserWindow();;
    if (firstBrowserWindow != NULL) {
        closeAllOtherWindows(firstBrowserWindow);
        
        /*
         * Remove all tabs from window.
         * The tab content will be deleted by the session manager.
         */
        std::vector<BrowserTabContent*> windowTabContent;
        firstBrowserWindow->removeAndReturnAllTabs(windowTabContent);
    }
    
    /*
     * Close Overlay and palette editor windows since the files
     * displayed in them may become invalid
     */
    for (std::set<OverlaySettingsEditorDialog*>::iterator overlayEditorIter = m_overlaySettingsEditors.begin();
         overlayEditorIter != m_overlaySettingsEditors.end();
         overlayEditorIter++) {
        OverlaySettingsEditorDialog* med = *overlayEditorIter;
        med->close();
    }
    if (m_paletteColorMappingEditor != NULL) {
        m_paletteColorMappingEditor->close();
    }
    
    /*
     * Update the windows
     */
    EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());    
    
    /*
     * Blocking user-interface and graphics event will speed up
     * restoration of the user interface.
     */
    bool blockUserInteraceUpdateEvents = true;
    if (blockUserInteraceUpdateEvents) {
        EventManager::get()->blockEvent(EventTypeEnum::EVENT_USER_INTERFACE_UPDATE,
                                        true);
    }
    EventManager::get()->blockEvent(EventTypeEnum::EVENT_GRAPHICS_UPDATE_ALL_WINDOWS,
                                    true);
    EventManager::get()->blockEvent(EventTypeEnum::EVENT_GRAPHICS_UPDATE_ONE_WINDOW,
                                    true);
    
    /*
     * Restore session manager
     */
    SessionManager::get()->restoreFromScene(sceneAttributes, 
                                            sceneClass->getClass("m_sessionManager"));

    EventProgressUpdate progressEvent("");
    
    /*
     * See if models available (user may have cancelled scene loading.
     */
    EventModelGetAll getAllModelsEvent;
    EventManager::get()->sendEvent(getAllModelsEvent.getPointer());
    const bool haveModels = (getAllModelsEvent.getModels().empty() == false);

    ElapsedTimer timer;
    timer.start();
    
    if (haveModels) {
        /*
         * Get open windows
         */
        std::list<BrainBrowserWindow*> availableWindows;
        const int32_t numBrowserWindows = static_cast<int32_t>(m_brainBrowserWindows.size());
        for (int32_t i = 0; i < numBrowserWindows; i++) {
            if (m_brainBrowserWindows[i] != NULL) {
                availableWindows.push_back(m_brainBrowserWindows[i]);
            }
        }
        
        /*
         * Restore windows
         */
        progressEvent.setProgressMessage("Restoring browser windows");
        EventManager::get()->sendEvent(progressEvent.getPointer());
        const SceneClassArray* browserWindowArray = sceneClass->getClassArray("m_brainBrowserWindows");
        if (browserWindowArray != NULL) {
            const int32_t numBrowserClasses = browserWindowArray->getNumberOfArrayElements();
            for (int32_t i = 0; i < numBrowserClasses; i++) {
                const SceneClass* browserClass = browserWindowArray->getClassAtIndex(i);
                BrainBrowserWindow* bbw = NULL;
                if (availableWindows.empty() == false) {
                    bbw = availableWindows.front();
                    availableWindows.pop_front();
                }
                else {
                    bbw = newBrainBrowserWindow(NULL,
                                                NULL,
                                                false);
                }
                if (bbw != NULL) {
                    bbw->restoreFromScene(sceneAttributes,
                                          browserClass);
                }
            }
        }
        
        CaretLogFine("Time to restore browser windows was "
                     + QString::number(timer.getElapsedTimeSeconds(), 'f', 3)
                     + " seconds");
        timer.reset();
        
        /*
         * Restore information window
         */
        progressEvent.setProgressMessage("Restoring Information Window");
        EventManager::get()->sendEvent(progressEvent.getPointer());
        const SceneClass* infoWindowClass = sceneClass->getClass("m_informationDisplayDialog");
        if (infoWindowClass != NULL) {
            if (m_informationDisplayDialog == NULL) {
                processShowInformationWindow();
            }
            else if (m_informationDisplayDialog->isVisible() == false) {
                processShowInformationWindow();
            }
            m_informationDisplayDialog->restoreFromScene(sceneAttributes,
                                                         infoWindowClass);
        }
        else {
            if (m_informationDisplayDialog != NULL) {
                /*
                 * Will clear text
                 */
                m_informationDisplayDialog->restoreFromScene(sceneAttributes,
                                                             NULL);
            }
        }
        
        /*
         * Restore surface properties
         */
        progressEvent.setProgressMessage("Restoring Surface Properties Window");
        EventManager::get()->sendEvent(progressEvent.getPointer());
        const SceneClass* surfPropClass = sceneClass->getClass("m_surfacePropertiesEditorDialog");
        if (surfPropClass != NULL) {
            if (m_surfacePropertiesEditorDialog == NULL) {
                processShowSurfacePropertiesEditorDialog(firstBrowserWindow);
            }
            else if (m_surfacePropertiesEditorDialog->isVisible() == false) {
                processShowSurfacePropertiesEditorDialog(firstBrowserWindow);
            }
            m_surfacePropertiesEditorDialog->restoreFromScene(sceneAttributes,
                                                              surfPropClass);
        }
        
        CaretLogFine("Time to restore information/property windows was "
                     + QString::number(timer.getElapsedTimeSeconds(), 'f', 3)
                     + " seconds");
        timer.reset();
    }
    
    if (imageCaptureDialog != NULL) {
        imageCaptureDialog->updateDialog();
    }
    
    progressEvent.setProgressMessage("Invalidating coloring and updating user interface");
    EventManager::get()->sendEvent(progressEvent.getPointer());
    EventManager::get()->sendEvent(EventSurfaceColoringInvalidate().getPointer());

    if (blockUserInteraceUpdateEvents) {
        EventManager::get()->blockEvent(EventTypeEnum::EVENT_USER_INTERFACE_UPDATE,
                                    false);
    }
    EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
    
    /*
     * Unblock graphics updates
     */
    EventManager::get()->blockEvent(EventTypeEnum::EVENT_GRAPHICS_UPDATE_ALL_WINDOWS, 
                                    false);
    EventManager::get()->blockEvent(EventTypeEnum::EVENT_GRAPHICS_UPDATE_ONE_WINDOW,
                                    false);

    progressEvent.setProgressMessage("Updating graphics in all windows");
    EventManager::get()->sendEvent(progressEvent.getPointer());
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());

    CaretLogFine("Time to update graphics in all windows was "
                 + QString::number(timer.getElapsedTimeSeconds(), 'f', 3)
                 + " seconds");
    timer.reset();

    const int64_t totalNumberOfEvents = (EventManager::get()->getEventIssuedCounter()
                                         - eventCountAtStart);
    CaretLogFine("Time to restore scene was "
                 + QString::number(sceneRestoreTimer.getElapsedTimeSeconds(), 'f', 3)
                 + " seconds with "
                 + AString::number(totalNumberOfEvents)
                 + " events");
}

/**
 * Get the name of the data file that may have been set
 * if Workbench was started by double-clicking a file
 * in the Mac OSX finder.
 */
AString
GuiManager::getNameOfDataFileToOpenAfterStartup() const
{
    return m_nameOfDataFileToOpenAfterStartup;
}

/**
 * Process identification after item(s) selected using a selection manager.
 *
 * @parma tabIndex
 *    Index of tab in which identification took place.  This value may
 *    be negative indicating that the identification request is not
 *    from a browser tab.  One source for this is the Select Brainordinate
 *    option on the Information Window.
 * @param selectionManager
 *    The selection manager.
 * @param parentWidget
 *    Widget on which any error message windows are displayed.
 */
void
GuiManager::processIdentification(const int32_t tabIndex,
                                  SelectionManager* selectionManager,
                                  QWidget* parentWidget)
{
    CursorDisplayScoped cursor;
    cursor.showWaitCursor();
    
    Brain* brain = GuiManager::get()->getBrain();
    CiftiConnectivityMatrixDataFileManager* ciftiConnectivityManager = SessionManager::get()->getCiftiConnectivityMatrixDataFileManager();
    CiftiFiberTrajectoryManager* ciftiFiberTrajectoryManager = SessionManager::get()->getCiftiFiberTrajectoryManager();
    ChartingDataManager* chartingDataManager = brain->getChartingDataManager();
    IdentificationManager* identificationManager = brain->getIdentificationManager();
    
    bool updateGraphicsFlag = false;
    bool updateInformationFlag = false;
    std::vector<AString> ciftiLoadingInfo;
    
    const QString breakAndIndent("<br>&nbsp;&nbsp;&nbsp;&nbsp;");
    SelectionItemSurfaceNodeIdentificationSymbol* nodeIdSymbol = selectionManager->getSurfaceNodeIdentificationSymbol();
    SelectionItemVoxelIdentificationSymbol*  voxelIdSymbol = selectionManager->getVoxelIdentificationSymbol();
    if ((nodeIdSymbol->getSurface() != NULL)
        && (nodeIdSymbol->getNodeNumber() >= 0)) {
        const Surface* surface = nodeIdSymbol->getSurface();
        const int32_t surfaceNumberOfNodes = surface->getNumberOfNodes();
        const int32_t nodeIndex = nodeIdSymbol->getNodeNumber();
        const StructureEnum::Enum structure = surface->getStructure();
        
        identificationManager->removeIdentifiedNodeItem(structure,
                                                        surfaceNumberOfNodes,
                                                        nodeIndex);
        updateGraphicsFlag = true;
        updateInformationFlag = true;
    }
    else if (voxelIdSymbol->isValid()) {
        float voxelXYZ[3];
        voxelIdSymbol->getVoxelXYZ(voxelXYZ);
        identificationManager->removeIdentifiedVoxelItem(voxelXYZ);

        updateGraphicsFlag = true;
        updateInformationFlag = true;
    }
    else {
        IdentifiedItem* identifiedItem = NULL;
        
        SelectionItemSurfaceNode* idNode = selectionManager->getSurfaceNodeIdentification();
        SelectionItemVoxel* idVoxel = selectionManager->getVoxelIdentification();
        
        /*
         * If there is a voxel ID but no node ID, identify a
         * node near the voxel coordinate, if it is close by.
         */
        bool nodeIdentificationCreatedFromVoxelIdentificationFlag = false;
        if (idNode->isValid() == false) {
            if (idVoxel->isValid()) {
                double doubleXYZ[3];
                idVoxel->getModelXYZ(doubleXYZ);
                const float voxelXYZ[3] = {
                    doubleXYZ[0],
                    doubleXYZ[1],
                    doubleXYZ[2]
                };
                Surface* surface = brain->getPrimaryAnatomicalSurfaceNearestCoordinate(voxelXYZ,
                                                                                       3.0);
                if (surface != NULL) {
                    const int nodeIndex = surface->closestNode(voxelXYZ);
                    if (nodeIndex >= 0) {
                        idNode->reset();
                        idNode->setBrain(brain);
                        idNode->setSurface(surface);
                        idNode->setNodeNumber(nodeIndex);
                        nodeIdentificationCreatedFromVoxelIdentificationFlag = true;
                    }
                }
            }
        }
        
        /*
         * Load CIFTI NODE data prior to ID Message so that CIFTI
         * data shown in identification text is correct for the
         * node that was loaded.
         */
        bool triedToLoadSurfaceODemandData = false;
        if (idNode->isValid()) {
            /*
             * NOT: node id was NOT created from voxel ID
             */
            if (nodeIdentificationCreatedFromVoxelIdentificationFlag == false) {
                Surface* surface = idNode->getSurface();
                const int32_t nodeIndex = idNode->getNodeNumber();
                try {
                    triedToLoadSurfaceODemandData = true;
                    
                    ciftiConnectivityManager->loadDataForSurfaceNode(brain,
                                                                     surface,
                                                                     nodeIndex,
                                                                     ciftiLoadingInfo);
                    
                    ciftiFiberTrajectoryManager->loadDataForSurfaceNode(brain,
                                                                        surface,
                                                                        nodeIndex,
                                                                        ciftiLoadingInfo);
                    chartingDataManager->loadChartForSurfaceNode(surface,
                                                                 nodeIndex);
                    updateGraphicsFlag = true;
                }
                catch (const DataFileException& e) {
                    cursor.restoreCursor();
                    QMessageBox::critical(parentWidget, "", e.whatString());
                    cursor.showWaitCursor();
                }
            }
        }
        
        /*
         * Load CIFTI VOXEL data prior to ID Message so that CIFTI
         * data shown in identification text is correct for the
         * voxel that was loaded.
         *
         * Note: If there was an attempt to load data for surface,
         * do not try to load voxel data.  Otherwise, if the voxel
         * data loading fails, it will clear the coloring for 
         * the connetivity data and make it appear as if loading data
         * failed.
         */
        if (idVoxel->isValid()
            && ( ! triedToLoadSurfaceODemandData)) {
            const VolumeMappableInterface* volumeFile = idVoxel->getVolumeFile();
            int64_t voxelIJK[3];
            idVoxel->getVoxelIJK(voxelIJK);
            if (volumeFile != NULL) {
                float xyz[3];
                volumeFile->indexToSpace(voxelIJK, xyz);
                
                updateGraphicsFlag = true;
                
                try {
                    ciftiConnectivityManager->loadDataForVoxelAtCoordinate(brain,
                                                                           xyz,
                                                           ciftiLoadingInfo);
                    ciftiFiberTrajectoryManager->loadDataForVoxelAtCoordinate(brain,
                                                                              xyz,
                                                                              ciftiLoadingInfo);
                }
                catch (const DataFileException& e) {
                    cursor.restoreCursor();
                    QMessageBox::critical(parentWidget, "", e.whatString());
                    cursor.showWaitCursor();
                }
                try {
                    chartingDataManager->loadChartForVoxelAtCoordinate(xyz);
                }
                catch (const DataFileException& e) {
                    cursor.restoreCursor();
                    QMessageBox::critical(parentWidget, "", e.whatString());
                    cursor.showWaitCursor();
                }
            }
        }
        
        SelectionItemChartMatrix* idChartMatrix = selectionManager->getChartMatrixIdentification();
        if (idChartMatrix->isValid()) {
            ChartableMatrixInterface* chartMatrixInterface = idChartMatrix->getChartableMatrixInterface();
            if (chartMatrixInterface != NULL) {
                CiftiConnectivityMatrixParcelFile* ciftiParcelFile = dynamic_cast<CiftiConnectivityMatrixParcelFile*>(chartMatrixInterface);
                if (ciftiParcelFile != NULL) {
                    if (ciftiParcelFile->isMapDataLoadingEnabled(0)) {
                        const int32_t rowIndex = idChartMatrix->getMatrixRowIndex();
                        const int32_t columnIndex = idChartMatrix->getMatrixColumnIndex();
                        if ((rowIndex >= 0)
                            && (columnIndex >= 0)) {
                            try {
                                ciftiConnectivityManager->loadRowOrColumnFromParcelFile(brain,
                                                                                        ciftiParcelFile,
                                                                                        rowIndex,
                                                                                        columnIndex,
                                                                                        ciftiLoadingInfo);
                                
                            }
                            catch (const DataFileException& e) {
                                cursor.restoreCursor();
                                QMessageBox::critical(parentWidget, "", e.whatString());
                                cursor.showWaitCursor();
                            }
                            updateGraphicsFlag = true;
                        }
                    }
                }
                
                CiftiScalarDataSeriesFile* scalarDataSeriesFile = dynamic_cast<CiftiScalarDataSeriesFile*>(chartMatrixInterface);
                if (scalarDataSeriesFile != NULL) {
                    const int32_t rowIndex = idChartMatrix->getMatrixRowIndex();
                    if (rowIndex >= 0) {
                        scalarDataSeriesFile->setSelectedMapIndex(tabIndex,
                                                                  rowIndex);
                        
                        const MapYokingGroupEnum::Enum mapYoking = scalarDataSeriesFile->getMatrixRowColumnMapYokingGroup(tabIndex);
                        
                        if (mapYoking != MapYokingGroupEnum::MAP_YOKING_GROUP_OFF) {
                            EventMapYokingSelectMap selectMapEvent(mapYoking,
                                                                   scalarDataSeriesFile,
                                                                   rowIndex,
                                                                   true);
                            EventManager::get()->sendEvent(selectMapEvent.getPointer());
                        }
                        else {
                            chartingDataManager->loadChartForCiftiMappableFileRow(scalarDataSeriesFile,
                                                                                  rowIndex);
                        }
                        
                        updateGraphicsFlag = true;
                    }
                }
            }
        }
        
        SelectionItemCiftiConnectivityMatrixRowColumn* idCiftiConnMatrix = selectionManager->getCiftiConnectivityMatrixRowColumnIdentification();
        if (idCiftiConnMatrix != NULL) {
            CiftiMappableConnectivityMatrixDataFile* matrixFile = idCiftiConnMatrix->getCiftiConnectivityMatrixFile();
            if (matrixFile != NULL) {
                const int32_t rowIndex = idCiftiConnMatrix->getMatrixRowIndex();
                const int32_t columnIndex = idCiftiConnMatrix->getMatrixColumnIndex();
                if ((rowIndex >= 0)
                    || (columnIndex >= 0)) {
                    ciftiConnectivityManager->loadRowOrColumnFromConnectivityMatrixFile(brain,
                                                                                        matrixFile,
                                                                                        rowIndex,
                                                                                        columnIndex,
                                                                                        ciftiLoadingInfo);
                    updateGraphicsFlag = true;
                
                }
            }
        }
        
        /*
         * Generate identification manager
         */
        const AString identificationMessage = selectionManager->getIdentificationText(brain);
        
        bool issuedIdentificationLocationEvent = false;
        if (idNode->isValid()) {
            Surface* surface = idNode->getSurface();
            const int32_t nodeIndex = idNode->getNodeNumber();
            
            /*
             * Save last selected node which may get used for foci creation.
             */
            selectionManager->setLastSelectedItem(idNode);
            
            
            BrainStructure* brainStructure = surface->getBrainStructure();
            CaretAssert(brainStructure);
            
            float xyz[3];
            const Surface* primaryAnatomicalSurface = brainStructure->getPrimaryAnatomicalSurface();
            if (primaryAnatomicalSurface != NULL) {
                primaryAnatomicalSurface->getCoordinate(nodeIndex,
                                                        xyz);
            }
            else {
                CaretLogWarning("No surface/primary anatomical surface for "
                                + StructureEnum::toGuiName(brainStructure->getStructure()));
                xyz[0] = -10000000.0;
                xyz[1] = -10000000.0;
                xyz[2] = -10000000.0;
            }
            
            identifiedItem = new IdentifiedItemNode(identificationMessage,
                                                    surface->getStructure(),
                                                    surface->getNumberOfNodes(),
                                                    nodeIndex);
            /*
             * Only issue identification event for node 
             * if it WAS NOT created from the voxel identification
             * location.
             */
            if (nodeIdentificationCreatedFromVoxelIdentificationFlag == false) {
                if (issuedIdentificationLocationEvent == false) {
                    EventIdentificationHighlightLocation idLocation(tabIndex,
                                                                    xyz,
                                                                    EventIdentificationHighlightLocation::LOAD_FIBER_ORIENTATION_SAMPLES_MODE_YES);
                    EventManager::get()->sendEvent(idLocation.getPointer());
                    issuedIdentificationLocationEvent = true;
                }
            }
        }
        
        if (idVoxel->isValid()) {
            const VolumeMappableInterface* volumeFile = idVoxel->getVolumeFile();
            int64_t voxelIJK[3];
            idVoxel->getVoxelIJK(voxelIJK);
            if (volumeFile != NULL) {
                float xyz[3];
                volumeFile->indexToSpace(voxelIJK, xyz);
                
                if (identifiedItem == NULL) {
                    identifiedItem = new IdentifiedItemVoxel(identificationMessage,
                                                             xyz);
                }
                
                if (issuedIdentificationLocationEvent == false) {
                    EventIdentificationHighlightLocation idLocation(tabIndex,
                                                                    xyz,
                                                                    EventIdentificationHighlightLocation::LOAD_FIBER_ORIENTATION_SAMPLES_MODE_YES);
                    EventManager::get()->sendEvent(idLocation.getPointer());
                    issuedIdentificationLocationEvent = true;
                }
                
                /*
                 * Save last selected node which may get used for foci creation.
                 */
                selectionManager->setLastSelectedItem(idVoxel);
            }
        }
        
        if (identifiedItem == NULL) {
            if (identificationMessage.isEmpty() == false) {
                identifiedItem = new IdentifiedItem(identificationMessage);
            }
        }
        
        AString ciftiInfo;
        if (ciftiLoadingInfo.empty() == false) {
            IdentificationStringBuilder ciftiIdStringBuilder;
            ciftiIdStringBuilder.addLine(false, "CIFTI data loaded", " ");
            for (std::vector<AString>::iterator iter = ciftiLoadingInfo.begin();
                 iter != ciftiLoadingInfo.end();
                 iter++) {
                ciftiIdStringBuilder.addLine(true, *iter);
            }
            
            ciftiInfo = ciftiIdStringBuilder.toString();
        }
        if (ciftiInfo.isEmpty() == false) {
            if (identifiedItem != NULL) {
                identifiedItem->appendText(ciftiInfo);
            }
            else {
                identifiedItem = new IdentifiedItem(ciftiInfo);
            }
        }
        
        if (identifiedItem != NULL) {
            identificationManager->addIdentifiedItem(identifiedItem);
            updateInformationFlag = true;
        }
    }
    
    if (updateInformationFlag) {
        EventManager::get()->sendEvent(EventUpdateInformationWindows().getPointer());
    }
    
    if (updateGraphicsFlag) {
        EventManager::get()->sendEvent(EventSurfaceColoringInvalidate().getPointer());
        EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
        EventManager::get()->sendEvent(EventUserInterfaceUpdate().addToolBar().addToolBox().getPointer());
    }
} // tabIndex



