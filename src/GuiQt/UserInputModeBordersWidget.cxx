
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

#include <QAction>
#include <QActionGroup>
#include <QBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QToolButton>


#define __USER_INPUT_MODE_BORDERS_WIDGET_DECLARE__
#include "UserInputModeBordersWidget.h"
#undef __USER_INPUT_MODE_BORDERS_WIDGET_DECLARE__

#include "CaretAssert.h"
#include "UserInputModeBorders.h"
#include "WuQtUtilities.h"

using namespace caret;



/**
 * \class UserInputModeBordersWidget 
 * \brief Controls for when the user input mode is border operations
 */

/**
 * Constructor.
 *
 * @param inputModeBorders
 *    Mouse processing for border operations.
 * @param parent
 *    Optional parent widget.
 */
UserInputModeBordersWidget::UserInputModeBordersWidget(UserInputModeBorders* inputModeBorders,
                                                       QWidget* parent)
: QWidget(parent)
{
    this->inputModeBorders = inputModeBorders;
    
    QLabel* nameLabel = new QLabel("Borders ");
    
    this->widgetMode = this->createModeWidget();
    
    this->widgetCreateOperation = this->createCreateOperationWidget();
    
    this->widgetEditOperation = this->createEditOperationWidget();
    
    this->widgetPointEditOperation = this->createPointEditOperationWidget();
    
    this->widgetUpdateOperation = this->createUpdateOperationWidget();
    
    this->operationStackedWidget = new QStackedWidget();
    this->operationStackedWidget->addWidget(this->widgetCreateOperation);
    this->operationStackedWidget->addWidget(this->widgetEditOperation);
    this->operationStackedWidget->addWidget(this->widgetPointEditOperation);
    this->operationStackedWidget->addWidget(this->widgetUpdateOperation);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    WuQtUtilities::setLayoutMargins(layout, 0, 0, 0);
    layout->addWidget(nameLabel);
    layout->addWidget(this->widgetMode);
    layout->addSpacing(10);
    layout->addWidget(this->operationStackedWidget);
    layout->addStretch();
}

/**
 * Destructor.
 */
UserInputModeBordersWidget::~UserInputModeBordersWidget()
{
    
}

/**
 * Update the contents of the widget.
 */
void 
UserInputModeBordersWidget::updateWidget()
{
    /*
     * Show the proper widget
     */
    switch (this->inputModeBorders->getMode()) {
        case UserInputModeBorders::MODE_CREATE:
            this->operationStackedWidget->setCurrentWidget(this->widgetCreateOperation);
            break;
        case UserInputModeBorders::MODE_EDIT:
            this->operationStackedWidget->setCurrentWidget(this->widgetEditOperation);
            break;
        case UserInputModeBorders::MODE_EDIT_POINTS:
            this->operationStackedWidget->setCurrentWidget(this->widgetPointEditOperation);
            break;
        case UserInputModeBorders::MODE_UPDATE:
            this->operationStackedWidget->setCurrentWidget(this->widgetUpdateOperation);
            break;
    }
    const int selectedModeInteger = (int)this->inputModeBorders->getMode();
    
    /*
     * Select the button for the mode
     */
    this->modeActionGroup->blockSignals(true);
    const QList<QAction*> modeActions = this->modeActionGroup->actions();
    QListIterator<QAction*> iter(modeActions);
    while (iter.hasNext()) {
        QAction* action = iter.next();
        const int modeInteger = action->data().toInt();
        if (selectedModeInteger == modeInteger) {
            action->setChecked(true);
            break;
        }
    }
    this->modeActionGroup->blockSignals(false);
}

/**
 * @return The mode widget.
 */
