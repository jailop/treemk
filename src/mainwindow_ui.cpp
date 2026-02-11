#include "filesystemtreeview.h"
#include "linkparser.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "markdownpreview.h"
#include "outlinepanel.h"
#include "settingsdialog.h"
#include "shortcutsdialog.h"
#include "tabeditor.h"
#include "thememanager.h"
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAction);
  fileMenu->addAction(openFolderAction);
  fileMenu->addSeparator();
  recentFoldersMenu = fileMenu->addMenu(tr("Recent &Folders"));
  populateRecentFoldersMenu();
  fileMenu->addSeparator();
  fileMenu->addAction(saveAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addSeparator();
  QMenu *exportMenu = fileMenu->addMenu(tr("&Export"));
  exportMenu->addAction(exportHtmlAction);
  exportMenu->addAction(exportPdfAction);
  exportMenu->addAction(exportDocxAction);
  exportMenu->addAction(exportPlainTextAction);
  fileMenu->addSeparator();
  fileMenu->addAction(closeTabAction);
  fileMenu->addAction(closeAllTabsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(settingsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(undoAction);
  editMenu->addAction(redoAction);
  editMenu->addSeparator();
  editMenu->addAction(cutAction);
  editMenu->addAction(copyAction);
  editMenu->addAction(pasteAction);
  editMenu->addSeparator();
   editMenu->addAction(findAction);
   editMenu->addAction(findReplaceAction);
   editMenu->addAction(searchInFilesAction);
   editMenu->addSeparator();
   editMenu->addAction(breakLinesAction);
   editMenu->addAction(joinLinesAction);
   editMenu->addSeparator();
   editMenu->addAction(quickOpenAction);

  insertMenu = menuBar()->addMenu(tr("&Insert"));
  insertMenu->addAction(insertImageAction);
  insertMenu->addAction(attachDocumentAction);
  insertMenu->addAction(insertFormulaAction);
  insertMenu->addSeparator();
  insertMenu->addAction(insertWikiLinkAction);
  insertMenu->addAction(insertLinkAction);
  insertMenu->addSeparator();
  insertMenu->addAction(insertHeaderAction);
   insertMenu->addAction(insertBoldAction);
   insertMenu->addAction(insertItalicAction);
   insertMenu->addAction(insertStrikethroughAction);
   insertMenu->addSeparator();
  insertMenu->addAction(insertCodeAction);
  insertMenu->addAction(insertCodeBlockAction);
  insertMenu->addSeparator();
  insertMenu->addAction(insertListAction);
  insertMenu->addAction(insertNumberedListAction);
  insertMenu->addAction(insertBlockquoteAction);
  insertMenu->addSeparator();
  insertMenu->addAction(insertHorizontalRuleAction);
  insertMenu->addAction(insertTableAction);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(toggleSidebarAction);
  viewMenu->addAction(cycleViewModeAction);
  viewMenu->addSeparator();
  QMenu *previewThemeMenu = viewMenu->addMenu(tr("Preview Theme"));
  previewThemeMenu->addAction(previewThemeLightAction);
  previewThemeMenu->addAction(previewThemeDarkAction);
  previewThemeMenu->addAction(previewThemeSepiaAction);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(keyboardShortcutsAction);
  helpMenu->addSeparator();
  helpMenu->addAction(aboutAction);
  helpMenu->addAction(aboutQtAction);
}

void MainWindow::createToolbar() {
  mainToolbar = addToolBar(tr("Main Toolbar"));
  mainToolbar->setObjectName("MainToolbar");
  mainToolbar->setMovable(false);
  
  // Set smaller icon size
  mainToolbar->setIconSize(QSize(16, 16));

  // File operations
  mainToolbar->addAction(newAction);
  mainToolbar->addAction(openFolderAction);
  mainToolbar->addAction(saveAction);
  mainToolbar->addSeparator();

  // Edit operations
  mainToolbar->addAction(undoAction);
  mainToolbar->addAction(redoAction);
  mainToolbar->addSeparator();

  // Formatting
  mainToolbar->addAction(insertBoldAction);
  mainToolbar->addAction(insertItalicAction);
  mainToolbar->addAction(insertCodeAction);
  mainToolbar->addSeparator();

  // Insert operations
  mainToolbar->addAction(insertLinkAction);
  mainToolbar->addAction(insertImageAction);
  mainToolbar->addAction(attachDocumentAction);
  mainToolbar->addAction(insertTableAction);
  mainToolbar->addSeparator();

  // Export operations
  mainToolbar->addAction(exportHtmlAction);
  mainToolbar->addAction(exportPdfAction);
  mainToolbar->addAction(exportDocxAction);
  mainToolbar->addAction(exportPlainTextAction);
  mainToolbar->addSeparator();

  // Search
  mainToolbar->addAction(findAction);
  mainToolbar->addAction(quickOpenAction);
  mainToolbar->addSeparator();

  // View toggles
  mainToolbar->addAction(toggleSidebarAction);
  mainToolbar->addAction(cycleViewModeAction);
}

void MainWindow::createLayout() {
  // Create left panel with tabs for File Tree, Outline, and Backlinks
  leftTabWidget = new QTabWidget(this);
  leftTabWidget->setTabPosition(QTabWidget::South);
  leftTabWidget->setMinimumWidth(150);

  // File tree tab
  treePanel = new QWidget(this);
  QVBoxLayout *treeLayout = new QVBoxLayout(treePanel);
  treeLayout->setContentsMargins(0, 0, 0, 0);

  treeView = new FileSystemTreeView(treePanel);
  treeLayout->addWidget(treeView);

  connect(treeView, &FileSystemTreeView::fileSelected, this,
          &MainWindow::onFileSelected);
  connect(treeView, &FileSystemTreeView::fileDoubleClicked, this,
          &MainWindow::onFileDoubleClicked);
  connect(treeView, &FileSystemTreeView::fileModifiedExternally, this,
          &MainWindow::onFileModifiedExternally);

  leftTabWidget->addTab(treePanel, tr("Files"));

  // Outline tab
  outlinePanel = new QWidget(this);
  QVBoxLayout *outlineLayout = new QVBoxLayout(outlinePanel);
  outlineLayout->setContentsMargins(0, 0, 0, 0);

  outlineView = new OutlinePanel(outlinePanel);
  outlineLayout->addWidget(outlineView);

  leftTabWidget->addTab(outlinePanel, tr("Outline"));

  // Backlinks tab
  backlinksPanel = new QWidget(this);
  QVBoxLayout *backlinksLayout = new QVBoxLayout(backlinksPanel);
  backlinksLayout->setContentsMargins(0, 0, 0, 0);

  backlinksView = new QListWidget(backlinksPanel);
  backlinksLayout->addWidget(backlinksView);

  connect(backlinksView, &QListWidget::itemDoubleClicked,
          [this](QListWidgetItem *item) {
            QString filePath = item->data(Qt::UserRole).toString();
            if (!filePath.isEmpty()) {
              loadFile(filePath);
            }
          });

  leftTabWidget->addTab(backlinksPanel, tr("Backlinks"));

  // Tab widget for multiple editors
  tabWidget = new QTabWidget(this);
  tabWidget->setTabsClosable(true);
  tabWidget->setMovable(true);
  tabWidget->setDocumentMode(true);

  connect(tabWidget, &QTabWidget::currentChanged, this,
          &MainWindow::onTabChanged);
  connect(tabWidget, &QTabWidget::tabCloseRequested, this,
          &MainWindow::onTabCloseRequested);

  // Main splitter
  mainSplitter = new QSplitter(Qt::Horizontal, this);
  mainSplitter->addWidget(leftTabWidget);
  mainSplitter->addWidget(tabWidget);
  mainSplitter->setStretchFactor(0, 0); // Left tab widget
  mainSplitter->setStretchFactor(1, 1); // Tab widget gets most space

  setCentralWidget(mainSplitter);

  // Create initial tab
  createNewTab();
}

void MainWindow::readSettings() {
  QPoint pos = settings->value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings->value("size", QSize(1024, 768)).toSize();
  resize(size);
  move(pos);

  if (settings->contains("mainSplitter"))
    mainSplitter->restoreState(settings->value("mainSplitter").toByteArray());

  bool sidebarVisible = settings->value("sidebarVisible", true).toBool();
  leftTabWidget->setVisible(sidebarVisible);
  toggleSidebarAction->setChecked(sidebarVisible);

  // Load and apply view mode
  int savedViewMode = settings->value("viewMode", ViewMode_Both).toInt();
  currentViewMode = static_cast<ViewMode>(savedViewMode);
  
  // Update action text based on current mode
  QString nextModeText;
  switch (currentViewMode) {
    case ViewMode_Both:
      nextModeText = tr("Cycle View Mode (Next: Editor Only)");
      break;
    case ViewMode_EditorOnly:
      nextModeText = tr("Cycle View Mode (Next: Preview Only)");
      break;
    case ViewMode_PreviewOnly:
      nextModeText = tr("Cycle View Mode (Next: Both)");
      break;
  }
  if (cycleViewModeAction) {
    cycleViewModeAction->setText(nextModeText);
  }

  // Load recent folders
  recentFolders = settings->value("recentFolders").toStringList();

   // Determine which folder to open
   QString folderToOpen;
   if (!m_startupPath.isEmpty()) {
     // Use startup path from command-line argument
     folderToOpen = m_startupPath;
   } else {
     // Use last opened folder
     folderToOpen = settings->value("lastFolder").toString();
   }

   if (!folderToOpen.isEmpty() && QDir(folderToOpen).exists()) {
     treeView->setRootPath(folderToOpen);
     currentFolder = folderToOpen;

     TabEditor *tab = currentTabEditor();
     if (tab && tab->editor()->getHighlighter()) {
       tab->editor()->getHighlighter()->setRootPath(folderToOpen);
     }

     linkParser->buildLinkIndex(folderToOpen);

     statusBar()->showMessage(tr("Opened folder: %1").arg(folderToOpen));
   }

   // Restore open files from last session (if enabled in preferences)
   bool restoreSession =
       settings->value("general/restoreSession", true).toBool();
   QStringList openFiles = settings->value("session/openFiles").toStringList();
   int activeTabIndex = settings->value("session/activeTab", -1).toInt();

   if (!m_startupFile.isEmpty()) {
     // If a specific file was provided via command-line, open it
     // Close the default empty tab if it exists
     if (tabWidget->count() == 1) {
       TabEditor *firstTab = qobject_cast<TabEditor *>(tabWidget->widget(0));
       if (firstTab && firstTab->filePath().isEmpty() &&
           !firstTab->editor()->isModified()) {
         tabWidget->removeTab(0);
         delete firstTab;
       }
     }
     loadFile(m_startupFile);
   } else if (restoreSession && !openFiles.isEmpty()) {
     // Close the default empty tab if it exists and is empty
     if (tabWidget->count() == 1) {
       TabEditor *firstTab = qobject_cast<TabEditor *>(tabWidget->widget(0));
       if (firstTab && firstTab->filePath().isEmpty() &&
           !firstTab->editor()->isModified()) {
         tabWidget->removeTab(0);
         delete firstTab;
       }
     }

      // Open each file from the session
      for (const QString &filePath : openFiles) {
        if (QFileInfo::exists(filePath)) {
          loadFile(filePath);
        }
      }

      // Restore the active tab
      if (activeTabIndex >= 0 && activeTabIndex < tabWidget->count()) {
        tabWidget->setCurrentIndex(activeTabIndex);
      }
    }
    
    // Apply view mode to current tab
    applyViewMode(currentViewMode);
}

void MainWindow::writeSettings() {
  settings->setValue("pos", pos());
  settings->setValue("size", size());
  settings->setValue("mainSplitter", mainSplitter->saveState());
  settings->setValue("sidebarVisible", leftTabWidget->isVisible());
  settings->setValue("viewMode", static_cast<int>(currentViewMode));
  settings->setValue("lastFolder", currentFolder);
  settings->setValue("recentFolders", recentFolders);

  // Save open files
  QStringList openFiles;
  int activeTabIndex = tabWidget->currentIndex();

  for (int i = 0; i < tabWidget->count(); ++i) {
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && !tab->filePath().isEmpty()) {
      openFiles.append(tab->filePath());
    }
  }

  settings->setValue("session/openFiles", openFiles);
  settings->setValue("session/activeTab", activeTabIndex);
}

void MainWindow::openSettings() {
  SettingsDialog dialog(this);
  connect(&dialog, &SettingsDialog::settingsChanged, this,
          &MainWindow::applySettings);

  dialog.exec();
}

void MainWindow::applySettings() {
  QString appTheme =
      settings->value("appearance/appTheme", "system").toString();
  if (ThemeManager::instance()) {
    ThemeManager::instance()->setAppTheme(appTheme);
  }
  QString editorScheme =
      settings->value("appearance/editorColorScheme", "auto").toString();
  if (ThemeManager::instance()) {
    ThemeManager::instance()->setEditorColorScheme(editorScheme);
  }
  QString resolvedEditorScheme = "light";
  if (ThemeManager::instance()) {
    resolvedEditorScheme =
        ThemeManager::instance()->getResolvedEditorColorSchemeName();
  }
  for (int i = 0; i < tabWidget->count(); ++i) {
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && tab->editor()) {
      if (ThemeManager::instance()) {
        tab->editor()->setPalette(ThemeManager::instance()->getEditorPalette());
        tab->editor()->setStyleSheet(
            ThemeManager::instance()->getEditorStyleSheet());
      }
      MarkdownHighlighter *highlighter = tab->editor()->highlighter();
      if (highlighter) {
        highlighter->setColorScheme(resolvedEditorScheme);
        bool codeSyntaxEnabled =
            settings->value("editor/enableCodeSyntax", false).toBool();
        highlighter->setCodeSyntaxEnabled(codeSyntaxEnabled);
        highlighter->rehighlight();
      }
    }
  }
  // Apply auto-save settings
  if (settings->value("autoSaveEnabled", true).toBool()) {
    int interval = settings->value("autoSaveInterval", 60).toInt();
    if (autoSaveTimer) {
      autoSaveTimer->start(interval * 1000);
    }
  } else {
    if (autoSaveTimer) {
      autoSaveTimer->stop();
    }
  }
  // Apply preview color scheme
  QString previewScheme =
      settings->value("appearance/previewColorScheme", "auto").toString();
  if (ThemeManager::instance()) {
    ThemeManager::instance()->setPreviewColorScheme(previewScheme);
  }
  // Get the resolved preview theme
  QString theme = settings->value("previewTheme", "light").toString();
  if (ThemeManager::instance()) {
    theme = ThemeManager::instance()->getResolvedPreviewColorSchemeName();
  }
  // Apply theme to all tabs
  for (int i = 0; i < tabWidget->count(); ++i) {
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && tab->preview()) {
      tab->preview()->setTheme(theme);
    }
  }
  // Update theme action checkboxes
  if (previewThemeDarkAction && theme == "dark") {
    previewThemeDarkAction->setChecked(true);
  } else if (previewThemeSepiaAction && theme == "sepia") {
    previewThemeSepiaAction->setChecked(true);
  } else if (previewThemeLightAction) {
    previewThemeLightAction->setChecked(true);
  }
  // Apply editor settings to all tabs
  QString fontFamily = settings->value("editor/font", "Sans Serif").toString();
  int fontSize = settings->value("editor/fontSize", 11).toInt();
  int tabWidth = settings->value("editor/tabWidth", 4).toInt();
  bool wordWrap = settings->value("editor/wordWrap", true).toBool();
  QFont font(fontFamily, fontSize);
  for (int i = 0; i < tabWidget->count(); ++i) {
     TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
     if (tab && tab->editor()) {
       tab->editor()->setFont(font);
       tab->editor()->setTabStopDistance(
           QFontMetrics(font).horizontalAdvance(' ') * tabWidth);
       tab->editor()->setLineWrapMode(wordWrap ? QTextEdit::WidgetWidth
                                               : QTextEdit::NoWrap);
     }
   }
  // Apply preview refresh rate
  int refreshRate = settings->value("preview/refreshRate", 500).toInt();
  if (previewUpdateTimer) {
    previewUpdateTimer->setInterval(refreshRate);
  }
}

void MainWindow::showKeyboardShortcuts() {
  ShortcutsDialog dialog(this);
  dialog.exec();
}
