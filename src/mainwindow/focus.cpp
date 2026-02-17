#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "sidebarpanel.h"
#include "tabeditor.h"
#include <QMenuBar>
#include <QSettings>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>

void MainWindow::toggleFocusMode() {
    focusModeActive = !focusModeActive;
    
    TabEditor* tab = currentTabEditor();
    
    if (focusModeActive) {
        preFocusModeViewMode = currentViewMode;
        
        if (sidebarPanel) {
            preFocusModeSidebarVisible = sidebarPanel->isVisible();
        }
        
        if (menuBar()) {
            menuBar()->hide();
        }
        if (mainToolbar) {
            mainToolbar->hide();
        }
        if (sidebarPanel) {
            sidebarPanel->hide();
        }
        if (statusBar()) {
            statusBar()->hide();
        }
        
        if (tab) {
            if (tab->editor()) {
                tab->editor()->setVisible(true);
                tab->editor()->setFocusModeEnabled(true);
            }
            if (tab->preview()) {
                tab->preview()->hide();
            }
        }
        
        if (tabWidget) {
            for (int i = 0; i < tabWidget->count(); ++i) {
                TabEditor* t = qobject_cast<TabEditor*>(tabWidget->widget(i));
                if (t && t->editor()) {
                    t->editor()->setVisible(true);
                    t->editor()->setFocusModeEnabled(true);
                }
            }
        }
        
        if (statusBar()) {
            statusBar()->show();
            statusBar()->showMessage(tr("Focus Mode: ON (F11 to exit)"), 2000);
            QTimer::singleShot(2000, this, [this]() {
                if (focusModeActive && statusBar()) {
                    statusBar()->hide();
                }
            });
        }
        
    } else {
        if (menuBar()) {
            menuBar()->show();
        }
        if (mainToolbar) {
            mainToolbar->show();
        }
        if (statusBar()) {
            statusBar()->show();
        }
        
        if (sidebarPanel) {
            sidebarPanel->setVisible(preFocusModeSidebarVisible);
        }
        
        if (tabWidget) {
            for (int i = 0; i < tabWidget->count(); ++i) {
                TabEditor* t = qobject_cast<TabEditor*>(tabWidget->widget(i));
                if (t && t->editor()) {
                    t->editor()->setFocusModeEnabled(false);
                }
            }
        }
        
        applyViewMode(preFocusModeViewMode, false);
        
        if (statusBar()) {
            statusBar()->showMessage(tr("Focus Mode: OFF"), 2000);
        }
    }
    
    if (toggleFocusModeAction) {
        toggleFocusModeAction->setChecked(focusModeActive);
    }
    
    if (settings) {
        settings->setValue("focusMode", focusModeActive);
    }
}