QWidget* 
UserInputModeBordersWidget::createModeWidget()
{
    QAction* createAction = WuQtUtilities::createAction("Create", "Create new borders", this);
    createAction->setCheckable(true);
    createAction->setData((int)UserInputModeBorders::MODE_CREATE);
    QToolButton* createToolButton = new QToolButton();
    createToolButton->setDefaultAction(createAction);
    
    QAction* editAction = WuQtUtilities::createAction("Edit", "Edit border attributes", this);
    editAction->setCheckable(true);
    editAction->setData((int)UserInputModeBorders::MODE_EDIT);
    QToolButton* editToolButton = new QToolButton();
    editToolButton->setDefaultAction(editAction);
    
    QAction* editPointsAction = WuQtUtilities::createAction("Points", "Edit border points", this);
    editPointsAction->setCheckable(true);
    editPointsAction->setData((int)UserInputModeBorders::MODE_EDIT_POINTS);
    QToolButton* editPointsToolButton = new QToolButton();
    editPointsToolButton->setDefaultAction(editPointsAction);
    
    QAction* updateAction = WuQtUtilities::createAction("Update", "Add/Remove/Replace segments of borders", this);
    updateAction->setCheckable(true);
    updateAction->setData((int)UserInputModeBorders::MODE_UPDATE);
    QToolButton* updateToolButton = new QToolButton();
    updateToolButton->setDefaultAction(updateAction);
    
    this->modeActionGroup = new QActionGroup(this);
    this->modeActionGroup->addAction(createAction);
    this->modeActionGroup->addAction(editAction);
    this->modeActionGroup->addAction(editPointsAction);
    this->modeActionGroup->addAction(updateAction);
    this->modeActionGroup->setExclusive(true);
    QObject::connect(this->modeActionGroup, SIGNAL(triggered(QAction*)),
                     this, SLOT(modeActionTriggered(QAction*)));
    
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    WuQtUtilities::setLayoutMargins(layout, 2, 0, 0);
    layout->addWidget(createToolButton);
    layout->addWidget(editToolButton);
    layout->addWidget(editPointsToolButton);
    layout->addWidget(updateToolButton);
    
    widget->setFixedWidth(widget->sizeHint().width());
    
    return widget;
}

/**
 * Called when a mode button is clicked.
 * @param action
 *    Action that was triggered.
 */
void 
UserInputModeBordersWidget::modeActionTriggered(QAction* action)
{
    CaretAssert(action);
    const int modeInteger = action->data().toInt();
    const UserInputModeBorders::Mode mode = (UserInputModeBorders::Mode)modeInteger;
    this->inputModeBorders->setMode(mode);
}

/**
 * @return The create operation widget.
 */
QWidget* 
UserInputModeBordersWidget::createCreateOperationWidget()
{
    QAction* drawAction = WuQtUtilities::createAction("Draw", 
                                                      "Start drawing a new border by either clicking\n"
                                                      "the mouse along the desired border path or by\n"
                                                      "moving the mouse with the left mouse button\n"
                                                      "depressed until the end point is reached.\n"
                                                      "Press the \"Finish\" button to complete the \n"
                                                      "border by setting its name and color", 
                                                      this);
    drawAction->setCheckable(true);
    QToolButton* drawToolButton = new QToolButton();
    drawToolButton->setDefaultAction(drawAction);
    
    QAction* finishAction = WuQtUtilities::createAction("Finish", 
                                                        "Finish drawing a new border by\n"
                                                        "setting the name and color", 
                                                        this);
    finishAction->setCheckable(true);
    QToolButton* finishToolButton = new QToolButton();
    finishToolButton->setDefaultAction(finishAction);
    
    QAction* undoAction = WuQtUtilities::createAction("Undo", 
                                                      "Remove (undo) the last border point being drawn", 
                                                      this);
    undoAction->setCheckable(true);
    QToolButton* undoToolButton = new QToolButton();
    undoToolButton->setDefaultAction(undoAction);
    
    QAction* resetAction = WuQtUtilities::createAction("Reset", 
                                                       "Remove all points in the unfinished border", 
                                                       this);
    resetAction->setCheckable(true);
    QToolButton* resetToolButton = new QToolButton();
    resetToolButton->setDefaultAction(resetAction);
    
    QAction* rotateAction = WuQtUtilities::createAction("Rotate", 
                                                        "Allow mouse to rotate the surface\n"
                                                        "instead of drawing a border", 
                                                        this);
    rotateAction->setCheckable(true);
    QToolButton* rotateToolButton = new QToolButton();
    rotateToolButton->setDefaultAction(rotateAction);
    
    this->createOperationActionGroup = new QActionGroup(this);
    this->createOperationActionGroup->addAction(drawAction);
    this->createOperationActionGroup->addAction(finishAction);
    this->createOperationActionGroup->addAction(undoAction);
    this->createOperationActionGroup->addAction(resetAction);
    this->createOperationActionGroup->addAction(rotateAction);
    this->createOperationActionGroup->setExclusive(true);
    QObject::connect(this->createOperationActionGroup, SIGNAL(triggered(QAction*)),
                     this, SLOT(createOperationActionTriggered(QAction*)));
    
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    WuQtUtilities::setLayoutMargins(layout, 2, 0, 0);
    layout->addWidget(drawToolButton);
    layout->addWidget(finishToolButton);
    layout->addWidget(undoToolButton);
    layout->addWidget(resetToolButton);
    layout->addWidget(rotateToolButton);
    
    widget->setFixedWidth(widget->sizeHint().width());
    return widget;
}

