#include "mainwindow.h"

#include <QCloseEvent>
#include <QFileInfo>
#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#include "defs.h"
#include "filesystemtreeview.h"
#include "linkparser.h"
#include "navigationhistory.h"
#include "tabeditor.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      fileMenu(nullptr),
      editMenu(nullptr),
      insertMenu(nullptr),
      viewMenu(nullptr),
      helpMenu(nullptr),
      recentFoldersMenu(nullptr),
      aiAssistMenu(nullptr),
      aiAssistEditSubmenu(nullptr),
      insertDateMenu(nullptr),
      insertTimeMenu(nullptr),
      newAction(nullptr),
      openFolderAction(nullptr),
      saveAction(nullptr),
      saveAsAction(nullptr),
      exportHtmlAction(nullptr),
      exportPdfAction(nullptr),
      exportDocxAction(nullptr),
      exportPlainTextAction(nullptr),
      insertImageAction(nullptr),
      insertFormulaAction(nullptr),
      insertWikiLinkAction(nullptr),
      attachDocumentAction(nullptr),
      insertHeaderAction(nullptr),
      insertBoldAction(nullptr),
      insertItalicAction(nullptr),
      insertStrikethroughAction(nullptr),
      insertCodeAction(nullptr),
      insertCodeBlockAction(nullptr),
      insertListAction(nullptr),
      insertNumberedListAction(nullptr),
      insertBlockquoteAction(nullptr),
      insertHorizontalRuleAction(nullptr),
      insertLinkAction(nullptr),
      insertTableAction(nullptr),
      insertDateAction(nullptr),
      insertTimeAction(nullptr),
      exitAction(nullptr),
      undoAction(nullptr),
      redoAction(nullptr),
      cutAction(nullptr),
      copyAction(nullptr),
      pasteAction(nullptr),
      findAction(nullptr),
      findReplaceAction(nullptr),
      searchInFilesAction(nullptr),
      quickOpenAction(nullptr),
      breakLinesAction(nullptr),
      joinLinesAction(nullptr),
      closeTabAction(nullptr),
      closeAllTabsAction(nullptr),
      toggleSidebarAction(nullptr),
      togglePreviewAction(nullptr),
      cycleViewModeAction(nullptr),
      // previewThemeLightAction(nullptr),
      // previewThemeDarkAction(nullptr),
      // previewThemeSepiaAction(nullptr),
      settingsAction(nullptr),
      userGuideAction(nullptr),
      aboutAction(nullptr),
      aboutQtAction(nullptr),
      keyboardShortcutsAction(nullptr),
      currentViewMode(ViewMode_Both) {
    settings = new QSettings(APP_LABEL, APP_LABEL, this);
    linkParser = new LinkParser(this);
    connect(linkParser, &LinkParser::indexBuildCompleted, this,
            &MainWindow::updateBacklinks);

    navigationHistory = new NavigationHistory(this);
    connect(navigationHistory, &NavigationHistory::canGoBackChanged, this,
            &MainWindow::updateNavigationActions);
    connect(navigationHistory, &NavigationHistory::canGoForwardChanged, this,
            &MainWindow::updateNavigationActions);
    connect(
        navigationHistory, &NavigationHistory::historyChanged, this, [this]() {
            // Update history panel display using filter function
            filterHistoryList();
        });

    setWindowTitle("TreeMk - Markdown Editor");
    setWindowIcon(QIcon::fromTheme("text-editor"));

    // Setup progress bar in status bar
    progressBar = new QProgressBar(this);
    progressBar->setMaximumWidth(200);
    progressBar->setVisible(false);
    statusBar()->addPermanentWidget(progressBar);
    statusBar()->showMessage(tr("Ready"));

    // Setup auto-save timer
    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);

    // Setup preview update timer
    previewUpdateTimer = new QTimer(this);
    previewUpdateTimer->setSingleShot(true);
    connect(previewUpdateTimer, &QTimer::timeout, this,
            &MainWindow::updatePreview);

    // Create UI components in order
    createLayout();
    createActions();
    createAIAssistMenu();
    createMenus();
    createToolbar();
    // Note: readSettings() is called after setStartupArguments() in main.cpp
}

MainWindow::~MainWindow() {}

void MainWindow::setStartupArguments(const QString& path, const QString& file) {
    m_startupPath = path;
    m_startupFile = file;
}

void MainWindow::initializeSettings() {
    readSettings();
    applySettings();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

int MainWindow::getLinkSearchDepth() const {
    return settings
        ->value("workspace/linkSearchDepth", DEFAULT_LINK_SEARCH_DEPTH)
        .toInt();
}

void MainWindow::buildLinkIndexAsync() {
    int depth = getLinkSearchDepth();
    auto future = QtConcurrent::run(
        [this, depth]() { linkParser->buildLinkIndex(currentFolder, depth); });
    Q_UNUSED(future);
}
