#include "mainwindow.h"
#include "tabeditor.h"
#include "filesystemtreeview.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "markdownpreview.h"
#include "linkparser.h"
#include "searchdialog.h"
#include "settingsdialog.h"
#include "formuladialog.h"
#include "quickopendialog.h"
#include "outlinepanel.h"
#include <QApplication>
#include <QMenuBar>
#include <QActionGroup>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QStatusBar>
#include <QInputDialog>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDialog>
#include <QPushButton>
#include <QTabWidget>
#include <QListWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    settings = new QSettings("MkEd", "MkEd", this);
    linkParser = new LinkParser();
    
    setWindowTitle("MkEd - Markdown Editor");
    setWindowIcon(QIcon::fromTheme("text-editor"));
    
    statusBar()->showMessage(tr("Ready"));
    
    createLayout();
    createActions();
    createMenus();
    
    readSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new file"));
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    openFolderAction = new QAction(tr("Open &Folder..."), this);
    openFolderAction->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    openFolderAction->setStatusTip(tr("Open a folder"));
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);

    saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setShortcuts(QKeySequence::SaveAs);
    saveAsAction->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    
    insertImageAction = new QAction(tr("Insert &Image..."), this);
    insertImageAction->setShortcut(QKeySequence(tr("Ctrl+Shift+I")));
    insertImageAction->setStatusTip(tr("Insert an image into the document"));
    connect(insertImageAction, &QAction::triggered, this, &MainWindow::insertImage);
    
    insertFormulaAction = new QAction(tr("Insert &Formula..."), this);
    insertFormulaAction->setShortcut(QKeySequence(tr("Ctrl+Shift+M")));
    insertFormulaAction->setStatusTip(tr("Insert a LaTeX formula"));
    connect(insertFormulaAction, &QAction::triggered, this, &MainWindow::insertFormula);
    
    insertWikiLinkAction = new QAction(tr("Insert &Wiki Link..."), this);
    insertWikiLinkAction->setShortcut(QKeySequence(tr("Ctrl+K")));
    insertWikiLinkAction->setStatusTip(tr("Insert a wiki-style link"));
    connect(insertWikiLinkAction, &QAction::triggered, this, &MainWindow::insertWikiLink);
    
    insertHeaderAction = new QAction(tr("Insert &Header"), this);
    insertHeaderAction->setShortcut(QKeySequence(tr("Ctrl+1")));
    insertHeaderAction->setStatusTip(tr("Insert a header"));
    connect(insertHeaderAction, &QAction::triggered, this, &MainWindow::insertHeader);
    
    insertBoldAction = new QAction(tr("&Bold"), this);
    insertBoldAction->setShortcut(QKeySequence::Bold);
    insertBoldAction->setStatusTip(tr("Make text bold"));
    connect(insertBoldAction, &QAction::triggered, this, &MainWindow::insertBold);
    
    insertItalicAction = new QAction(tr("&Italic"), this);
    insertItalicAction->setShortcut(QKeySequence::Italic);
    insertItalicAction->setStatusTip(tr("Make text italic"));
    connect(insertItalicAction, &QAction::triggered, this, &MainWindow::insertItalic);
    
    insertCodeAction = new QAction(tr("Inline &Code"), this);
    insertCodeAction->setShortcut(QKeySequence(tr("Ctrl+`")));
    insertCodeAction->setStatusTip(tr("Insert inline code"));
    connect(insertCodeAction, &QAction::triggered, this, &MainWindow::insertCode);
    
    insertCodeBlockAction = new QAction(tr("Code &Block"), this);
    insertCodeBlockAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
    insertCodeBlockAction->setStatusTip(tr("Insert code block"));
    connect(insertCodeBlockAction, &QAction::triggered, this, &MainWindow::insertCodeBlock);
    
    insertListAction = new QAction(tr("Bulleted &List"), this);
    insertListAction->setShortcut(QKeySequence(tr("Ctrl+Shift+8")));
    insertListAction->setStatusTip(tr("Insert bulleted list"));
    connect(insertListAction, &QAction::triggered, this, &MainWindow::insertList);
    
    insertNumberedListAction = new QAction(tr("&Numbered List"), this);
    insertNumberedListAction->setShortcut(QKeySequence(tr("Ctrl+Shift+7")));
    insertNumberedListAction->setStatusTip(tr("Insert numbered list"));
    connect(insertNumberedListAction, &QAction::triggered, this, &MainWindow::insertNumberedList);
    
    insertBlockquoteAction = new QAction(tr("Block&quote"), this);
    insertBlockquoteAction->setShortcut(QKeySequence(tr("Ctrl+Shift+.")));
    insertBlockquoteAction->setStatusTip(tr("Insert blockquote"));
    connect(insertBlockquoteAction, &QAction::triggered, this, &MainWindow::insertBlockquote);
    
    insertHorizontalRuleAction = new QAction(tr("Horizontal &Rule"), this);
    insertHorizontalRuleAction->setShortcut(QKeySequence(tr("Ctrl+Shift+-")));
    insertHorizontalRuleAction->setStatusTip(tr("Insert horizontal rule"));
    connect(insertHorizontalRuleAction, &QAction::triggered, this, &MainWindow::insertHorizontalRule);
    
    insertLinkAction = new QAction(tr("Insert Lin&k..."), this);
    insertLinkAction->setShortcut(QKeySequence(tr("Ctrl+L")));
    insertLinkAction->setStatusTip(tr("Insert hyperlink"));
    connect(insertLinkAction, &QAction::triggered, this, &MainWindow::insertLink);
    
    insertTableAction = new QAction(tr("Insert &Table"), this);
    insertTableAction->setShortcut(QKeySequence(tr("Ctrl+Shift+T")));
    insertTableAction->setStatusTip(tr("Insert table"));
    connect(insertTableAction, &QAction::triggered, this, &MainWindow::insertTable);
    
    closeTabAction = new QAction(tr("&Close Tab"), this);
    closeTabAction->setShortcut(QKeySequence(tr("Ctrl+W")));
    closeTabAction->setStatusTip(tr("Close current tab"));
    connect(closeTabAction, &QAction::triggered, this, &MainWindow::closeCurrentTab);
    
    closeAllTabsAction = new QAction(tr("Close &All Tabs"), this);
    closeAllTabsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+W")));
    closeAllTabsAction->setStatusTip(tr("Close all tabs"));
    connect(closeAllTabsAction, &QAction::triggered, this, &MainWindow::closeAllTabs);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    undoAction = new QAction(tr("&Undo"), this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setStatusTip(tr("Undo the last operation"));
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, this, [this]() {
        TabEditor *tab = currentTabEditor();
        if (tab) tab->editor()->undo();
    });

    redoAction = new QAction(tr("&Redo"), this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setStatusTip(tr("Redo the last operation"));
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, this, [this]() {
        TabEditor *tab = currentTabEditor();
        if (tab) tab->editor()->redo();
    });

    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setStatusTip(tr("Cut the current selection"));
    cutAction->setEnabled(false);
    connect(cutAction, &QAction::triggered, this, [this]() {
        TabEditor *tab = currentTabEditor();
        if (tab) tab->editor()->cut();
    });

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy the current selection"));
    copyAction->setEnabled(false);
    connect(copyAction, &QAction::triggered, this, [this]() {
        TabEditor *tab = currentTabEditor();
        if (tab) tab->editor()->copy();
    });

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste the clipboard contents"));
    connect(pasteAction, &QAction::triggered, this, [this]() {
        TabEditor *tab = currentTabEditor();
        if (tab) tab->editor()->paste();
    });
    
    findAction = new QAction(tr("&Find..."), this);
    findAction->setShortcuts(QKeySequence::Find);
    findAction->setStatusTip(tr("Find text"));
    connect(findAction, &QAction::triggered, this, &MainWindow::find);
    
    findReplaceAction = new QAction(tr("Find and &Replace..."), this);
    findReplaceAction->setShortcut(QKeySequence(tr("Ctrl+H")));
    findReplaceAction->setStatusTip(tr("Find and replace text"));
    connect(findReplaceAction, &QAction::triggered, this, &MainWindow::findAndReplace);
    
    searchInFilesAction = new QAction(tr("Search in &Files..."), this);
    searchInFilesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+F")));
    searchInFilesAction->setStatusTip(tr("Search across all files"));
    connect(searchInFilesAction, &QAction::triggered, this, &MainWindow::searchInFiles);
    
    quickOpenAction = new QAction(tr("&Quick Open..."), this);
    quickOpenAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    quickOpenAction->setStatusTip(tr("Quickly open a file"));
    connect(quickOpenAction, &QAction::triggered, this, &MainWindow::quickOpen);

    toggleTreeViewAction = new QAction(tr("&File Tree"), this);
    toggleTreeViewAction->setCheckable(true);
    toggleTreeViewAction->setChecked(true);
    toggleTreeViewAction->setShortcut(QKeySequence(tr("Ctrl+T")));
    toggleTreeViewAction->setStatusTip(tr("Toggle file tree view"));
    connect(toggleTreeViewAction, &QAction::triggered, this, &MainWindow::toggleTreeView);

    togglePreviewAction = new QAction(tr("&Preview"), this);
    togglePreviewAction->setCheckable(true);
    togglePreviewAction->setChecked(true);
    togglePreviewAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    togglePreviewAction->setStatusTip(tr("Toggle preview panel"));
    connect(togglePreviewAction, &QAction::triggered, this, &MainWindow::togglePreview);
    
    toggleBacklinksAction = new QAction(tr("&Backlinks"), this);
    toggleBacklinksAction->setCheckable(true);
    toggleBacklinksAction->setChecked(false);
    toggleBacklinksAction->setShortcut(QKeySequence(tr("Ctrl+Shift+B")));
    toggleBacklinksAction->setStatusTip(tr("Toggle backlinks panel"));
    connect(toggleBacklinksAction, &QAction::triggered, this, &MainWindow::toggleBacklinks);
    
    toggleOutlineAction = new QAction(tr("&Outline"), this);
    toggleOutlineAction->setCheckable(true);
    toggleOutlineAction->setChecked(true);
    toggleOutlineAction->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    toggleOutlineAction->setStatusTip(tr("Toggle document outline"));
    connect(toggleOutlineAction, &QAction::triggered, this, &MainWindow::toggleOutline);
    
    previewThemeLightAction = new QAction(tr("Light Theme"), this);
    previewThemeLightAction->setCheckable(true);
    previewThemeLightAction->setChecked(true);
    connect(previewThemeLightAction, &QAction::triggered, this, &MainWindow::setPreviewThemeLight);
    
    previewThemeDarkAction = new QAction(tr("Dark Theme"), this);
    previewThemeDarkAction->setCheckable(true);
    connect(previewThemeDarkAction, &QAction::triggered, this, &MainWindow::setPreviewThemeDark);
    
    previewThemeSepiaAction = new QAction(tr("Sepia Theme"), this);
    previewThemeSepiaAction->setCheckable(true);
    connect(previewThemeSepiaAction, &QAction::triggered, this, &MainWindow::setPreviewThemeSepia);
    
    QActionGroup *themeGroup = new QActionGroup(this);
    themeGroup->addAction(previewThemeLightAction);
    themeGroup->addAction(previewThemeDarkAction);
    themeGroup->addAction(previewThemeSepiaAction);

    settingsAction = new QAction(tr("&Settings..."), this);
    settingsAction->setShortcut(QKeySequence(tr("Ctrl+,")));
    settingsAction->setStatusTip(tr("Open settings dialog"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openFolderAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
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
    editMenu->addAction(quickOpenAction);
    
    insertMenu = menuBar()->addMenu(tr("&Insert"));
    insertMenu->addAction(insertImageAction);
    insertMenu->addAction(insertFormulaAction);
    insertMenu->addSeparator();
    insertMenu->addAction(insertWikiLinkAction);
    insertMenu->addAction(insertLinkAction);
    insertMenu->addSeparator();
    insertMenu->addAction(insertHeaderAction);
    insertMenu->addAction(insertBoldAction);
    insertMenu->addAction(insertItalicAction);
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
    viewMenu->addAction(toggleTreeViewAction);
    viewMenu->addAction(togglePreviewAction);
    viewMenu->addAction(toggleBacklinksAction);
    viewMenu->addAction(toggleOutlineAction);
    viewMenu->addSeparator();
    QMenu *previewThemeMenu = viewMenu->addMenu(tr("Preview Theme"));
    previewThemeMenu->addAction(previewThemeLightAction);
    previewThemeMenu->addAction(previewThemeDarkAction);
    previewThemeMenu->addAction(previewThemeSepiaAction);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::createLayout()
{
    // Tree panel
    treePanel = new QWidget(this);
    QVBoxLayout *treeLayout = new QVBoxLayout(treePanel);
    treeLayout->setContentsMargins(0, 0, 0, 0);
    
    treeView = new FileSystemTreeView(treePanel);
    treeLayout->addWidget(treeView);
    treePanel->setMinimumWidth(150);
    
    connect(treeView, &FileSystemTreeView::fileSelected,
            this, &MainWindow::onFileSelected);
    connect(treeView, &FileSystemTreeView::fileDoubleClicked,
            this, &MainWindow::onFileDoubleClicked);
    connect(treeView, &FileSystemTreeView::fileModifiedExternally,
            this, &MainWindow::onFileModifiedExternally);

    // Tab widget for multiple editors
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    
    connect(tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
    
    // Backlinks panel
    backlinksPanel = new QWidget(this);
    QVBoxLayout *backlinksLayout = new QVBoxLayout(backlinksPanel);
    backlinksLayout->setContentsMargins(5, 5, 5, 5);
    
    QLabel *backlinksLabel = new QLabel(tr("Backlinks"), backlinksPanel);
    backlinksLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    backlinksLayout->addWidget(backlinksLabel);
    
    backlinksView = new QListWidget(backlinksPanel);
    backlinksLayout->addWidget(backlinksView);
    backlinksPanel->setMinimumWidth(150);
    backlinksPanel->setVisible(false);
    
    connect(backlinksView, &QListWidget::itemDoubleClicked,
            [this](QListWidgetItem *item) {
                QString filePath = item->data(Qt::UserRole).toString();
                if (!filePath.isEmpty()) {
                    loadFile(filePath);
                }
            });

    // Main splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(treePanel);
    mainSplitter->addWidget(tabWidget);
    mainSplitter->addWidget(backlinksPanel);
    mainSplitter->setStretchFactor(0, 0);  // Tree panel
    mainSplitter->setStretchFactor(1, 1);  // Tab widget gets most space
    mainSplitter->setStretchFactor(2, 0);  // Backlinks panel
    
    setCentralWidget(mainSplitter);
    
    // Create initial tab
    createNewTab();
}

void MainWindow::readSettings()
{
    QPoint pos = settings->value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings->value("size", QSize(1024, 768)).toSize();
    resize(size);
    move(pos);

    if (settings->contains("mainSplitter"))
        mainSplitter->restoreState(settings->value("mainSplitter").toByteArray());

    bool treeVisible = settings->value("treeVisible", true).toBool();
    treePanel->setVisible(treeVisible);
    toggleTreeViewAction->setChecked(treeVisible);
    
    QString lastFolder = settings->value("lastFolder").toString();
    if (!lastFolder.isEmpty() && QDir(lastFolder).exists()) {
        treeView->setRootPath(lastFolder);
        currentFolder = lastFolder;
        
        TabEditor *tab = currentTabEditor();
        if (tab && tab->editor()->getHighlighter()) {
            tab->editor()->getHighlighter()->setRootPath(lastFolder);
        }
        
        linkParser->buildLinkIndex(lastFolder);
        
        statusBar()->showMessage(tr("Opened folder: %1").arg(lastFolder));
    }
}

void MainWindow::writeSettings()
{
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->setValue("mainSplitter", mainSplitter->saveState());
    settings->setValue("treeVisible", treePanel->isVisible());
    settings->setValue("lastFolder", currentFolder);
}

void MainWindow::newFile()
{
    createNewTab();
}

void MainWindow::openFolder()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Open Folder"),
        currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folder.isEmpty()) {
        treeView->setRootPath(folder);
        currentFolder = folder;
        
        TabEditor *tab = currentTabEditor();
        if (tab && tab->editor()->getHighlighter()) {
            tab->editor()->getHighlighter()->setRootPath(folder);
        }
        
        linkParser->buildLinkIndex(folder);
        
        statusBar()->showMessage(tr("Opened folder: %1").arg(folder));
    }
}

void MainWindow::save()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) {
        return;
    }
    
    if (tab->filePath().isEmpty()) {
        saveAs();
    } else {
        if (tab->saveFile()) {
            statusBar()->showMessage(tr("File saved"), 3000);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Could not save file!"));
        }
    }
}

