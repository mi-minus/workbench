/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
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

#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTabBar>

#include "Brain.h"
#include "BrainBrowserWindow.h"
#include "BrainBrowserWindowToolBar.h"
#include "BrainBrowserWindowToolBox.h"
#include "BrainOpenGLWidget.h"
#include "BrowserTabContent.h"
#include "CaretAssert.h"
#include "CaretPreferences.h"
#include "EventBrowserWindowNew.h"
#include "CaretLogger.h"
#include "EventDataFileRead.h"
#include "EventManager.h"
#include "EventGraphicsUpdateAllWindows.h"
#include "EventGraphicsUpdateOneWindow.h"
#include "EventPaletteColorMappingEditor.h"
#include "EventSpecFileReadDataFiles.h"
#include "EventSurfaceColoringInvalidate.h"
#include "EventUserInterfaceUpdate.h"
#include "FileInformation.h"
#include "GuiManager.h"
#include "ManageLoadedFilesDialog.h"
#include "PaletteColorMappingEditorDialog.h"
#include "SessionManager.h"
#include "SpecFile.h"
#include "SpecFileDialog.h"
#include "WuQFileDialog.h"
#include "WuQtUtilities.h"

using namespace caret;

/**
 * Constructor.
 *
 * @param browserWindowIndex
 *    Index for this window.
 * @param browserTabContent  
 *    If not NULL, this is the tab displayed in the window.
 *    If NULL, a new tab is created.
 * @param parent
 *    Parent of this object
 * @param flags
 *    Flags for Qt.
 */
BrainBrowserWindow::BrainBrowserWindow(const int browserWindowIndex,
                                       BrowserTabContent* browserTabContent,
                                       QWidget* parent,
                                       Qt::WindowFlags flags)
: QMainWindow(parent, flags)
{
    this->paletteColorMappingEditor = NULL;
    GuiManager* guiManager = GuiManager::get();
    
    this->setAttribute(Qt::WA_DeleteOnClose);
    
    this->browserWindowIndex = browserWindowIndex;
    
    this->setWindowTitle(guiManager->applicationName() 
                         + " "
                         + AString::number(this->browserWindowIndex + 1));
    
    this->openGLWidget = new BrainOpenGLWidget(this,
                                               browserWindowIndex);
    
    const int openGLSizeX = 500;
    const int openGLSizeY = 375;
    this->openGLWidget->setMinimumSize(openGLSizeX, 
                                       openGLSizeY);
    
    this->setCentralWidget(this->openGLWidget);
    
    this->toolBox = new BrainBrowserWindowToolBox(this->browserWindowIndex,
                                                  ("ToolBox " + AString::number(this->browserWindowIndex + 1)),
                                                  Qt::Horizontal,
                                                  this);
    this->addDockWidget(Qt::BottomDockWidgetArea,
                        this->toolBox,
                        Qt::Horizontal);
    QObject::connect(this->toolBox, SIGNAL(controlRemoved()),
                     this, SLOT(shrinkToolbox()));
    
    this->toolbar = new BrainBrowserWindowToolBar(this->browserWindowIndex,
                                                  browserTabContent,
                                                  this->toolBox->toggleViewAction(),
                                                  this);
    this->addToolBar(this->toolbar);
    
    this->createActions();
    
    this->createMenus();
    
    this->toolbar->updateToolBar();

    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_USER_INTERFACE_UPDATE);
    EventManager::get()->addEventListener(this, EventTypeEnum::EVENT_PALETTE_COLOR_MAPPING_EDITOR);
}

/**
 * Destructor.
 */
BrainBrowserWindow::~BrainBrowserWindow()
{
    EventManager::get()->removeAllEventsFromListener(this);    
}

/**
 * @return the index of this browser window.
 */
int32_t 
BrainBrowserWindow::getBrowserWindowIndex() const 
{ 
    return this->browserWindowIndex; 
}

/**
 * Called when the window is requested to close.
 *
 * @param event
 *     CloseEvent that may or may not be accepted
 *     allowing the window to close.
 */
