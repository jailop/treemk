#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "filesystemtreeview.h"
#include "helpdialog.h"
#include "linkparser.h"
#include "logic/aiprovider.h"
#include "logic/mainfilelocator.h"
#include "logic/ollamaprovider.h"
#include "logic/openaiprovider.h"
#include "logic/systemprompts.h"
#include "mainwindow.h"
#include "managers/windowmanager.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "markdownpreview.h"
#include "outlinepanel.h"
#include "settingsdialog.h"
#include "shortcutsdialog.h"
#include "sidebarpanel.h"
#include "tabeditor.h"
#include "thememanager.h"

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
    QMenu* exportMenu = fileMenu->addMenu(tr("&Export"));
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
    // Create AI Assistant submenu
    createAIAssistMenu();
    aiAssistEditSubmenu = editMenu->addMenu(tr("AI &Assistant"));
    aiAssistEditSubmenu->addActions(aiAssistMenu->actions());
    editMenu->addSeparator();
    editMenu->addAction(breakLinesAction);
    editMenu->addAction(joinLinesAction);
    // moved to goMenu
    // editMenu->addSeparator();
    // editMenu->addAction(quickOpenAction);

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
    insertMenu->addSeparator();

    // Date submenu
    insertDateMenu = insertMenu->addMenu(tr("Date"));

    QAction* dateYMD = insertDateMenu->addAction(tr("YYYY-MM-DD"));
    connect(dateYMD, &QAction::triggered, this,
            [this]() { insertDateWithFormat("yyyy-MM-dd"); });

    QAction* dateDMY = insertDateMenu->addAction(tr("DD/MM/YYYY"));
    connect(dateDMY, &QAction::triggered, this,
            [this]() { insertDateWithFormat("dd/MM/yyyy"); });

    QAction* dateMDY = insertDateMenu->addAction(tr("MM/DD/YYYY"));
    connect(dateMDY, &QAction::triggered, this,
            [this]() { insertDateWithFormat("MM/dd/yyyy"); });

    QAction* dateLong = insertDateMenu->addAction(tr("Month DD, YYYY"));
    connect(dateLong, &QAction::triggered, this,
            [this]() { insertDateWithFormat("MMMM dd, yyyy"); });

    QAction* dateFull =
        insertDateMenu->addAction(tr("Weekday, Month DD, YYYY"));
    connect(dateFull, &QAction::triggered, this,
            [this]() { insertDateWithFormat("dddd, MMMM dd, yyyy"); });

    insertDateMenu->addSeparator();
    insertDateMenu->addAction(insertDateAction);  // Insert with last format

    // Time submenu
    insertTimeMenu = insertMenu->addMenu(tr("Time"));

    QAction* timeHM = insertTimeMenu->addAction(tr("HH:MM"));
    connect(timeHM, &QAction::triggered, this,
            [this]() { insertTimeWithFormat("HH:mm"); });

    QAction* timeHMS = insertTimeMenu->addAction(tr("HH:MM:SS"));
    connect(timeHMS, &QAction::triggered, this,
            [this]() { insertTimeWithFormat("HH:mm:ss"); });

    QAction* time12H = insertTimeMenu->addAction(tr("hh:mm AM/PM"));
    connect(time12H, &QAction::triggered, this,
            [this]() { insertTimeWithFormat("hh:mm AP"); });

    QAction* time12HMS = insertTimeMenu->addAction(tr("hh:mm:ss AM/PM"));
    connect(time12HMS, &QAction::triggered, this,
            [this]() { insertTimeWithFormat("hh:mm:ss AP"); });

    insertTimeMenu->addSeparator();
    insertTimeMenu->addAction(insertTimeAction);  // Insert with last format

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(toggleSidebarAction);
    viewMenu->addAction(cycleViewModeAction);
    viewMenu->addSeparator();
    /*
    QMenu* previewThemeMenu = viewMenu->addMenu(tr("Preview Theme"));
    previewThemeMenu->addAction(previewThemeLightAction);
    previewThemeMenu->addAction(previewThemeDarkAction);
    previewThemeMenu->addAction(previewThemeSepiaAction);
    */

    goMenu = menuBar()->addMenu(tr("&Go"));
    goMenu->addAction(backAction);
    goMenu->addAction(forwardAction);
    goMenu->addSeparator();
    goMenu->addAction(quickOpenAction);
    goMenu->addSeparator();
    goMenu->addAction(nextTabAction);
    goMenu->addAction(previousTabAction);
    goMenu->addAction(closeTabAction);
    goMenu->addAction(closeAllTabsAction);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(userGuideAction);
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

    // Navigation
    mainToolbar->addAction(backAction);
    mainToolbar->addAction(forwardAction);
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

    // AI and Search
    // Create AI Assistant dropdown button
    QToolButton* aiAssistButton = new QToolButton(this);
    aiAssistButton->setDefaultAction(aiAssistAction);
    aiAssistButton->setPopupMode(QToolButton::MenuButtonPopup);
    aiAssistButton->setMenu(aiAssistMenu);
    aiAssistButton->setToolTip(
        tr("AI Assist (click for custom, dropdown for presets)"));
    mainToolbar->addWidget(aiAssistButton);

    mainToolbar->addAction(findAction);
    mainToolbar->addAction(quickOpenAction);
    mainToolbar->addSeparator();

    // View toggles
    mainToolbar->addAction(toggleSidebarAction);
    mainToolbar->addAction(cycleViewModeAction);
}