void MainWindow::saveAs()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) {
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
        tr("Markdown Files (*.md *.markdown);;All Files (*)")
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    if (tab->saveFileAs(filePath)) {
        currentFilePath = filePath;
        linkParser->buildLinkIndex(currentFolder);
        statusBar()->showMessage(tr("File saved as: %1").arg(QFileInfo(filePath).fileName()), 3000);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not save file!"));
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About MkEd"),
        tr("<h2>MkEd - Markdown Editor</h2>"
           "<p>Version 0.1.0</p>"
           "<p>A feature-rich Markdown text editor built with Qt 6, "
           "designed for organizing and managing interconnected notes.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>File system tree view</li>"
           "<li>Wiki-style linking</li>"
           "<li>Markdown preview</li>"
           "<li>Image embedding</li>"
           "<li>LaTeX formula rendering</li>"
           "</ul>"
           "<p>Copyright © 2024</p>"));
}

void MainWindow::toggleTreeView()
{
    treePanel->setVisible(toggleTreeViewAction->isChecked());
}

void MainWindow::togglePreview()
{
    TabEditor *tab = currentTabEditor();
    if (tab) {
        tab->preview()->setVisible(togglePreviewAction->isChecked());
    }
}

void MainWindow::toggleOutline()
{
    TabEditor *tab = currentTabEditor();
    if (tab) {
        tab->outline()->setVisible(toggleOutlineAction->isChecked());
    }
}

