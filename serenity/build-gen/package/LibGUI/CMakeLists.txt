# - THIS FILE HAS BEEN GENERATED - DO NOT EDIT
# - To regenerate, please use the meta tool.

cmake_minimum_required(VERSION 3.10)

set(PROJECT_ROOT_DIR "/home/ema/checkout/serenity")

set(SOURCES
    "/home/ema/checkout/serenity/Libraries/LibGUI/GWindowServerConnection.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GTextDocument.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GAction.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GTabWidget.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GSpinBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GTreeView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GRadioButton.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GTableView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GMessageBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GStatusBar.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GSortingProxyModel.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GModel.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GCheckBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GUndoStack.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GDragOperation.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GStackWidget.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GScrollableWidget.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GComboBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GDialog.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GApplication.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GScrollBar.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GVariant.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GResizeCorner.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GJsonArrayModel.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GAbstractTableView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GTextEditor.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GLazyWidget.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GFileSystemModel.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GWindow.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GLayout.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GClipboard.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GListView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GFilePicker.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GProgressBar.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GMenuItem.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GCommand.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GPainter.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GMenu.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GItemView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GBoxLayout.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GToolBar.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GSlider.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GWidget.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GInputBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GColumnsView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GMenuBar.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GLabel.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GModelSelection.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GAboutDialog.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GAbstractButton.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GIcon.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GAbstractView.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GSplitter.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GButton.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GShortcut.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GColorPicker.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GGroupBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GActionGroup.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GFrame.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GFontDatabase.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GTextBox.cpp"
    "/home/ema/checkout/serenity/Libraries/LibGUI/GDesktop.cpp"
)
set(INCLUDE_DIRS
    "${PROJECT_ROOT_DIR}/Libraries/LibGUI"
    "${gendata}/LibGUI"
)
set(STATIC_LINK_LIBRARIES
    "AK"
)
add_library(gui STATIC ${SOURCES})
target_include_directories(gui PUBLIC ${INCLUDE_DIRS})
target_link_libraries(gui PUBLIC ${STATIC_LINK_LIBRARIES})