void 
BrainBrowserWindow::closeEvent(QCloseEvent* event)
{
    /*
     * The GuiManager may warn user about closing the 
     * window and the user may cancel closing of the window.
     */
    GuiManager* guiManager = GuiManager::get();
    if (guiManager->allowBrainBrowserWindowToClose(this,
                                                   this->toolbar->tabBar->count())) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

/**
 * Create actions for this window.
 */
void 
BrainBrowserWindow::createActions()
{
    CaretAssert(this->toolbar);
    
    GuiManager* guiManager = GuiManager::get();
    
    this->newWindowAction =
    WuQtUtilities::createAction("New Window",
                                "Creates a new window for viewing brain models",
                                Qt::CTRL+Qt::Key_N,
                                this,
                                this,
                                SLOT(processNewWindow()));
    
    this->newTabAction =
    WuQtUtilities::createAction("New Tab", 
                                "Create a new tab (window pane) in the window",
                                Qt::CTRL + Qt::Key_T,
                                this,
                                this->toolbar,
                                SLOT(addNewTab()));
    
    this->openFileAction =
    WuQtUtilities::createAction("Open File...", 
                                "Open a data file including a spec file",
                                Qt::CTRL+Qt::Key_O,
                                this,
                                this,
                                SLOT(processDataFileOpen()));
    
    this->openFileViaSpecFileAction =
    WuQtUtilities::createAction("Open File via Spec File...", 
                                "Open a data file listed in the Spec File",
                                Qt::CTRL + Qt::SHIFT + Qt::Key_O,
                                this,
                                this,
                                SLOT(processDataFileOpenFromSpecFile()));
    this->openFileViaSpecFileAction->setEnabled(false);
    
    
    this->manageFilesAction =
    WuQtUtilities::createAction("Manage/Save Files...", 
                                "Manage and Save Loaded Files",
                                Qt::CTRL + Qt::Key_M,
                                this,
                                this,
                                SLOT(processManageSaveLoadedFiles()));
    
    this->closeSpecFileAction =
    WuQtUtilities::createAction("Close Spec File",
                                "Close the Spec File",
                                this,
                                this,
                                SLOT(processCloseSpecFile()));
    
    this->closeTabAction =
    WuQtUtilities::createAction("Close Tab",
                                "Close the active tab (window pane) in the window",
                                Qt::CTRL + Qt::Key_W,
                                this,
                                this->toolbar,
                                SLOT(closeSelectedTab()));
    
    this->closeWindowAction = 
    WuQtUtilities::createAction("Close Window",
                                "Close the window",
                                Qt::CTRL + Qt::SHIFT + Qt::Key_W,
                                this,
                                this,
                                SLOT(close()));
    
    this->captureImageAction =
    WuQtUtilities::createAction("Capture Image...",
                                "Capture an Image of the windows content",
                                this,
                                this,
                                SLOT(processCaptureImage()));
    
    this->preferencesAction = 
    WuQtUtilities::createAction("Preferences...",
                                "Edit the preferences",
                                this,
                                this,
                                SLOT(processEditPreferences()));
    
    this->exitProgramAction =
    WuQtUtilities::createAction("Exit", 
                                "Exit (quit) the program",
                                Qt::CTRL+Qt::Key_Q, 
                                this,
                                this,
                                SLOT(processExitProgram()));
    
    this->montageTabsAction =
    WuQtUtilities::createAction("Montage Tabs",
                                "Show all tab content in a grid",
                                this,
                                this,
                                SLOT(processToggleMontageTabs()));
    this->montageTabsAction->blockSignals(true);
    this->montageTabsAction->setCheckable(true);
    this->montageTabsAction->setChecked(false);
    this->montageTabsAction->blockSignals(false);
    this->montageTabsAction->setEnabled(false);
    
    this->showToolBarAction =
    WuQtUtilities::createAction("Toolbar", 
                                "Show or hide the toolbar",
                                this,
                                this->toolbar,
                                SLOT(showHideToolBar(bool)));
    this->showToolBarAction->setCheckable(true);
    this->showToolBarAction->setChecked(true);
    
    this->viewFullScreenAction =
    WuQtUtilities::createAction("View Full Screen",
                                "Fill the screen with only the graphics area",
                                Qt::CTRL + Qt::Key_F,
                                this,
                                this,
                                SLOT(processViewFullScreen()));
    this->viewFullScreenAction->blockSignals(true);
    this->viewFullScreenAction->setCheckable(true);
    this->viewFullScreenAction->setChecked(false);
    this->viewFullScreenAction->blockSignals(false);
    
    this->nextTabAction =
    WuQtUtilities::createAction("Next Tab",
                                "Move to the next tab",
                                Qt::CTRL + Qt::Key_Right,
                                this,
                                this->toolbar,
                                SLOT(nextTab()));
    
    this->previousTabAction =
    WuQtUtilities::createAction("Previous Tab",
                                "Move to the previous tab",
                                Qt::CTRL + Qt::Key_Left,
                                this,
                                this->toolbar,
                                SLOT(previousTab()));
    
    this->renameSelectedTabAction =
    WuQtUtilities::createAction("Rename Selected Tab...",
                                "Change the name of the selected tab",
                                this,
                                this->toolbar,
                                SLOT(renameTab()));
    
    this->moveTabsInWindowToNewWindowsAction =
    WuQtUtilities::createAction("Move All Tabs in Current Window to New Windows",
                                "Move all but the left most tab to new windows",
                                this,
                                this->toolbar,
                                SLOT(moveTabsToNewWindows()));
    
    this->moveTabsFromAllWindowsToOneWindowAction =
    WuQtUtilities::createAction("Move All Tabs From All Windows Into Selected Window",
                                "Move all tabs from all windows into selected window",
                                this,
                                this,
                                SLOT(processMoveAllTabsToOneWindow()));
    
    this->bringAllToFrontAction =
    WuQtUtilities::createAction("Bring All To Front",
                                "Move all windows on top of other application windows",
                                this,
                                guiManager,
                                SLOT(processBringAllWindowsToFront()));
    
    this->identifyWindowAction =
    WuQtUtilities::createAction("Identify Window...",
                                "Show the Identify Window",
                                Qt::CTRL + Qt::Key_I,
                                this,
                                guiManager,
                                SLOT(processShowIdentifyWindow()));
    this->identifyWindowAction->setEnabled(false);
    
    this->dataDisplayAction =
    WuQtUtilities::createAction("Data Display...",
                                "Show the Data Display Window",
                                Qt::CTRL + Qt::Key_D,
                                this,
                                guiManager,
                                SLOT(processShowDataDisplayWindow()));
    this->dataDisplayAction->setEnabled(false);
    
    this->helpOnlineAction =
    WuQtUtilities::createAction("Show Help (Online)...",
                                "Show the Help Window",
                                this,
                                guiManager,
                                SLOT(processShowHelpOnlineWindow()));
    this->helpOnlineAction->setEnabled(false);
    
    this->searchHelpOnlineAction =
    WuQtUtilities::createAction("Search Help (Online)...",
                                "Show the Search Helper Window",
                                this,
                                guiManager,
                                SLOT(processShowSearchHelpOnlineWindow()));
    this->searchHelpOnlineAction->setEnabled(false);
    
}

/**
 * Create menus for this window.
 */
void 
BrainBrowserWindow::createMenus()
{
    /*
     * Create the menu bar and add menus to it.
     */
    QMenuBar* menuBar = this->menuBar();
    menuBar->addMenu(this->createMenuFile());
    menuBar->addMenu(this->createMenuView());
    menuBar->addMenu(this->createMenuData());
    menuBar->addMenu(this->createMenuSurface());
    menuBar->addMenu(this->createMenuVolume());
    menuBar->addMenu(this->createMenuWindow());
    menuBar->addMenu(this->createMenuHelp());    
}

/**
 * Create the file menu.
 * @return the file menu.
 */
QMenu* 
BrainBrowserWindow::createMenuFile()
{
    QMenu* menu = new QMenu("File", this);

    menu->addAction(this->preferencesAction);
#ifndef CARET_OS_MACOSX
    menu->addSeparator();
#endif // CARET_OS_MACOSX
    menu->addAction(this->newWindowAction);
    menu->addAction(this->newTabAction);
    menu->addSeparator();
    menu->addAction(this->openFileAction);
    this->recentSpecFileMenu = menu->addMenu("Open Recent Spec File");
    QObject::connect(this->recentSpecFileMenu, SIGNAL(aboutToShow()),
                     this, SLOT(processRecentSpecFileMenuAboutToBeDisplayed()));
    QObject::connect(this->recentSpecFileMenu, SIGNAL(triggered(QAction*)),
                     this, SLOT(processRecentSpecFileMenuSelection(QAction*)));
    menu->addAction(this->openFileViaSpecFileAction);
    menu->addAction(this->manageFilesAction);
    menu->addAction(this->closeSpecFileAction);
    menu->addSeparator();
    menu->addAction(this->captureImageAction);
    menu->addSeparator();
    menu->addAction(this->closeTabAction);
    menu->addAction(this->closeWindowAction);
    menu->addSeparator();
#ifndef CARET_OS_MACOSX
    menu->addSeparator();
#endif // CARET_OS_MACOSX
    menu->addAction(this->exitProgramAction);
    
    return menu;
}

/**
 * Called when Open Recent Spec File Menu is about to be displayed
 * and creates the content of the menu.
 */
void 
BrainBrowserWindow::processRecentSpecFileMenuAboutToBeDisplayed()
{
    this->recentSpecFileMenu->clear();
    
    CaretPreferences* prefs = SessionManager::get()->getCaretPreferences();
    std::vector<AString> recentSpecFiles;
    prefs->getPreviousSpecFiles(recentSpecFiles);
    
    const int32_t numRecentSpecFiles = static_cast<int>(recentSpecFiles.size());
    for (int32_t i = 0; i < numRecentSpecFiles; i++) {
        FileInformation fileInfo(recentSpecFiles[i]);
        QString path = fileInfo.getPathName();
        QString name = fileInfo.getFileName();
        if (path.isEmpty() == false) {
            name += (" (" + path + ")");
        }
        QString fullPath = fileInfo.getFilePath();
        
        QAction* action = new QAction(name,
                                      this->recentSpecFileMenu);
        action->setData(fullPath);
        this->recentSpecFileMenu->addAction(action);
    } 
    
    if (numRecentSpecFiles > 0) {
        this->recentSpecFileMenu->addSeparator();
        QAction* action = new QAction("Clear Menu",
                                      this->recentSpecFileMenu);
        action->setData("CLEAR_CLEAR");
        this->recentSpecFileMenu->addAction(action);
    }
}

/**
 * Called when an item is selected from the recent spec file
 * menu.
 * @param itemAction
 *    Action of the menu item that was selected.
 */
void 
BrainBrowserWindow::processRecentSpecFileMenuSelection(QAction* itemAction)
{
    AString errorMessages;
    
    const AString specFileName = itemAction->data().toString();
    if (specFileName == "CLEAR_CLEAR") {
        CaretPreferences* prefs = SessionManager::get()->getCaretPreferences();
        prefs->clearPreviousSpecFiles();
        return;
    }
    
    if (specFileName.isEmpty() == false) {
        SpecFile specFile;
        try {
            specFile.readFile(specFileName);
        }
        catch (DataFileException e) {
            errorMessages += e.whatString();
        }
        
        SpecFileDialog sfd(&specFile,
                           this);
        if (sfd.exec() == QDialog::Accepted) {
            EventSpecFileReadDataFiles readSpecFileEvent(GuiManager::get()->getBrain(),
                                                         &specFile);
            
            EventManager::get()->sendEvent(readSpecFileEvent.getPointer());
            
            if (readSpecFileEvent.isError()) {
                if (errorMessages.isEmpty() == false) {
                    errorMessages += "\n";
                }
                errorMessages += readSpecFileEvent.getErrorMessage();
            }
        }
        
        this->toolbar->addDefaultTabsAfterLoadingSpecFile();
        
        if (errorMessages.isEmpty() == false) {
            QMessageBox::critical(this, 
                                  "ERROR", 
                                  errorMessages);
        }
        
        EventManager::get()->sendEvent(EventSurfaceColoringInvalidate().getPointer());
        EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
        EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
    }
}

/**
 * Create the view menu.
 * @return the view menu.
 */
QMenu* 
BrainBrowserWindow::createMenuView()
{
    QMenu* menu = new QMenu("View", this);
    
    menu->addAction(this->montageTabsAction);
    menu->addSeparator();
    menu->addAction(this->showToolBarAction);
    QAction* showToolBoxAction = this->toolbar->getShowToolBoxAction();
    if (showToolBoxAction != NULL) {
        menu->addAction(showToolBoxAction);
    }
    else {
        CaretLogSevere("Show toolbox action needs to be created (is NULL)");
    }
    menu->addMenu(this->createMenuViewMoveToolBox());
    menu->addSeparator();
    menu->addAction(this->viewFullScreenAction);
    
    return menu;
}

/**
 * Create the toolbox menu.
 * @return the toolbox menu.
 */
QMenu* 
BrainBrowserWindow::createMenuViewMoveToolBox()
{
    QMenu* menu = new QMenu("Move Toolbox", this);
    
    menu->addAction("Float", this, SLOT(processMoveToolBoxToFloat()));
    menu->addSeparator();
    menu->addAction("Bottom", this, SLOT(processMoveToolBoxToBottom()));
    menu->addAction("Left", this, SLOT(processMoveToolBoxToLeft()));
    menu->addAction("Right", this, SLOT(processMoveToolBoxToRight()));
    menu->addAction("Top", this, SLOT(processMoveToolBoxToTop()));
    
    return menu;
}

/**
 * Create the data menu.
 * @return the data menu.
 */
QMenu* 
BrainBrowserWindow::createMenuData()
{
    QMenu* menu = new QMenu("Data", this);
    
    return menu;
}

/**
 * Create the surface menu.
 * @return the surface menu.
 */
QMenu* 
BrainBrowserWindow::createMenuSurface()
{
    QMenu* menu = new QMenu("Surface", this);
    
    return menu;
}

/**
 * Create the volume menu.
 * @return the volume menu.
 */
QMenu* 
BrainBrowserWindow::createMenuVolume()
{
    QMenu* menu = new QMenu("Volume", this);
    
    return menu;
}

/**
 * Create the window menu.
 * @return the window menu.
 */
QMenu* 
BrainBrowserWindow::createMenuWindow()
{
    this->moveSelectedTabToWindowMenu = new QMenu("Move Selected Tab to Window");
    QObject::connect(this->moveSelectedTabToWindowMenu, SIGNAL(aboutToShow()),
                     this, SLOT(processMoveSelectedTabToWindowMenuAboutToBeDisplayed()));
    QObject::connect(this->moveSelectedTabToWindowMenu, SIGNAL(triggered(QAction*)),
                     this, SLOT(processMoveSelectedTabToWindowMenuSelection(QAction*)));
    
    QMenu* menu = new QMenu("Window", this);
    
    menu->addAction(this->nextTabAction);
    menu->addAction(this->previousTabAction);
    menu->addAction(this->renameSelectedTabAction);
    menu->addSeparator();
    menu->addAction(this->moveTabsInWindowToNewWindowsAction);
    menu->addAction(this->moveTabsFromAllWindowsToOneWindowAction);
    menu->addMenu(this->moveSelectedTabToWindowMenu);
    menu->addSeparator();
    menu->addAction(this->dataDisplayAction);
    menu->addAction(this->identifyWindowAction);
    menu->addSeparator();
    menu->addAction(this->bringAllToFrontAction);
    
    return menu;
}

/**
 * Create the help menu.
 * @return the help menu.
 */
QMenu* 
BrainBrowserWindow::createMenuHelp()
{
    QMenu* menu = new QMenu("Help", this);
    
    menu->addAction(this->helpOnlineAction);
    menu->addAction(this->searchHelpOnlineAction);
    
    return menu;
}

/**
 * Called when capture image is selected.
 */
void 
BrainBrowserWindow::processCaptureImage()
{
    GuiManager::get()->processShowImageCaptureDialog(this);
}

/**
 * Called when capture image is selected.
 */
void 
BrainBrowserWindow::processEditPreferences()
{
    GuiManager::get()->processShowPreferencesDialog(this);
}

/**
 * Called when close spec file is selected.
 */
void 
BrainBrowserWindow::processCloseSpecFile()
{
    Brain* brain = GuiManager::get()->getBrain();
    brain->resetBrain();
    EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
}

/**
 * Create a new window.
 */
void 
BrainBrowserWindow::processNewWindow()
{
    EventBrowserWindowNew eventNewBrowser(this, NULL);
    EventManager::get()->sendEvent(eventNewBrowser.getPointer());
    EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
}

/**
 * Called when open data file is selected.
 */
void 
BrainBrowserWindow::processDataFileOpen()
{
    if (this->previousOpenFileNameFilter.isEmpty()) {
        this->previousOpenFileNameFilter = 
            DataFileTypeEnum::toQFileDialogFilter(DataFileTypeEnum::SPECIFICATION);
    }
    
    /*
     * Get all file filters.
     */
    std::vector<DataFileTypeEnum::Enum> dataFileTypes;
    DataFileTypeEnum::getAllEnums(dataFileTypes, false, false);
    QStringList filenameFilterList;
    filenameFilterList.append("Any File (*)");
    for (std::vector<DataFileTypeEnum::Enum>::const_iterator iter = dataFileTypes.begin();
         iter != dataFileTypes.end();
         iter++) {
        AString filterName = DataFileTypeEnum::toQFileDialogFilter(*iter);
        filenameFilterList.append(filterName);
    }
    
    /*
     * Setup file selection dialog.
     */
    WuQFileDialog fd(this);
    fd.setAcceptMode(WuQFileDialog::AcceptOpen);
    fd.setNameFilters(filenameFilterList);
    fd.setFileMode(WuQFileDialog::ExistingFiles);
    fd.setViewMode(WuQFileDialog::List);
    fd.selectFilter(this->previousOpenFileNameFilter);
    
    AString errorMessages;
    
    if (fd.exec()) {
        QStringList selectedFiles = fd.selectedFiles();
        this->previousOpenFileNameFilter = fd.selectedFilter();
        
        /*
         * Load each file.
         */
        QStringListIterator nameIter(selectedFiles);
        while (nameIter.hasNext()) {
            AString name = nameIter.next();
            
            //FileInformation fileInfo(name);
            //if (fileInfo.isAbsolute()) {
            //    prefs->addToPreviousOpenFileDirectories(fileInfo.getPathName());
            //}
            
            bool isValidType = false;
            DataFileTypeEnum::Enum fileType = DataFileTypeEnum::fromFileExtension(name, &isValidType);
            if (isValidType) {
                if (fileType == DataFileTypeEnum::SPECIFICATION) {
                    SpecFile specFile;
                    try {
                        specFile.readFile(name);
                    }
                    catch (DataFileException e) {
                        errorMessages += e.whatString();
                    }
                    
                    SpecFileDialog sfd(&specFile,
                                       this);
                    if (sfd.exec() == QDialog::Accepted) {
                        EventSpecFileReadDataFiles readSpecFileEvent(GuiManager::get()->getBrain(),
                                                                     &specFile);
                        
                        EventManager::get()->sendEvent(readSpecFileEvent.getPointer());
                        
                        if (readSpecFileEvent.isError()) {
                            if (errorMessages.isEmpty() == false) {
                                errorMessages += "\n";
                            }
                            errorMessages += readSpecFileEvent.getErrorMessage();
                        }
                    }
                    
                    this->toolbar->addDefaultTabsAfterLoadingSpecFile();
                }
                else {
                    EventDataFileRead loadFileEvent(GuiManager::get()->getBrain(),
                                                    fileType,
                                                    name);
                    
                    EventManager::get()->sendEvent(loadFileEvent.getPointer());
                    
                    if (loadFileEvent.isError()) {
                        if (errorMessages.isEmpty() == false) {
                            errorMessages += "\n";
                        }
                        errorMessages += loadFileEvent.getErrorMessage();
                    }                    
                }
            }
            else {
                if (errorMessages.isEmpty() == false) {
                    errorMessages += "\n";
                }
                errorMessages += ("Extension for " + name + " does not match a Caret file type");
            }
        }
        
    }

    if (errorMessages.isEmpty() == false) {
        QMessageBox::critical(this, 
                              "ERROR", 
                              errorMessages);
    }
    
    EventManager::get()->sendEvent(EventSurfaceColoringInvalidate().getPointer());
    EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
}

/**
 * Called when open data file from spec file is selected.
 */
void 
BrainBrowserWindow::processDataFileOpenFromSpecFile()
{
}

/**
 * Called when manage/save loaded files is selected.
 */
void 
BrainBrowserWindow::processManageSaveLoadedFiles()
{
    ManageLoadedFilesDialog manageLoadedFile(this,
                                             GuiManager::get()->getBrain());
    manageLoadedFile.exec();
}

/**
 * Exit the program.
 */
void 
BrainBrowserWindow::processExitProgram()
{
    GuiManager::get()->exitProgram(this);
}

/**
 * Called when toggle montage tabs is selected.
 */
void 
BrainBrowserWindow::processToggleMontageTabs()
{
    
}

/**
 * Restore the status of window components.
 * @param wcs
 *    Window component status that is restored.
 */
void 
BrainBrowserWindow::restoreWindowComponentStatus(const WindowComponentStatus& wcs)
{
    if (this->showToolBarAction->isChecked() != wcs.isToolBarDisplayed) {
        this->showToolBarAction->trigger();
    }
    if (this->toolBox->toggleViewAction()->isChecked() != wcs.isToolBoxDisplayed) {
        this->toolBox->toggleViewAction()->trigger();
    }
}

/**
 * Save the status of window components.
 * @param wcs
 *    Will contains status after exit.
 * @param hideComponents
 *    If true, any components (toolbar/toolbox) will be hidden.
 */
void 
BrainBrowserWindow::saveWindowComponentStatus(WindowComponentStatus& wcs,
                                              bool hideComponents)
{
    wcs.isToolBarDisplayed = this->showToolBarAction->isChecked();
    wcs.isToolBoxDisplayed = this->toolBox->toggleViewAction()->isChecked();
    
    if (hideComponents) {
        if (wcs.isToolBarDisplayed) {
            this->showToolBarAction->trigger();
        }
        if (wcs.isToolBoxDisplayed) {
            this->toolBox->toggleViewAction()->trigger();
        }
    }
}

/**
 * Called when view full screen is selected.
 */
void 
BrainBrowserWindow::processViewFullScreen()
{
    if (this->isFullScreen()) {
        this->showNormal();
        this->restoreWindowComponentStatus(this->fullScreenEnteredWindowComponentStatus);
    }
    else {
        this->showFullScreen();
        this->saveWindowComponentStatus(this->fullScreenEnteredWindowComponentStatus, true);
    }
}

/**
 * Called when move all tabs to one window is selected.
 */
void 
BrainBrowserWindow::processMoveAllTabsToOneWindow()
{
    std::vector<BrowserTabContent*> otherTabContent;
    GuiManager::get()->closeOtherWindowsAndReturnTheirTabContent(this,
                                                                 otherTabContent);
    
    const int32_t numOtherTabs = static_cast<int32_t>(otherTabContent.size());
    for (int32_t i = 0; i < numOtherTabs; i++) {
        this->toolbar->addNewTab(otherTabContent[i]);
        this->toolbar->updateToolBar();
    }
    
    EventManager::get()->sendEvent(EventSurfaceColoringInvalidate().getPointer());
    EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
    EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(this->browserWindowIndex).getPointer());
}