void MainWindow::toggleBacklinks()
{
    backlinksPanel->setVisible(toggleBacklinksAction->isChecked());
}

void MainWindow::onWikiLinkClicked(const QString &linkTarget)
{
    if (currentFilePath.isEmpty()) {
        statusBar()->showMessage(tr("No file open"), 3000);
        return;
    }
    
    QString targetPath = linkParser->resolveLinkTarget(linkTarget, currentFilePath);
    
    if (targetPath.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Link Not Found"),
            tr("The file '%1' does not exist in the current directory or its subdirectories.\n\nDo you want to create it?").arg(linkTarget),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            QString fileName = linkTarget;
            if (!fileName.endsWith(".md") && !fileName.endsWith(".markdown")) {
                fileName += ".md";
            }
            
            // Create in the same directory as current file
            QFileInfo currentFileInfo(currentFilePath);
            QString newFilePath = currentFileInfo.dir().filePath(fileName);
            QFile file(newFilePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(QString("# %1\n\n").arg(linkTarget).toUtf8());
                file.close();
                loadFile(newFilePath);
            }
        }
    } else {
        if (!maybeSave()) {
            return;
        }
        loadFile(targetPath);
    }
}

void MainWindow::updateBacklinks()
{
    backlinksView->clear();
    
    if (currentFilePath.isEmpty() || currentFolder.isEmpty()) {
        return;
    }
    
    QVector<QString> backlinks = linkParser->getBacklinks(currentFilePath);
    
    for (const QString &backlink : backlinks) {
        QFileInfo fileInfo(backlink);
        QListWidgetItem *item = new QListWidgetItem(fileInfo.fileName());
        item->setData(Qt::UserRole, backlink);
        item->setToolTip(backlink);
        backlinksView->addItem(item);
    }
    
    if (backlinks.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(tr("No backlinks"));
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        backlinksView->addItem(item);
    }
}

