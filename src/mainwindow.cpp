#include "defs.h"
#include "mainwindow.h"
#include "filesystemtreeview.h"
#include "linkparser.h"
#include "tabeditor.h"
#include <QCloseEvent>
#include <QProgressBar>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), fileMenu(nullptr), editMenu(nullptr),
      insertMenu(nullptr), viewMenu(nullptr), helpMenu(nullptr),
      recentFoldersMenu(nullptr), newAction(nullptr), openFolderAction(nullptr),
      saveAction(nullptr), saveAsAction(nullptr), exportHtmlAction(nullptr),
      exportPdfAction(nullptr), exportDocxAction(nullptr),
      exportPlainTextAction(nullptr), insertImageAction(nullptr),
      insertFormulaAction(nullptr), insertWikiLinkAction(nullptr),
      attachDocumentAction(nullptr), insertHeaderAction(nullptr),
       insertBoldAction(nullptr), insertItalicAction(nullptr),
       insertStrikethroughAction(nullptr),
      insertCodeAction(nullptr), insertCodeBlockAction(nullptr),
      insertListAction(nullptr), insertNumberedListAction(nullptr),
      insertBlockquoteAction(nullptr), insertHorizontalRuleAction(nullptr),
      insertLinkAction(nullptr), insertTableAction(nullptr),
      exitAction(nullptr), undoAction(nullptr), redoAction(nullptr),
      cutAction(nullptr), copyAction(nullptr), pasteAction(nullptr),
      findAction(nullptr), findReplaceAction(nullptr),
       searchInFilesAction(nullptr), quickOpenAction(nullptr),
       breakLinesAction(nullptr), joinLinesAction(nullptr),
       closeTabAction(nullptr), closeAllTabsAction(nullptr),
      toggleSidebarAction(nullptr), togglePreviewAction(nullptr),
      cycleViewModeAction(nullptr),
      previewThemeLightAction(nullptr), previewThemeDarkAction(nullptr),
      previewThemeSepiaAction(nullptr), settingsAction(nullptr),
      aboutAction(nullptr), aboutQtAction(nullptr),
      keyboardShortcutsAction(nullptr), currentViewMode(ViewMode_Both) {
  settings = new QSettings(APP_LABEL, APP_LABEL, this);
  linkParser = new LinkParser();
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
  createMenus();
  createToolbar();
  readSettings();
}

MainWindow::~MainWindow() { delete linkParser; }

void MainWindow::closeEvent(QCloseEvent *event) {
  if (maybeSave()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}