void MainWindow::createLayout() {
    // Create sidebar panel from UI file
    sidebarPanel = new SidebarPanel(this);
    
    // Get references to widgets from UI
    leftTabWidget = sidebarPanel->getLeftTabWidget();
    treePanel = sidebarPanel->getTreePanel();
    outlinePanel = sidebarPanel->getOutlinePanel();
    backlinksPanel = sidebarPanel->getBacklinksPanel();
    historyPanel = sidebarPanel->getHistoryPanel();
    backlinksView = sidebarPanel->getBacklinksView();
    historyView = sidebarPanel->getHistoryView();
    historyFilterInput = sidebarPanel->getHistoryFilterInput();
    
    // Create and setup tree view
    treeView = new FileSystemTreeView(treePanel);
    sidebarPanel->setTreeView(treeView);
    
    connect(treeView, &FileSystemTreeView::fileSelected, this,
            &MainWindow::onFileSelected);
    connect(treeView, &FileSystemTreeView::fileDoubleClicked, this,
            &MainWindow::onFileDoubleClicked);
    connect(treeView, &FileSystemTreeView::fileModifiedExternally, this,
            &MainWindow::onFileModifiedExternally);
    connect(treeView, &FileSystemTreeView::folderChanged, this,
            &MainWindow::onFolderChanged);
    connect(treeView, &FileSystemTreeView::openInNewWindowRequested, this,
            [](const QString& path) {
                QFileInfo info(path);
                if (info.isDir()) {
                    WindowManager::instance()->createWindow(path, QString());
                } else {
                    WindowManager::instance()->createWindow(info.absolutePath(),
                                                            path);
                }
            });
    connect(treeView, &FileSystemTreeView::fileDeleted, this,
            &MainWindow::onFileDeleted);
    connect(treeView, &FileSystemTreeView::fileRenamed, this,
            &MainWindow::onFileRenamed);
    
    // Create and setup outline view
    outlineView = new OutlinePanel(outlinePanel);
    sidebarPanel->setOutlineView(outlineView);
    
    // Setup backlinks connections
    connect(backlinksView, &QListWidget::itemDoubleClicked,
            [this](QListWidgetItem* item) {
                QString filePath = item->data(Qt::UserRole).toString();
                if (!filePath.isEmpty()) {
                    loadFile(filePath);
                }
            });
    
    // Setup history connections
    connect(historyFilterInput, &QLineEdit::textChanged, this,
            &MainWindow::filterHistoryList);
    
    connect(historyView, &QListWidget::itemDoubleClicked,
            [this](QListWidgetItem* item) {
                QString filePath = item->data(Qt::UserRole).toString();
                if (!filePath.isEmpty()) {
                    loadFile(filePath);
                }
            });
    
    // Set tab icons and tooltips
    leftTabWidget->setTabIcon(0, QIcon::fromTheme("folder", 
                                style()->standardIcon(QStyle::SP_DirIcon)));
    leftTabWidget->setTabToolTip(0, tr("Files"));
    
    leftTabWidget->setTabIcon(1, QIcon::fromTheme("view-list-tree",
                                style()->standardIcon(QStyle::SP_FileDialogDetailedView)));
    leftTabWidget->setTabToolTip(1, tr("Outline"));
    
    leftTabWidget->setTabIcon(2, QIcon::fromTheme("insert-link",
                                style()->standardIcon(QStyle::SP_ArrowBack)));
    leftTabWidget->setTabToolTip(2, tr("Backlinks"));
    
    leftTabWidget->setTabIcon(3, QIcon::fromTheme("document-open-recent",
                                style()->standardIcon(QStyle::SP_FileDialogBack)));
    leftTabWidget->setTabToolTip(3, tr("History"));

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
    mainSplitter->addWidget(sidebarPanel);
    mainSplitter->addWidget(tabWidget);
    mainSplitter->setStretchFactor(0, 0);  // Sidebar panel
    mainSplitter->setStretchFactor(1, 1);  // Tab widget gets most space

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
        mainSplitter->restoreState(
            settings->value("mainSplitter").toByteArray());

    bool sidebarVisible = settings->value("sidebarVisible", true).toBool();
    sidebarPanel->setVisible(sidebarVisible);
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

    // Update the recent folders menu with loaded data
    populateRecentFoldersMenu();

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

        TabEditor* tab = currentTabEditor();
        if (tab && tab->editor()->getHighlighter()) {
            tab->editor()->getHighlighter()->setRootPath(folderToOpen);
        }

        buildLinkIndexAsync();
        
        // Restore the file tree root if it was set to a subdirectory
        QString fileTreeRoot = settings->value("session/fileTreeRoot").toString();
        if (!fileTreeRoot.isEmpty() && 
            QDir(fileTreeRoot).exists() && 
            fileTreeRoot != folderToOpen &&
            fileTreeRoot.startsWith(folderToOpen)) {
            // Only restore if it's a subdirectory of the current folder
            treeView->setRootPath(fileTreeRoot);
        }

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
            TabEditor* firstTab =
                qobject_cast<TabEditor*>(tabWidget->widget(0));
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
            TabEditor* firstTab =
                qobject_cast<TabEditor*>(tabWidget->widget(0));
            if (firstTab && firstTab->filePath().isEmpty() &&
                !firstTab->editor()->isModified()) {
                tabWidget->removeTab(0);
                delete firstTab;
            }
        }

        // Open each file from the session
        for (const QString& filePath : openFiles) {
            if (QFileInfo::exists(filePath)) {
                loadFile(filePath);
            }
        }

        // Restore the active tab
        if (activeTabIndex >= 0 && activeTabIndex < tabWidget->count()) {
            tabWidget->setCurrentIndex(activeTabIndex);
        }
    } else if (!folderToOpen.isEmpty()) {
        // No specific file or session to restore, try to open main file
        QString mainFileName =
            settings->value("workspace/mainFileName", "main.md").toString();
        QString mainFilePath =
            MainFileLocator::findMainFile(folderToOpen, mainFileName);

        if (!mainFilePath.isEmpty() && QFileInfo::exists(mainFilePath)) {
            // Close the default empty tab if it exists
            if (tabWidget->count() == 1) {
                TabEditor* firstTab =
                    qobject_cast<TabEditor*>(tabWidget->widget(0));
                if (firstTab && firstTab->filePath().isEmpty() &&
                    !firstTab->editor()->isModified()) {
                    tabWidget->removeTab(0);
                    delete firstTab;
                }
            }
            loadFile(mainFilePath);
        }
    }

    // Apply view mode to current tab
    applyViewMode(currentViewMode, false);
}