/**
 * Called when the move tab to window is about to be displayed.
 */
void 
BrainBrowserWindow::processMoveSelectedTabToWindowMenuAboutToBeDisplayed()
{
    this->moveSelectedTabToWindowMenu->clear();

    if (this->getBrowserTabContent() == NULL) {
        return;
    }
    
    /*
     * Allow movement of tab to new window ONLY if this window
     * contains more than one tab.
     */
    if (this->toolbar->tabBar->count() > 1) {
        QAction* toNewWindowAction = new QAction("New Window",
                                                 this->moveSelectedTabToWindowMenu);
        toNewWindowAction->setData(qVariantFromValue((void*)NULL));
        this->moveSelectedTabToWindowMenu->addAction(toNewWindowAction);
    }
    
    std::vector<BrainBrowserWindow*> browserWindows = GuiManager::get()->getAllBrainBrowserWindows();
    for (int32_t i = 0; i < static_cast<int32_t>(browserWindows.size()); i++) {
        if (browserWindows[i] != this) {
            QString name = "Window " + QString::number(browserWindows[i]->getBrowserWindowIndex() + 1);
            QAction* action = new QAction(name,
                                          this->moveSelectedTabToWindowMenu);
            action->setData(qVariantFromValue((void*)browserWindows[i]));
            this->moveSelectedTabToWindowMenu->addAction(action);
        }
    }    
}