void MainWindow::onFileSelected(const QString &filePath)
{
    statusBar()->showMessage(tr("Selected: %1").arg(filePath));
}

void MainWindow::onFileDoubleClicked(const QString &filePath)
{
    if (!maybeSave()) {
        return;
    }
    
    loadFile(filePath);
}

void MainWindow::onFileModifiedExternally(const QString &filePath)
{
    if (filePath != currentFilePath) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    statusBar()->showMessage(tr("File modified externally: %1").arg(fileInfo.fileName()), 5000);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("File Modified"),
        tr("The file has been modified by another application:\n\n%1\n\n"
           "Do you want to reload it? (Unsaved changes will be lost)").arg(fileInfo.fileName()),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        loadFile(filePath);
    }
}

void MainWindow::onDocumentModified()
{
    // This is now handled per-tab by connecting each tab's signals
    // when tabs are created. Keep this for future use if needed.
}

void MainWindow::autoSave()
{
    TabEditor *tab = currentTabEditor();
    if (!tab || tab->filePath().isEmpty() || !tab->isModified()) {
        return;
    }
    
    if (tab->saveFile()) {
        statusBar()->showMessage(tr("Auto-saved"), 2000);
    }
}

bool MainWindow::maybeSave()
{
    TabEditor *tab = currentTabEditor();
    if (!tab || !tab->isModified()) {
        return true;
    }
    
    QString fileName = tab->fileName();
    if (fileName.isEmpty()) {
        fileName = tr("Untitled");
    }
    
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this, tr("Unsaved Changes"),
        tr("The document '%1' has unsaved changes.\n\n"
           "Do you want to save them?").arg(fileName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );
    
    if (reply == QMessageBox::Save) {
        if (tab->filePath().isEmpty()) {
            saveAs();
            return !tab->filePath().isEmpty();
        }
        return tab->saveFile();
    } else if (reply == QMessageBox::Cancel) {
        return false;
    }
    
    return true;
}