/**
 * Called when a create mode button is clicked.
 * @param action
 *    Action that was triggered.
 */
void 
UserInputModeBordersWidget::createOperationActionTriggered(QAction* action)
{
    
}

/**
 * @return The edit mode widget.
 */
QWidget* 
UserInputModeBordersWidget::createEditOperationWidget()
{
    QAction* deleteAction = WuQtUtilities::createAction("Delete", 
                                                        "Delete a border by clicking\n"
                                                        "the mouse over the border",
                                                        this);
    deleteAction->setCheckable(true);
    QToolButton* deleteToolButton = new QToolButton();
    deleteToolButton->setDefaultAction(deleteAction);
    
    QAction* editAction = WuQtUtilities::createAction("Edit", 
                                                      "Edit a border by clicking the mouse\n"
                                                      "on a border which then pops up a dialog\n"
                                                      "for editing the borders attributes", 
                                                      this);
    editAction->setCheckable(true);
    QToolButton* editToolButton = new QToolButton();
    editToolButton->setDefaultAction(editAction);
    
    QAction* reverseAction = WuQtUtilities::createAction("Reverse", 
                                                         "Reverse the order of the points in a\n"
                                                         "border by clicking the mouse on the border", 
                                                         this);
    reverseAction->setCheckable(true);
    QToolButton* reverseToolButton = new QToolButton();
    reverseToolButton->setDefaultAction(reverseAction);
    
    this->editOperationActionGroup = new QActionGroup(this);
    this->editOperationActionGroup->addAction(deleteAction);
    this->editOperationActionGroup->addAction(editAction);
    this->editOperationActionGroup->addAction(reverseAction);
    this->editOperationActionGroup->setExclusive(true);
    QObject::connect(this->editOperationActionGroup, SIGNAL(triggered(QAction*)),
                     this, SLOT(editOperationActionTriggered(QAction*)));
    
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    WuQtUtilities::setLayoutMargins(layout, 2, 0, 0);
    layout->addWidget(deleteToolButton);
    layout->addWidget(editToolButton);
    layout->addWidget(reverseToolButton);
    
    widget->setFixedWidth(widget->sizeHint().width());
    return widget;
}

/**
 * Called when a edit button is clicked.
 * @param action
 *    Action that was triggered.
 */
void 
UserInputModeBordersWidget::editOperationActionTriggered(QAction* action)
{
    
}

/**
 * @return The point edit mode widget.
 */
