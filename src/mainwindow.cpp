#include "mainwindow.h"
#include "filesystemtreeview.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "markdownpreview.h"
#include "linkparser.h"
#include "searchdialog.h"
#include "settingsdialog.h"
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
#include <QListWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    settings = new QSettings("MkEd", "MkEd", this);
    linkParser = new LinkParser();
    
    setWindowTitle("MkEd - Markdown Editor");
    setWindowIcon(QIcon::fromTheme("text-editor"));
    
    statusBar()->showMessage(tr("Ready"));
    
    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    int autoSaveInterval = settings->value("autoSaveInterval", 60).toInt();
    autoSaveTimer->start(autoSaveInterval * 1000);
    
    previewUpdateTimer = new QTimer(this);
    previewUpdateTimer->setSingleShot(true);
    connect(previewUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePreview);
    
    createLayout();
    createActions();
    createMenus();
    
    // Connect editor signals to actions (must be after both are created)
    connect(editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::onDocumentModified);
    connect(editor->document(), &QTextDocument::undoAvailable,
            undoAction, &QAction::setEnabled);
    connect(editor->document(), &QTextDocument::redoAvailable,
            redoAction, &QAction::setEnabled);
    connect(editor, &MarkdownEditor::copyAvailable,
            cutAction, &QAction::setEnabled);
    connect(editor, &MarkdownEditor::copyAvailable,
            copyAction, &QAction::setEnabled);
    connect(editor->document(), &QTextDocument::contentsChanged,
            [this]() { previewUpdateTimer->start(500); });
    
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

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    undoAction = new QAction(tr("&Undo"), this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setStatusTip(tr("Undo the last operation"));
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, editor, &MarkdownEditor::undo);

    redoAction = new QAction(tr("&Redo"), this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setStatusTip(tr("Redo the last operation"));
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, editor, &MarkdownEditor::redo);

    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setStatusTip(tr("Cut the current selection"));
    cutAction->setEnabled(false);
    connect(cutAction, &QAction::triggered, editor, &MarkdownEditor::cut);

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy the current selection"));
    copyAction->setEnabled(false);
    connect(copyAction, &QAction::triggered, editor, &MarkdownEditor::copy);

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste the clipboard contents"));
    connect(pasteAction, &QAction::triggered, editor, &MarkdownEditor::paste);
    
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
    toggleBacklinksAction->setShortcut(QKeySequence(tr("Ctrl+B")));
    toggleBacklinksAction->setStatusTip(tr("Toggle backlinks panel"));
    connect(toggleBacklinksAction, &QAction::triggered, this, &MainWindow::togglePreview);
    
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

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(toggleTreeViewAction);
    viewMenu->addAction(togglePreviewAction);
    viewMenu->addAction(toggleBacklinksAction);
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

    editorPanel = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout(editorPanel);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    
    editor = new MarkdownEditor(editorPanel);
    editorLayout->addWidget(editor);
    editorPanel->setMinimumWidth(200);
    
    connect(editor, &MarkdownEditor::wikiLinkClicked,
            this, &MainWindow::onWikiLinkClicked);

    previewPanel = new QWidget(this);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewPanel);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    
    preview = new MarkdownPreview(previewPanel);
    previewLayout->addWidget(preview);
    previewPanel->setMinimumWidth(200);
    
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

    rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(previewPanel);
    rightSplitter->addWidget(backlinksPanel);
    rightSplitter->setStretchFactor(0, 3);
    rightSplitter->setStretchFactor(1, 1);

    editorSplitter = new QSplitter(Qt::Horizontal, this);
    editorSplitter->addWidget(editorPanel);
    editorSplitter->addWidget(rightSplitter);
    editorSplitter->setStretchFactor(0, 1);
    editorSplitter->setStretchFactor(1, 1);

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(treePanel);
    mainSplitter->addWidget(editorSplitter);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    setCentralWidget(mainSplitter);
}