/**
 * Called when move tab to window menu item is selected.
 * This window may close if there are no more tabs after
 * the tab is removed.
 * @param action
 *    Action from menu item that was selected.
 */
void 
BrainBrowserWindow::processMoveSelectedTabToWindowMenuSelection(QAction* action)
{
    if (action != NULL) {
        void* p = action->data().value<void*>();
        BrainBrowserWindow* moveToBrowserWindow = (BrainBrowserWindow*)p;
        
        BrowserTabContent* btc = this->getBrowserTabContent();
        
        this->toolbar->removeTabWithContent(btc);
            
        if (moveToBrowserWindow != NULL) {
            moveToBrowserWindow->toolbar->addNewTab(btc);
        }
        else {
            EventBrowserWindowNew newWindow(this,
                                            btc);
            EventManager::get()->sendEvent(newWindow.getPointer());
        }
        
        if (this->toolbar->tabBar->count() <= 0) {
            EventManager::get()->removeAllEventsFromListener(this);  // ignore update requests
            this->close();
        }
    }
}

/**
 * Called to move the toolbox to the left side of the window.
 */
void 
BrainBrowserWindow::processMoveToolBoxToLeft()
{
    this->moveToolBox(Qt::LeftDockWidgetArea);
}

/**
 * Called to move the toolbox to the right side of the window.
 */