bool MainWindow::loadFile(const QString &filePath)
{
    // Check if file is already open in a tab
    int existingIndex = findTabIndexByPath(filePath);
    if (existingIndex >= 0) {
        tabWidget->setCurrentIndex(existingIndex);
        return true;
    }
    
    // Load in current tab if it's empty and unmodified
    TabEditor *tab = currentTabEditor();
    if (tab && tab->filePath().isEmpty() && !tab->isModified()) {
        if (tab->loadFile(filePath)) {
            currentFilePath = filePath;
            
            // Update recent files
            if (!recentFiles.contains(filePath)) {
                recentFiles.prepend(filePath);
                if (recentFiles.size() > 10) {
                    recentFiles.removeLast();
                }
            }
            
            updateBacklinks();
            setWindowTitle(QString("%1 - MkEd").arg(QFileInfo(filePath).fileName()));
            statusBar()->showMessage(tr("Loaded: %1").arg(QFileInfo(filePath).fileName()), 3000);
            return true;
        }
    } else {
        // Create new tab
        tab = createNewTab();
        if (tab->loadFile(filePath)) {
            currentFilePath = filePath;
            
            // Update recent files
            if (!recentFiles.contains(filePath)) {
                recentFiles.prepend(filePath);
                if (recentFiles.size() > 10) {
                    recentFiles.removeLast();
                }
            }
            
            updateBacklinks();
            setWindowTitle(QString("%1 - MkEd").arg(QFileInfo(filePath).fileName()));
            statusBar()->showMessage(tr("Loaded: %1").arg(QFileInfo(filePath).fileName()), 3000);
            return true;
        }
    }
    
    QMessageBox::warning(this, tr("Error"), tr("Could not load file!"));
    return false;
}

void MainWindow::find()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    bool ok;
    QString searchText = QInputDialog::getText(this, tr("Find"),
                                              tr("Find text:"), QLineEdit::Normal,
                                              "", &ok);
    
    if (!ok || searchText.isEmpty()) {
        return;
    }
    
    QTextCursor cursor = tab->editor()->textCursor();
    QTextDocument::FindFlags flags;
    
    QTextCursor foundCursor = tab->editor()->document()->find(searchText, cursor, flags);
    
    if (foundCursor.isNull()) {
        foundCursor = tab->editor()->document()->find(searchText, 0, flags);
    }
    
    if (!foundCursor.isNull()) {
        tab->editor()->setTextCursor(foundCursor);
        statusBar()->showMessage(tr("Found: %1").arg(searchText), 3000);
    } else {
        QMessageBox::information(this, tr("Find"),
            tr("Text not found: %1").arg(searchText));
    }
}

void MainWindow::findAndReplace()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find and Replace"));
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    QFormLayout *formLayout = new QFormLayout();
    QLineEdit *findEdit = new QLineEdit(&dialog);
    QLineEdit *replaceEdit = new QLineEdit(&dialog);
    
    formLayout->addRow(tr("Find:"), findEdit);
    formLayout->addRow(tr("Replace with:"), replaceEdit);
    
    mainLayout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *findNextButton = new QPushButton(tr("Find Next"), &dialog);
    QPushButton *replaceButton = new QPushButton(tr("Replace"), &dialog);
    QPushButton *replaceAllButton = new QPushButton(tr("Replace All"), &dialog);
    QPushButton *closeButton = new QPushButton(tr("Close"), &dialog);
    
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(findNextButton, &QPushButton::clicked, [tab, findEdit, this]() {
        QString searchText = findEdit->text();
        if (searchText.isEmpty()) {
            return;
        }
        
        QTextCursor cursor = tab->editor()->textCursor();
        QTextDocument::FindFlags flags;
        
        QTextCursor foundCursor = tab->editor()->document()->find(searchText, cursor, flags);
        
        if (foundCursor.isNull()) {
            foundCursor = tab->editor()->document()->find(searchText, 0, flags);
        }
        
        if (!foundCursor.isNull()) {
            tab->editor()->setTextCursor(foundCursor);
        } else {
            QMessageBox::information(this, tr("Find"), tr("Text not found"));
        }
    });
    
    connect(replaceButton, &QPushButton::clicked, [tab, findEdit, replaceEdit, this]() {
        QTextCursor cursor = tab->editor()->textCursor();
        if (cursor.hasSelection() && cursor.selectedText() == findEdit->text()) {
            cursor.insertText(replaceEdit->text());
            statusBar()->showMessage(tr("Replaced"), 2000);
        }
    });
    
    connect(replaceAllButton, &QPushButton::clicked, [tab, findEdit, replaceEdit, &dialog, this]() {
        QString searchText = findEdit->text();
        QString replaceText = replaceEdit->text();
        
        if (searchText.isEmpty()) {
            return;
        }
        
        QTextCursor cursor = tab->editor()->document()->find(searchText);
        int count = 0;
        
        tab->editor()->textCursor().beginEditBlock();
        
        while (!cursor.isNull()) {
            cursor.insertText(replaceText);
            cursor = tab->editor()->document()->find(searchText, cursor);
            count++;
        }
        
        tab->editor()->textCursor().endEditBlock();
        
        QMessageBox::information(&dialog, tr("Replace All"),
            tr("Replaced %1 occurrence(s)").arg(count));
        
        statusBar()->showMessage(tr("Replaced %1 occurrence(s)").arg(count), 3000);
    });
    
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    dialog.exec();
}