void MainWindow::writeSettings() {
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->setValue("mainSplitter", mainSplitter->saveState());
    settings->setValue("sidebarVisible", leftTabWidget->isVisible());
    settings->setValue("viewMode", static_cast<int>(currentViewMode));
    settings->setValue("lastFolder", currentFolder);
    settings->setValue("recentFolders", recentFolders);
    
    // Save the current file tree root path (which may be a subdirectory)
    if (treeView) {
        settings->setValue("session/fileTreeRoot", treeView->rootPath());
    }

    // Save open files
    QStringList openFiles;
    int activeTabIndex = tabWidget->currentIndex();

    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
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
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->editor()) {
            if (ThemeManager::instance()) {
                tab->editor()->setPalette(
                    ThemeManager::instance()->getEditorPalette());
                tab->editor()->setStyleSheet(
                    ThemeManager::instance()->getEditorStyleSheet());
            }
            MarkdownHighlighter* highlighter = tab->editor()->highlighter();
            if (highlighter) {
                highlighter->setColorScheme(resolvedEditorScheme);
                bool codeSyntaxEnabled =
                    settings->value("editor/enableCodeSyntax", false).toBool();
                highlighter->setCodeSyntaxEnabled(codeSyntaxEnabled);
                highlighter->rehighlight();
            }
            // Apply line numbers visibility
            bool showLineNumbers =
                settings->value("editor/showLineNumbers", true).toBool();
            tab->editor()->setLineNumbersVisible(showLineNumbers);
            
            // Apply word prediction setting
            bool wordPredictionEnabled =
                settings->value("editor/enableWordPrediction", true).toBool();
            tab->editor()->setPredictionEnabled(wordPredictionEnabled);
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
    // Apply AI settings
    QString activeProviderName =
        settings->value("ai/provider", "ollama").toString();
    AIProviderManager::instance()->setActiveProvider(activeProviderName);

    AIProvider* aiProvider = AIProviderManager::instance()->activeProvider();
    if (aiProvider) {
        if (aiProvider->name() == "Ollama") {
            OllamaProvider* ollama = dynamic_cast<OllamaProvider*>(aiProvider);
            if (ollama) {
                QString endpoint =
                    settings
                        ->value("ai/ollama/endpoint", "http://localhost:11434")
                        .toString();
                QString model =
                    settings->value("ai/ollama/model", "llama3.2").toString();
                int timeout = settings->value("ai/ollama/timeout", 60).toInt();

                ollama->setEndpoint(endpoint);
                ollama->setModel(model);
                ollama->setTimeout(timeout);
            }
        } else if (aiProvider->name() == "OpenAI") {
            OpenAIProvider* openai = dynamic_cast<OpenAIProvider*>(aiProvider);
            if (openai) {
                QString endpoint = settings
                                       ->value("ai/openai/endpoint",
                                               "https://api.openai.com/v1")
                                       .toString();
                QString apiKey =
                    settings->value("ai/openai/apikey", "").toString();
                if (apiKey.isEmpty()) {
                    apiKey = qEnvironmentVariable("OPENAI_API_KEY");
                }
                QString model =
                    settings->value("ai/openai/model", "gpt-4o-mini")
                        .toString();
                int timeout = settings->value("ai/openai/timeout", 60).toInt();

                openai->setEndpoint(endpoint);
                openai->setApiKey(apiKey);
                openai->setModel(model);
                openai->setTimeout(timeout);
            }
        }
    }

    // Reload system prompts
    SystemPrompts::instance()->loadFromSettings();

    // Rebuild AI assist menu (may have changed)
    createAIAssistMenu();

    // Apply AI enabled/disabled state
    bool aiEnabled = settings->value("ai/enabled", true).toBool();
    aiAssistAction->setEnabled(aiEnabled);
    if (aiAssistMenu) {
        aiAssistMenu->setEnabled(aiEnabled);
    }
    if (aiAssistEditSubmenu) {
        aiAssistEditSubmenu->setEnabled(aiEnabled);
    }
    
    // Apply AI enabled state to all editor tabs
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->editor()) {
            tab->editor()->setAIAssistEnabled(aiEnabled);
        }
    }

    // Get the resolved preview theme
    QString theme = settings->value("previewTheme", "light").toString();
    if (ThemeManager::instance()) {
        theme = ThemeManager::instance()->getResolvedPreviewColorSchemeName();
    }
    // Apply theme to all tabs
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->preview()) {
            tab->preview()->setTheme(theme);
        }
    }
    /* TODO to be removed. theme is general for all the app, not just
     * preview.
    if (previewThemeDarkAction && theme == "dark") {
        previewThemeDarkAction->setChecked(true);
    } else if (previewThemeSepiaAction && theme == "sepia") {
        previewThemeSepiaAction->setChecked(true);
    } else if (previewThemeLightAction) {
        previewThemeLightAction->setChecked(true);
    }
    */

    // Apply editor settings to all tabs
    QString fontFamily =
        settings->value("editor/font", "Sans Serif").toString();
    int fontSize = settings->value("editor/fontSize", 11).toInt();
    int tabWidth = settings->value("editor/tabWidth", 4).toInt();
    bool wordWrap = settings->value("editor/wordWrap", true).toBool();
    QFont font(fontFamily, fontSize);
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
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
    HelpDialog* helpDialog = new HelpDialog(this);
    helpDialog->showTopic("keyboard-shortcuts");
    helpDialog->setAttribute(Qt::WA_DeleteOnClose);
    helpDialog->show();
}