void 
BrainBrowserWindow::processMoveToolBoxToRight()
{
    this->moveToolBox(Qt::RightDockWidgetArea);
}

/**
 * Called to move the toolbox to the top side of the window.
 */
void 
BrainBrowserWindow::processMoveToolBoxToTop()
{
    this->moveToolBox(Qt::TopDockWidgetArea);
}

/**
 * Called to move the toolbox to the bottom side of the window.
 */
void 
BrainBrowserWindow::processMoveToolBoxToBottom()
{
    this->moveToolBox(Qt::BottomDockWidgetArea);
}

/**
 * Called to move the toolbox to float outside of the window.
 */
void 
BrainBrowserWindow::processMoveToolBoxToFloat()
{
    this->moveToolBox(Qt::NoDockWidgetArea);
}

void 
BrainBrowserWindow::moveToolBox(Qt::DockWidgetArea area)
{
        switch (area) {
            case Qt::LeftDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::LeftDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            case Qt::RightDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::RightDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            case Qt::TopDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::TopDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            case Qt::BottomDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::BottomDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            default:
                this->toolBox->setFloating(true);
                break;
    }

    if (this->toolBox->isFloating() == false) {
        this->shrinkToolbox();
    }
    /*
     * This code will allow the region of the main window
     * containing the dock widget to shrink without changing 
     * the vertical size of the OpenGL graphics widget
     * and without changing the width of the main window.
     *
    const int centralMinHeight = this->centralWidget()->minimumHeight();
    const int centralMaxHeight = this->centralWidget()->maximumHeight();
    this->centralWidget()->setFixedHeight(this->centralWidget()->height());
    const int minWidth = this->minimumWidth();
    const int maxWidth = this->maximumWidth();
    this->setFixedWidth(this->width());
    this->adjustSize();
    this->setMinimumWidth(minWidth);
    this->setMaximumWidth(maxWidth);
    this->centralWidget()->setMinimumHeight(centralMinHeight);
    this->centralWidget()->setMaximumHeight(centralMaxHeight);
*/
    /*
    switch (area) {
        case Qt::LeftDockWidgetArea:
            break;
        case Qt::RightDockWidgetArea:
            break;
        case Qt::TopDockWidgetArea:
            this->toolBox->setFloating(false);
            this->addDockWidget(Qt::TopDockWidgetArea, this->toolBox);
            break;
        case Qt::BottomDockWidgetArea:
            this->toolBox->setFloating(false);
            this->addDockWidget(Qt::BottomDockWidgetArea, this->toolBox);
            break;
        default:
            this->toolBox->setFloating(true);
            break;
    }
*/
}