void MainWindow::updatePreview()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QString markdown = tab->editor()->toPlainText();
    tab->preview()->setMarkdownContent(markdown);
    tab->outline()->updateOutline(markdown);
}

void MainWindow::setPreviewThemeLight()
{
    TabEditor *tab = currentTabEditor();
    if (tab) tab->preview()->setTheme("light");
}

void MainWindow::setPreviewThemeDark()
{
    TabEditor *tab = currentTabEditor();
    if (tab) tab->preview()->setTheme("dark");
}

void MainWindow::setPreviewThemeSepia()
{
    TabEditor *tab = currentTabEditor();
    if (tab) tab->preview()->setTheme("sepia");
}

void MainWindow::searchInFiles()
{
    if (currentFolder.isEmpty()) {
        QMessageBox::information(this, tr("No Folder Open"),
            tr("Please open a folder first (File → Open Folder)."));
        return;
    }
    
    SearchDialog *dialog = new SearchDialog(currentFolder, this);
    connect(dialog, &SearchDialog::fileSelected,
            this, &MainWindow::onSearchResultSelected);
    dialog->exec();
    delete dialog;
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Apply settings
        if (dialog.getAutoSaveEnabled()) {
            autoSaveTimer->start(dialog.getAutoSaveInterval() * 1000);
        } else {
            autoSaveTimer->stop();
        }
        
        // Apply theme to current tab
        QString theme = dialog.getDefaultTheme();
        TabEditor *tab = currentTabEditor();
        if (tab) {
            tab->preview()->setTheme(theme);
        }
        
        // Update theme action checkboxes
        if (theme == "dark") {
            previewThemeDarkAction->setChecked(true);
        } else if (theme == "sepia") {
            previewThemeSepiaAction->setChecked(true);
        } else {
            previewThemeLightAction->setChecked(true);
        }
        
        statusBar()->showMessage(tr("Settings saved"), 3000);
    }
}

void MainWindow::onSearchResultSelected(const QString &filePath, int lineNumber)
{
    if (!maybeSave()) {
        return;
    }
    
    if (loadFile(filePath)) {
        TabEditor *tab = currentTabEditor();
        if (tab) {
            // Move cursor to the specified line
            QTextCursor cursor = tab->editor()->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
            tab->editor()->setTextCursor(cursor);
            tab->editor()->centerCursor();
        
            // Add to recent files
            if (!recentFiles.contains(filePath)) {
                recentFiles.prepend(filePath);
                if (recentFiles.size() > 10) {
                    recentFiles.removeLast();
                }
            }
        }
        
        statusBar()->showMessage(tr("Jumped to line %1").arg(lineNumber), 3000);
    }
}

void MainWindow::insertImage()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QString imagePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Image"),
        currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
        tr("Images (*.png *.jpg *.jpeg *.gif *.bmp *.svg);;All Files (*)")
    );
    
    if (imagePath.isEmpty()) {
        return;
    }
    
    // Get alt text from user
    bool ok;
    QString altText = QInputDialog::getText(
        this,
        tr("Image Alt Text"),
        tr("Enter alt text for the image (optional):"),
        QLineEdit::Normal,
        QFileInfo(imagePath).baseName(),
        &ok
    );
    
    if (!ok) {
        return;
    }
    
    // Convert to relative path if possible
    QString finalPath = imagePath;
    if (!currentFolder.isEmpty() && !currentFilePath.isEmpty()) {
        QDir currentDir = QFileInfo(currentFilePath).dir();
        finalPath = currentDir.relativeFilePath(imagePath);
    }
    
    // Insert Markdown image syntax
    QString imageMarkdown = QString("![%1](%2)").arg(altText, finalPath);
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(imageMarkdown);
    
    statusBar()->showMessage(tr("Image inserted"), 3000);
}

void MainWindow::insertFormula()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    FormulaDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString formula = dialog.getFormula();
        
        if (!formula.isEmpty()) {
            QTextCursor cursor = tab->editor()->textCursor();
            
            if (dialog.isBlockFormula()) {
                // Block formula - add on new lines
                cursor.insertText("\n" + formula + "\n");
            } else {
                // Inline formula
                cursor.insertText(formula);
            }
            
            statusBar()->showMessage(tr("Formula inserted"), 3000);
        }
    }
}