QWidget* 
UserInputModeBordersWidget::createPointEditOperationWidget()
{
    QAction* deleteAction = WuQtUtilities::createAction("Delete", 
                                                        "Delete points in a border by\n"
                                                        "clicking them with the mouse", 
                                                        this);
    deleteAction->setCheckable(true);
    QToolButton* deleteToolButton = new QToolButton();
    deleteToolButton->setDefaultAction(deleteAction);
    
    QAction* moveAction = WuQtUtilities::createAction("Move", 
                                                      "Move points in a border by depressing the\n"
                                                      "mouse over a border point, move the mouse,\n"
                                                      "and release the mouse", 
                                                      this);
    moveAction->setCheckable(true);
    QToolButton* moveToolButton = new QToolButton();
    moveToolButton->setDefaultAction(moveAction);
    
    this->pointEditOperationActionGroup = new QActionGroup(this);
    this->pointEditOperationActionGroup->addAction(deleteAction);
    this->pointEditOperationActionGroup->addAction(moveAction);
    this->pointEditOperationActionGroup->setExclusive(true);
    QObject::connect(this->pointEditOperationActionGroup, SIGNAL(triggered(QAction*)),
                     this, SLOT(pointEditOperationActionTriggered(QAction*)));
    
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    WuQtUtilities::setLayoutMargins(layout, 2, 0, 0);
    layout->addWidget(deleteToolButton);
    layout->addWidget(moveToolButton);
    
    widget->setFixedWidth(widget->sizeHint().width());
    return widget;
}

/**
 * Called when a point edit button is clicked.
 * @param action
 *    Action that was triggered.
 */
void 
UserInputModeBordersWidget::pointEditOperationActionTriggered(QAction* action)
{
    
}

/**
 * @return The update operation widget.
 */
QWidget* 
UserInputModeBordersWidget::createUpdateOperationWidget()
{
    QAction* eraseAction = WuQtUtilities::createAction("Erase", 
                                                       "Erase from an end of the border by\n"
                                                       "depressing the mouse at the end of\n"
                                                       "the border, moving the mouse along the\n"
                                                       "border to the new end point, and then\n"
                                                       "release the mouse button", 
                                                       this);
    eraseAction->setCheckable(true);
    QToolButton* eraseToolButton = new QToolButton();
    eraseToolButton->setDefaultAction(eraseAction);
    
    QAction* extendAction = WuQtUtilities::createAction("Extend", 
                                                        "Extend a border by depressing the mouse button\n"
                                                        "at an end of the border, moving the mouse\n"
                                                        "to create the new segment, and then \n"
                                                        "releasing the mouse button", 
                                                        this);
    extendAction->setCheckable(true);
    QToolButton* extendToolButton = new QToolButton();
    extendToolButton->setDefaultAction(extendAction);
    
    QAction* replaceAction = WuQtUtilities::createAction("Replace", 
                                                         "Replace a segment in the border by moving the\n"
                                                         "mouse to a location on the border, depressing the\n"
                                                         "mouse button, moving the mouse along the new\n"
                                                         "segment, returning the mouse to a point on the\n"
                                                         "border, and then releasing the mouse", 
                                                         this);
    replaceAction->setCheckable(true);
    QToolButton* replaceToolButton = new QToolButton();
    replaceToolButton->setDefaultAction(replaceAction);
    
    this->updateOperationActionGroup = new QActionGroup(this);
    this->updateOperationActionGroup->addAction(eraseAction);
    this->updateOperationActionGroup->addAction(extendAction);
    this->updateOperationActionGroup->addAction(replaceAction);
    this->updateOperationActionGroup->setExclusive(true);
    QObject::connect(this->updateOperationActionGroup, SIGNAL(triggered(QAction*)),
                     this, SLOT(updateOperationActionTriggered(QAction*)));
    
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    WuQtUtilities::setLayoutMargins(layout, 2, 0, 0);
    layout->addWidget(eraseToolButton);
    layout->addWidget(extendToolButton);
    layout->addWidget(replaceToolButton);
    
    widget->setFixedWidth(widget->sizeHint().width());
    return widget;
}

/**
 * Called when a update button is clicked.
 * @param action
 *    Action that was triggered.
 */
void 
UserInputModeBordersWidget::updateOperationActionTriggered(QAction* action)
{
    
}