/**
 * Shrink the toolbox after a control is removed.
 */
void 
BrainBrowserWindow::shrinkToolbox()
{
    if (this->toolBox->isFloating() == false) {
        switch (this->dockWidgetArea(this->toolBox)) {
            case Qt::LeftDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::LeftDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            case Qt::RightDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::LeftDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            case Qt::TopDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::TopDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            case Qt::BottomDockWidgetArea:
                this->toolBox->setFloating(false);
                this->addDockWidget(Qt::BottomDockWidgetArea, 
                                    this->toolBox,
                                    Qt::Horizontal);
                break;
            default:
                this->toolBox->setFloating(true);
                break;
        }
    }
     /*
     * This code will allow the region of the main window
     * containing the dock widget to shrink without changing 
     * the vertical size of the OpenGL graphics widget
     * and without changing the width of the main window.
     */
     const int centralMinHeight = this->centralWidget()->minimumHeight();
     const int centralMaxHeight = this->centralWidget()->maximumHeight();
     this->centralWidget()->setFixedHeight(this->centralWidget()->height());
     const int minWidth = this->minimumWidth();
     const int maxWidth = this->maximumWidth();
     this->setFixedWidth(this->width());
     this->toolBox->adjustSize();
     this->setFixedWidth(this->width());
     this->adjustSize();
     this->setMinimumWidth(minWidth);
     this->setMaximumWidth(maxWidth);
     this->centralWidget()->setMinimumHeight(centralMinHeight);
     this->centralWidget()->setMaximumHeight(centralMaxHeight);
}