void MainWindow::insertWikiLink()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    if (currentFilePath.isEmpty()) {
        QMessageBox::information(this, tr("No File Open"),
            tr("Please open or create a file first."));
        return;
    }
    
    // Get directory of current file
    QFileInfo currentFileInfo(currentFilePath);
    QDir currentDir = currentFileInfo.dir();
    
    // Show file browser to select target file
    QString targetPath = QFileDialog::getOpenFileName(
        this,
        tr("Select Target File"),
        currentDir.path(),
        tr("Markdown Files (*.md *.markdown);;All Files (*)")
    );
    
    if (targetPath.isEmpty()) {
        return;
    }
    
    // Get just the filename without extension
    QFileInfo fileInfo(targetPath);
    QString linkTarget = fileInfo.completeBaseName();
    
    // Ask for display text
    bool ok;
    QString displayText = QInputDialog::getText(
        this,
        tr("Wiki Link Display Text"),
        tr("Enter display text (leave empty to use filename):"),
        QLineEdit::Normal,
        linkTarget,
        &ok
    );
    
    if (!ok) {
        return;
    }
    
    // Insert wiki link
    QString wikiLink;
    if (displayText.isEmpty() || displayText == linkTarget) {
        wikiLink = QString("[[%1]]").arg(linkTarget);
    } else {
        wikiLink = QString("[[%1|%2]]").arg(linkTarget, displayText);
    }
    
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(wikiLink);
    
    statusBar()->showMessage(tr("Wiki link inserted (relative to current file)"), 3000);
}

void MainWindow::insertHeader()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    bool ok;
    QStringList items;
    items << "# Header 1" << "## Header 2" << "### Header 3" 
          << "#### Header 4" << "##### Header 5" << "###### Header 6";
    
    QString item = QInputDialog::getItem(this, tr("Insert Header"),
                                         tr("Select header level:"), items, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        QTextCursor cursor = tab->editor()->textCursor();
        cursor.insertText(item + " ");
        statusBar()->showMessage(tr("Header inserted"), 3000);
    }
}

void MainWindow::insertBold()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    QString selectedText = cursor.selectedText();
    
    if (selectedText.isEmpty()) {
        cursor.insertText("**bold text**");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
    } else {
        cursor.insertText("**" + selectedText + "**");
    }
    
    tab->editor()->setTextCursor(cursor);
    statusBar()->showMessage(tr("Bold formatting applied"), 3000);
}

void MainWindow::insertItalic()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    QString selectedText = cursor.selectedText();
    
    if (selectedText.isEmpty()) {
        cursor.insertText("*italic text*");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    } else {
        cursor.insertText("*" + selectedText + "*");
    }
    
    tab->editor()->setTextCursor(cursor);
    statusBar()->showMessage(tr("Italic formatting applied"), 3000);
}

void MainWindow::insertCode()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    QString selectedText = cursor.selectedText();
    
    if (selectedText.isEmpty()) {
        cursor.insertText("`code`");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    } else {
        cursor.insertText("`" + selectedText + "`");
    }
    
    tab->editor()->setTextCursor(cursor);
    statusBar()->showMessage(tr("Code formatting applied"), 3000);
}

void MainWindow::insertCodeBlock()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    bool ok;
    QString language = QInputDialog::getText(
        this,
        tr("Code Block"),
        tr("Enter language (optional):"),
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (ok) {
        QTextCursor cursor = tab->editor()->textCursor();
        QString codeBlock = QString("\n```%1\nYour code here\n```\n").arg(language);
        cursor.insertText(codeBlock);
        statusBar()->showMessage(tr("Code block inserted"), 3000);
    }
}

void MainWindow::insertList()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText("\n- List item 1\n- List item 2\n- List item 3\n");
    statusBar()->showMessage(tr("List inserted"), 3000);
}

void MainWindow::insertNumberedList()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText("\n1. First item\n2. Second item\n3. Third item\n");
    statusBar()->showMessage(tr("Numbered list inserted"), 3000);
}

void MainWindow::insertBlockquote()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    QString selectedText = cursor.selectedText();
    
    if (selectedText.isEmpty()) {
        cursor.insertText("\n> Quote text here\n");
    } else {
        cursor.insertText("\n> " + selectedText + "\n");
    }
    
    statusBar()->showMessage(tr("Blockquote inserted"), 3000);
}

void MainWindow::insertHorizontalRule()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText("\n---\n");
    statusBar()->showMessage(tr("Horizontal rule inserted"), 3000);
}

void MainWindow::insertLink()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    bool ok;
    QString text = QInputDialog::getText(
        this,
        tr("Insert Link"),
        tr("Enter link text:"),
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || text.isEmpty()) {
        return;
    }
    
    QString url = QInputDialog::getText(
        this,
        tr("Insert Link"),
        tr("Enter URL:"),
        QLineEdit::Normal,
        "https://",
        &ok
    );
    
    if (ok && !url.isEmpty()) {
        QTextCursor cursor = tab->editor()->textCursor();
        cursor.insertText(QString("[%1](%2)").arg(text, url));
        statusBar()->showMessage(tr("Link inserted"), 3000);
    }
}