void MainWindow::readSettings()
{
    QPoint pos = settings->value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings->value("size", QSize(1024, 768)).toSize();
    resize(size);
    move(pos);

    if (settings->contains("mainSplitter"))
        mainSplitter->restoreState(settings->value("mainSplitter").toByteArray());
    if (settings->contains("editorSplitter"))
        editorSplitter->restoreState(settings->value("editorSplitter").toByteArray());

    bool treeVisible = settings->value("treeVisible", true).toBool();
    treePanel->setVisible(treeVisible);
    toggleTreeViewAction->setChecked(treeVisible);

    bool previewVisible = settings->value("previewVisible", true).toBool();
    previewPanel->setVisible(previewVisible);
    togglePreviewAction->setChecked(previewVisible);
    
    QString lastFolder = settings->value("lastFolder").toString();
    if (!lastFolder.isEmpty() && QDir(lastFolder).exists()) {
        treeView->setRootPath(lastFolder);
        currentFolder = lastFolder;
        
        if (editor->getHighlighter()) {
            editor->getHighlighter()->setRootPath(lastFolder);
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
    settings->setValue("editorSplitter", editorSplitter->saveState());
    settings->setValue("treeVisible", treePanel->isVisible());
    settings->setValue("previewVisible", previewPanel->isVisible());
    settings->setValue("lastFolder", currentFolder);
}

void MainWindow::newFile()
{
    if (currentFolder.isEmpty()) {
        QMessageBox::information(this, tr("No Folder Open"), 
            tr("Please open a folder first (File → Open Folder)."));
        return;
    }
    
    bool ok;
    QString fileName = QInputDialog::getText(this, tr("New File"),
                                            tr("File name:"), QLineEdit::Normal,
                                            "untitled.md", &ok);
    
    if (!ok || fileName.isEmpty()) {
        return;
    }
    
    if (!fileName.endsWith(".md") && !fileName.endsWith(".markdown")) {
        fileName += ".md";
    }
    
    QString filePath = QDir(currentFolder).filePath(fileName);
    
    if (QFile::exists(filePath)) {
        QMessageBox::warning(this, tr("Error"), tr("File already exists!"));
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create file!"));
        return;
    }
    file.close();
    
    statusBar()->showMessage(tr("Created file: %1").arg(fileName));
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
        
        if (editor->getHighlighter()) {
            editor->getHighlighter()->setRootPath(folder);
        }
        
        linkParser->buildLinkIndex(folder);
        
        statusBar()->showMessage(tr("Opened folder: %1").arg(folder));
    }
}

void MainWindow::save()
{
    if (currentFilePath.isEmpty()) {
        saveAs();
        return;
    }
    
    saveFile(currentFilePath);
}

void MainWindow::saveAs()
{
    if (currentFolder.isEmpty()) {
        QMessageBox::information(this, tr("No Folder Open"), 
            tr("Please open a folder first (File → Open Folder)."));
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        currentFolder,
        tr("Markdown Files (*.md *.markdown);;All Files (*)")
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    if (!filePath.endsWith(".md") && !filePath.endsWith(".markdown")) {
        filePath += ".md";
    }
    
    if (saveFile(filePath)) {
        currentFilePath = filePath;
        QFileInfo fileInfo(filePath);
        setWindowTitle(tr("MkEd - %1").arg(fileInfo.fileName()));
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
    if (sender() == toggleBacklinksAction) {
        backlinksPanel->setVisible(toggleBacklinksAction->isChecked());
    } else {
        previewPanel->setVisible(togglePreviewAction->isChecked());
    }
}

void MainWindow::onWikiLinkClicked(const QString &linkTarget)
{
    if (currentFolder.isEmpty()) {
        statusBar()->showMessage(tr("No folder open"), 3000);
        return;
    }
    
    QString targetPath = linkParser->resolveLinkTarget(linkTarget, currentFolder);
    
    if (targetPath.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Link Not Found"),
            tr("The file '%1' does not exist.\n\nDo you want to create it?").arg(linkTarget),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            QString fileName = linkTarget;
            if (!fileName.endsWith(".md") && !fileName.endsWith(".markdown")) {
                fileName += ".md";
            }
            
            QString newFilePath = QDir(currentFolder).filePath(fileName);
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
    QString title = windowTitle();
    bool isModified = editor->isModified();
    
    if (isModified && !title.startsWith("*")) {
        setWindowTitle("*" + title);
    } else if (!isModified && title.startsWith("*")) {
        setWindowTitle(title.mid(1));
    }
}

void MainWindow::autoSave()
{
    if (currentFilePath.isEmpty() || !editor->isModified()) {
        return;
    }
    
    if (saveFile(currentFilePath)) {
        statusBar()->showMessage(tr("Auto-saved"), 2000);
    }
}

bool MainWindow::maybeSave()
{
    if (!editor->isModified()) {
        return true;
    }
    
    QFileInfo fileInfo(currentFilePath);
    QString fileName = fileInfo.fileName();
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
        if (currentFilePath.isEmpty()) {
            return saveAs(), !currentFilePath.isEmpty();
        }
        return saveFile(currentFilePath);
    } else if (reply == QMessageBox::Cancel) {
        return false;
    }
    
    return true;
}

bool MainWindow::saveFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Could not save file:\n%1").arg(filePath));
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << editor->toPlainText();
    file.close();
    
    editor->document()->setModified(false);
    
    QFileInfo fileInfo(filePath);
    statusBar()->showMessage(tr("Saved: %1").arg(fileInfo.fileName()), 3000);
    
    return true;
}

bool MainWindow::loadFile(const QString &filePath)
{
    statusBar()->showMessage(tr("Opening: %1").arg(filePath));
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Could not open file:\n%1").arg(filePath));
        return false;
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    editor->setPlainText(content);
    editor->document()->setModified(false);
    currentFilePath = filePath;
    
    QFileInfo fileInfo(filePath);
    setWindowTitle(tr("MkEd - %1").arg(fileInfo.fileName()));
    statusBar()->showMessage(tr("Loaded: %1").arg(fileInfo.fileName()), 3000);
    
    updateBacklinks();
    
    return true;
}

void MainWindow::find()
{
    bool ok;
    QString searchText = QInputDialog::getText(this, tr("Find"),
                                              tr("Find text:"), QLineEdit::Normal,
                                              "", &ok);
    
    if (!ok || searchText.isEmpty()) {
        return;
    }
    
    QTextCursor cursor = editor->textCursor();
    QTextDocument::FindFlags flags;
    
    QTextCursor foundCursor = editor->document()->find(searchText, cursor, flags);
    
    if (foundCursor.isNull()) {
        foundCursor = editor->document()->find(searchText, 0, flags);
    }
    
    if (!foundCursor.isNull()) {
        editor->setTextCursor(foundCursor);
        statusBar()->showMessage(tr("Found: %1").arg(searchText), 3000);
    } else {
        QMessageBox::information(this, tr("Find"),
            tr("Text not found: %1").arg(searchText));
    }
}

void MainWindow::findAndReplace()
{
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
    
    connect(findNextButton, &QPushButton::clicked, [this, findEdit]() {
        QString searchText = findEdit->text();
        if (searchText.isEmpty()) {
            return;
        }
        
        QTextCursor cursor = editor->textCursor();
        QTextDocument::FindFlags flags;
        
        QTextCursor foundCursor = editor->document()->find(searchText, cursor, flags);
        
        if (foundCursor.isNull()) {
            foundCursor = editor->document()->find(searchText, 0, flags);
        }
        
        if (!foundCursor.isNull()) {
            editor->setTextCursor(foundCursor);
        } else {
            QMessageBox::information(this, tr("Find"), tr("Text not found"));
        }
    });
    
    connect(replaceButton, &QPushButton::clicked, [this, findEdit, replaceEdit]() {
        QTextCursor cursor = editor->textCursor();
        if (cursor.hasSelection() && cursor.selectedText() == findEdit->text()) {
            cursor.insertText(replaceEdit->text());
            statusBar()->showMessage(tr("Replaced"), 2000);
        }
    });
    
    connect(replaceAllButton, &QPushButton::clicked, [this, findEdit, replaceEdit, &dialog]() {
        QString searchText = findEdit->text();
        QString replaceText = replaceEdit->text();
        
        if (searchText.isEmpty()) {
            return;
        }
        
        QTextCursor cursor = editor->document()->find(searchText);
        int count = 0;
        
        editor->textCursor().beginEditBlock();
        
        while (!cursor.isNull()) {
            cursor.insertText(replaceText);
            cursor = editor->document()->find(searchText, cursor);
            count++;
        }
        
        editor->textCursor().endEditBlock();
        
        QMessageBox::information(&dialog, tr("Replace All"),
            tr("Replaced %1 occurrence(s)").arg(count));
        
        statusBar()->showMessage(tr("Replaced %1 occurrence(s)").arg(count), 3000);
    });
    
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    dialog.exec();
}

void MainWindow::updatePreview()
{
    QString markdown = editor->toPlainText();
    preview->setMarkdownContent(markdown);
}

void MainWindow::setPreviewThemeLight()
{
    preview->setTheme("light");
    updatePreview();
}

void MainWindow::setPreviewThemeDark()
{
    preview->setTheme("dark");
    updatePreview();
}

void MainWindow::setPreviewThemeSepia()
{
    preview->setTheme("sepia");
    updatePreview();
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
        
        // Apply theme
        QString theme = dialog.getDefaultTheme();
        preview->setTheme(theme);
        updatePreview();
        
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
        // Move cursor to the specified line
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
        editor->setTextCursor(cursor);
        editor->centerCursor();
        
        statusBar()->showMessage(tr("Jumped to line %1").arg(lineNumber), 3000);
    }
}