/**
 * Remove and return all tabs from this toolbar.
 * After this the window containing this toolbar 
 * will contain no tabs!
 *
 * @param allTabContent
 *    Will contain the content from the tabs upon return.
 */
void 
BrainBrowserWindow::removeAndReturnAllTabs(std::vector<BrowserTabContent*>& allTabContent)
{
    this->toolbar->removeAndReturnAllTabs(allTabContent);
}

/**
 * Receive events from the event manager.
 * 
 * @param event
 *   Event sent by event manager.
 */
void 
BrainBrowserWindow::receiveEvent(Event* event)
{
    if (event->getEventType() == EventTypeEnum::EVENT_USER_INTERFACE_UPDATE) {
        EventUserInterfaceUpdate* uiEvent =
        dynamic_cast<EventUserInterfaceUpdate*>(event);
        CaretAssert(uiEvent);
        
        this->toolbar->updateToolBar();
        
        uiEvent->setEventProcessed();
    }
    else if (event->getEventType() == EventTypeEnum::EVENT_PALETTE_COLOR_MAPPING_EDITOR) {
        EventPaletteColorMappingEditor* mapEditEvent =
        dynamic_cast<EventPaletteColorMappingEditor*>(event);
        CaretAssert(mapEditEvent);
        
        const int browserWindowIndex = mapEditEvent->getBrowserWindowIndex();
        if (browserWindowIndex == this->browserWindowIndex) {
            CaretMappableDataFile* mapFile = mapEditEvent->getCaretMappableDataFile();
            const int mapIndex = mapEditEvent->getMapIndex();
            
            if (this->paletteColorMappingEditor == NULL) {
                this->paletteColorMappingEditor =
                new PaletteColorMappingEditorDialog(this);
            }
            this->paletteColorMappingEditor->updatePaletteEditor(mapFile, mapIndex);
            this->paletteColorMappingEditor->show();
            this->paletteColorMappingEditor->raise();
            this->paletteColorMappingEditor->activateWindow();
            WuQtUtilities::moveWindowToSideOfParent(this,
                                                    this->paletteColorMappingEditor);
            mapEditEvent->setEventProcessed();
        }
        else {
            if (this->paletteColorMappingEditor != NULL) {
                this->paletteColorMappingEditor->hide();
            }
        }
    }
    else {
        
    }
}

/**
 * @return Return the active browser tab content in
 * this browser window.
 */
BrowserTabContent* 
BrainBrowserWindow::getBrowserTabContent()
{
    return this->toolbar->getTabContentFromSelectedTab();
}

/**
 * Returns a popup menu for the main window.
 * Overrides that in QMainWindow and prevents the 
 * default context menu from appearing.
 *
 * @return Context menu for display or NULL if
 * nothing available. 
 */
QMenu* 
BrainBrowserWindow::createPopupMenu()
{
    return NULL;
}

/**
 * Capture an image of the window's graphics area using 
 * the given image size.  If either of the image dimensions
 * is zero, the image will be the size of the graphcis 
 * area.
 *
 * @param imageSizeX
 *    Desired X size of image.
 * @param imageSizeY
 *    Desired X size of image.
 * @return
 *    An image of the graphics area.
 */
QImage 
BrainBrowserWindow::captureImageOfGraphicsArea(const int32_t imageSizeX,
                                               const int32_t imageSizeY)
{
    QImage image = this->openGLWidget->captureImage(imageSizeX, 
                                                    imageSizeY);
    
    return image;
}