void MainWindow::insertTable()
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    bool ok;
    int rows = QInputDialog::getInt(
        this,
        tr("Insert Table"),
        tr("Number of rows:"),
        3, 2, 20, 1, &ok
    );
    
    if (!ok) {
        return;
    }
    
    int cols = QInputDialog::getInt(
        this,
        tr("Insert Table"),
        tr("Number of columns:"),
        3, 2, 10, 1, &ok
    );
    
    if (!ok) {
        return;
    }
    
    QString table = "\n";
    
    // Header row
    table += "|";
    for (int c = 0; c < cols; ++c) {
        table += QString(" Header %1 |").arg(c + 1);
    }
    table += "\n";
    
    // Separator row
    table += "|";
    for (int c = 0; c < cols; ++c) {
        table += " --- |";
    }
    table += "\n";
    
    // Data rows
    for (int r = 0; r < rows - 1; ++r) {
        table += "|";
        for (int c = 0; c < cols; ++c) {
            table += " Cell |";
        }
        table += "\n";
    }
    
    table += "\n";
    
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(table);
    statusBar()->showMessage(tr("Table inserted"), 3000);
}

void MainWindow::quickOpen()
{
    if (currentFolder.isEmpty()) {
        QMessageBox::information(this, tr("No Folder Open"),
            tr("Please open a folder first."));
        return;
    }
    
    QuickOpenDialog dialog(currentFolder, recentFiles, this);
    connect(&dialog, &QuickOpenDialog::fileSelected,
            this, [this](const QString &filePath) {
        if (!maybeSave()) {
            return;
        }
        loadFile(filePath);
    });
    dialog.exec();
}

void MainWindow::jumpToLine(int lineNumber)
{
    TabEditor *tab = currentTabEditor();
    if (!tab) return;
    
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
    tab->editor()->setTextCursor(cursor);
    tab->editor()->centerCursor();
    tab->editor()->setFocus();
}

TabEditor* MainWindow::currentTabEditor() const
{
    return qobject_cast<TabEditor*>(tabWidget->currentWidget());
}

TabEditor* MainWindow::createNewTab()
{
    TabEditor *tab = new TabEditor(this);
    
    // Connect wiki link clicks
    connect(tab->editor(), &MarkdownEditor::wikiLinkClicked,
            this, &MainWindow::onWikiLinkClicked);
    
    // Connect outline clicks
    connect(tab->outline(), &OutlinePanel::headerClicked,
            this, &MainWindow::jumpToLine);
    
    // Connect modification signal
    connect(tab, &TabEditor::modificationChanged,
            this, [this, tab](bool modified) {
        int index = tabWidget->indexOf(tab);
        if (index >= 0) {
            QString tabText = tab->fileName();
            if (modified) {
                tabText += " *";
            }
            tabWidget->setTabText(index, tabText);
        }
    });
    
    // Connect file path changes
    connect(tab, &TabEditor::filePathChanged,
            this, [this, tab](const QString &) {
        int index = tabWidget->indexOf(tab);
        if (index >= 0) {
            tabWidget->setTabText(index, tab->fileName());
            tabWidget->setTabToolTip(index, tab->filePath());
        }
    });
    
    int index = tabWidget->addTab(tab, tr("Untitled"));
    tabWidget->setCurrentIndex(index);
    
    return tab;
}

TabEditor* MainWindow::findTabByPath(const QString &filePath) const
{
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor *tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->filePath() == filePath) {
            return tab;
        }
    }
    return nullptr;
}

int MainWindow::findTabIndexByPath(const QString &filePath) const
{
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor *tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->filePath() == filePath) {
            return i;
        }
    }
    return -1;
}

void MainWindow::onTabChanged(int index)
{
    TabEditor *tab = qobject_cast<TabEditor*>(tabWidget->widget(index));
    if (tab) {
        currentFilePath = tab->filePath();
        updateBacklinks();
        
        // Update window title
        if (!tab->filePath().isEmpty()) {
            setWindowTitle(QString("%1 - MkEd").arg(tab->fileName()));
        } else {
            setWindowTitle("MkEd");
        }
    }
}

void MainWindow::onTabCloseRequested(int index)
{
    TabEditor *tab = qobject_cast<TabEditor*>(tabWidget->widget(index));
    if (!tab) {
        return;
    }
    
    // Check if modified
    if (tab->isModified()) {
        tabWidget->setCurrentIndex(index);
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("'%1' has unsaved changes. Do you want to save before closing?").arg(tab->fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            if (tab->filePath().isEmpty()) {
                QString filePath = QFileDialog::getSaveFileName(
                    this,
                    tr("Save File"),
                    currentFolder,
                    tr("Markdown Files (*.md *.markdown);;All Files (*)")
                );
                
                if (filePath.isEmpty()) {
                    return;
                }
                
                if (!tab->saveFileAs(filePath)) {
                    QMessageBox::warning(this, tr("Error"), tr("Could not save file!"));
                    return;
                }
            } else {
                if (!tab->saveFile()) {
                    QMessageBox::warning(this, tr("Error"), tr("Could not save file!"));
                    return;
                }
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    tabWidget->removeTab(index);
    delete tab;
    
    // Create new tab if all closed
    if (tabWidget->count() == 0) {
        createNewTab();
    }
}

void MainWindow::closeCurrentTab()
{
    int index = tabWidget->currentIndex();
    if (index >= 0) {
        onTabCloseRequested(index);
    }
}

void MainWindow::closeAllTabs()
{
    while (tabWidget->count() > 0) {
        onTabCloseRequested(0);
        if (tabWidget->count() > 0 && qobject_cast<TabEditor*>(tabWidget->widget(0))->isModified()) {
            // User cancelled, stop closing
            break;
        }
    }
}
