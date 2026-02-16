#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QRegularExpression>
#include <QStyle>
#include <QSvgRenderer>
#include <QTextCursor>

#include "defs.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "shortcutmanager.h"
#include "tabeditor.h"

static QIcon iconWithFallback(const QString& themeName,
                              QStyle::StandardPixmap fallback) {
    QIcon icon = QIcon::fromTheme(themeName);
    if (icon.isNull()) {
        icon = qApp->style()->standardIcon(fallback);
    }
    return icon;
}

static QIcon adaptiveSvgIcon(const QString& resourcePath) {
    QIcon icon;
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return icon;
    }
    QString svgContent = QString::fromUtf8(file.readAll());
    file.close();
    QPalette palette = qApp->palette();
    QColor normalColor = palette.color(QPalette::WindowText);
    QColor disabledColor =
        palette.color(QPalette::Disabled, QPalette::WindowText);
    auto createPixmap = [&svgContent](const QColor& color,
                                      const QSize& size) -> QPixmap {
        QString coloredSvg = svgContent;
        coloredSvg.replace("currentColor", color.name());
        QSvgRenderer renderer(coloredSvg.toUtf8());
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        return pixmap;
    };
    QList<QSize> sizes = {{16, 16}, {24, 24}, {32, 32}, {48, 48}};
    for (const QSize& size : sizes) {
        icon.addPixmap(createPixmap(normalColor, size), QIcon::Normal);
        icon.addPixmap(createPixmap(disabledColor, size), QIcon::Disabled);
    }
    return icon;
}

void MainWindow::createActions() {
    const char* new_file_label = "Create a new file";
    newAction =
        new QAction(iconWithFallback("document-new", QStyle::SP_FileIcon),
                    tr("&New"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr(new_file_label));
    newAction->setToolTip(tr(new_file_label));
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    const char* open_file_label = "Open an existing file";
    openFolderAction =
        new QAction(iconWithFallback("folder-open", QStyle::SP_DirIcon),
                    tr("Open &Folder..."), this);
    openFolderAction->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    openFolderAction->setStatusTip(tr(open_file_label));
    openFolderAction->setToolTip(tr(open_file_label));
    connect(openFolderAction, &QAction::triggered, this,
            &MainWindow::openFolder);

    saveAction = new QAction(
        iconWithFallback("document-save", QStyle::SP_DialogSaveButton),
        tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document"));
    saveAction->setToolTip(tr("Save the document"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);

    saveAsAction = new QAction(
        iconWithFallback("document-save-as", QStyle::SP_DialogSaveButton),
        tr("Save &As..."), this);
    saveAsAction->setShortcuts(QKeySequence::SaveAs);
    saveAsAction->setStatusTip(tr("Save the document under a new name"));
    saveAsAction->setToolTip(tr("Save document with a new name"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);

    printAction = new QAction(
        iconWithFallback("document-print", QStyle::SP_DialogSaveButton),
        tr("&Print..."), this);
    printAction->setShortcuts(QKeySequence::Print);
    printAction->setStatusTip(tr("Print the document preview"));
    printAction->setToolTip(tr("Print preview"));
    connect(printAction, &QAction::triggered, this, &MainWindow::print);

    exportHtmlAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/export-html.svg"),
                    tr("Export to &HTML..."), this);
    exportHtmlAction->setStatusTip(tr("Export the document to HTML format"));
    exportHtmlAction->setToolTip(tr("Export to HTML"));
    connect(exportHtmlAction, &QAction::triggered, this,
            &MainWindow::exportToHtml);

    exportPdfAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/export-pdf.svg"),
                    tr("Export to &PDF..."), this);
    exportPdfAction->setStatusTip(tr("Export the document to PDF format"));
    exportPdfAction->setToolTip(tr("Export to PDF"));
    connect(exportPdfAction, &QAction::triggered, this,
            &MainWindow::exportToPdf);

    exportDocxAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/export-docx.svg"),
                    tr("Export to &Word..."), this);
    exportDocxAction->setStatusTip(
        tr("Export the document to Microsoft Word format"));
    exportDocxAction->setToolTip(tr("Export to Word"));
    connect(exportDocxAction, &QAction::triggered, this,
            &MainWindow::exportToDocx);

    exportPlainTextAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/export-text.svg"),
                    tr("Export to Plain &Text..."), this);
    exportPlainTextAction->setStatusTip(
        tr("Export the document to plain text format"));
    exportPlainTextAction->setToolTip(tr("Export to plain text"));
    connect(exportPlainTextAction, &QAction::triggered, this,
            &MainWindow::exportToPlainText);

    insertImageAction = new QAction(
        iconWithFallback("insert-image", QStyle::SP_FileDialogInfoView),
        tr("Insert &Image..."), this);
    insertImageAction->setShortcut(QKeySequence(tr("Ctrl+Shift+I")));
    insertImageAction->setStatusTip(tr("Insert an image into the document"));
    insertImageAction->setToolTip(tr("Insert an image"));
    connect(insertImageAction, &QAction::triggered, this,
            &MainWindow::insertImage);

    insertFormulaAction =
        new QAction(iconWithFallback("preferences-desktop-font",
                                     QStyle::SP_FileDialogInfoView),
                    tr("Insert &Formula..."), this);
    insertFormulaAction->setShortcut(QKeySequence(tr("Ctrl+Shift+M")));
    insertFormulaAction->setStatusTip(tr("Insert a LaTeX formula"));
    insertFormulaAction->setToolTip(tr("Insert a LaTeX formula"));
    connect(insertFormulaAction, &QAction::triggered, this,
            &MainWindow::insertFormula);

    insertWikiLinkAction =
        new QAction(iconWithFallback("insert-link", QStyle::SP_FileLinkIcon),
                    tr("Insert &Wiki Link..."), this);
    insertWikiLinkAction->setShortcut(QKeySequence(tr("Ctrl+K")));
    insertWikiLinkAction->setStatusTip(tr("Insert a wiki-style link"));
    insertWikiLinkAction->setToolTip(tr("Insert a wiki-style link"));
    connect(insertWikiLinkAction, &QAction::triggered, this,
            &MainWindow::insertWikiLink);

    attachDocumentAction =
        new QAction(iconWithFallback("edit-paste", QStyle::SP_DriveFDIcon),
                    tr("Attach &Document..."), this);
    attachDocumentAction->setShortcut(QKeySequence(tr("Ctrl+Shift+D")));
    attachDocumentAction->setStatusTip(tr("Attach a document file"));
    attachDocumentAction->setToolTip(tr("Attach a document"));
    connect(attachDocumentAction, &QAction::triggered, this,
            &MainWindow::attachDocument);

    insertHeaderAction = new QAction(
        iconWithFallback("format-text-bold", QStyle::SP_DialogYesButton),
        tr("Insert &Header"), this);
    insertHeaderAction->setShortcut(QKeySequence(tr("Ctrl+1")));
    insertHeaderAction->setStatusTip(tr("Insert a header"));
    insertHeaderAction->setToolTip(tr("Insert a header"));
    connect(insertHeaderAction, &QAction::triggered, this,
            &MainWindow::insertHeader);

    insertBoldAction = new QAction(
        iconWithFallback("format-text-bold", QStyle::SP_DialogYesButton),
        tr("&Bold"), this);
    insertBoldAction->setShortcut(QKeySequence::Bold);
    insertBoldAction->setStatusTip(tr("Make text bold"));
    insertBoldAction->setToolTip(tr("Make text bold"));
    connect(insertBoldAction, &QAction::triggered, this,
            &MainWindow::insertBold);

    insertItalicAction = new QAction(
        iconWithFallback("format-text-italic", QStyle::SP_DialogApplyButton),
        tr("&Italic"), this);
    insertItalicAction->setShortcut(QKeySequence::Italic);
    insertItalicAction->setStatusTip(tr("Make text italic"));
    insertItalicAction->setToolTip(tr("Make text italic"));
    connect(insertItalicAction, &QAction::triggered, this,
            &MainWindow::insertItalic);

    insertStrikethroughAction = new QAction(tr("Strikethrough"), this);
    insertStrikethroughAction->setStatusTip(tr("Strike out selected text"));
    connect(insertStrikethroughAction, &QAction::triggered, this,
            &MainWindow::insertStrikethrough);

    insertCodeAction =
        new QAction(iconWithFallback("text-x-generic", QStyle::SP_ComputerIcon),
                    tr("Inline &Code"), this);
    insertCodeAction->setShortcut(QKeySequence(tr("Ctrl+`")));
    insertCodeAction->setStatusTip(tr("Insert inline code"));
    insertCodeAction->setToolTip(tr("Insert inline code"));
    connect(insertCodeAction, &QAction::triggered, this,
            &MainWindow::insertCode);

    insertCodeBlockAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/code-block.svg"),
                    tr("Code &Block"), this);
    insertCodeBlockAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
    insertCodeBlockAction->setStatusTip(tr("Insert code block"));
    insertCodeBlockAction->setToolTip(tr("Insert code block"));
    connect(insertCodeBlockAction, &QAction::triggered, this,
            &MainWindow::insertCodeBlock);

    insertListAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/list-bullet.svg"),
                    tr("Bulleted &List"), this);
    insertListAction->setShortcut(QKeySequence(tr("Ctrl+Shift+8")));
    insertListAction->setStatusTip(tr("Insert bulleted list"));
    insertListAction->setToolTip(tr("Insert bulleted list"));
    connect(insertListAction, &QAction::triggered, this,
            &MainWindow::insertList);

    insertNumberedListAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/list-numbered.svg"),
                    tr("&Numbered List"), this);
    insertNumberedListAction->setShortcut(QKeySequence(tr("Ctrl+Shift+7")));
    insertNumberedListAction->setStatusTip(tr("Insert numbered list"));
    insertNumberedListAction->setToolTip(tr("Insert numbered list"));
    connect(insertNumberedListAction, &QAction::triggered, this,
            &MainWindow::insertNumberedList);

    insertBlockquoteAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/blockquote.svg"),
                    tr("Block&quote"), this);
    insertBlockquoteAction->setShortcut(QKeySequence(tr("Ctrl+Shift+.")));
    insertBlockquoteAction->setStatusTip(tr("Insert blockquote"));
    insertBlockquoteAction->setToolTip(tr("Insert blockquote"));
    connect(insertBlockquoteAction, &QAction::triggered, this,
            &MainWindow::insertBlockquote);

    insertHorizontalRuleAction =
        new QAction(adaptiveSvgIcon(":/icons/icons/horizontal-rule.svg"),
                    tr("Horizontal &Rule"), this);
    insertHorizontalRuleAction->setShortcut(QKeySequence(tr("Ctrl+Shift+-")));
    insertHorizontalRuleAction->setStatusTip(tr("Insert horizontal rule"));
    insertHorizontalRuleAction->setToolTip(tr("Insert horizontal rule"));
    connect(insertHorizontalRuleAction, &QAction::triggered, this,
            &MainWindow::insertHorizontalRule);

    insertLinkAction =
        new QAction(iconWithFallback("insert-link", QStyle::SP_FileLinkIcon),
                    tr("Insert Lin&k..."), this);
    insertLinkAction->setShortcut(QKeySequence(tr("Ctrl+L")));
    insertLinkAction->setStatusTip(tr("Insert hyperlink"));
    insertLinkAction->setToolTip(tr("Insert hyperlink"));
    connect(insertLinkAction, &QAction::triggered, this,
            &MainWindow::insertLink);

    insertTableAction = new QAction(
        iconWithFallback("insert-table", QStyle::SP_FileDialogListView),
        tr("Insert &Table"), this);
    insertTableAction->setShortcut(QKeySequence(tr("Ctrl+Shift+T")));
    insertTableAction->setStatusTip(tr("Insert table"));
    insertTableAction->setToolTip(tr("Insert table"));
    connect(insertTableAction, &QAction::triggered, this,
            &MainWindow::insertTable);

    insertDateAction = new QAction(tr("Insert Date"), this);
    insertDateAction->setShortcut(QKeySequence(tr("Alt+D")));
    insertDateAction->setStatusTip(
        tr("Insert current date with last used format"));
    insertDateAction->setToolTip(tr("Insert date (Alt+D)"));
    connect(insertDateAction, &QAction::triggered, this,
            &MainWindow::insertDate);

    insertTimeAction = new QAction(tr("Insert Time"), this);
    insertTimeAction->setShortcut(QKeySequence(tr("Alt+T")));
    insertTimeAction->setStatusTip(
        tr("Insert current time with last used format"));
    insertTimeAction->setToolTip(tr("Insert time (Alt+T)"));
    connect(insertTimeAction, &QAction::triggered, this,
            &MainWindow::insertTime);

    // Navigation actions
    backAction =
        new QAction(iconWithFallback("go-previous", QStyle::SP_FileDialogBack),
                    tr("&Back"), this);
    backAction->setShortcut(QKeySequence(tr("Alt+Left")));
    backAction->setStatusTip(tr("Go back to the previous file"));
    backAction->setToolTip(tr("Go back"));
    backAction->setEnabled(false);
    connect(backAction, &QAction::triggered, this, &MainWindow::navigateBack);

    forwardAction =
        new QAction(iconWithFallback("go-next", QStyle::SP_ArrowRight),
                    tr("&Forward"), this);
    forwardAction->setShortcut(QKeySequence(tr("Alt+Right")));
    forwardAction->setStatusTip(tr("Go forward to the next file"));
    forwardAction->setToolTip(tr("Go forward"));
    forwardAction->setEnabled(false);
    connect(forwardAction, &QAction::triggered, this,
            &MainWindow::navigateForward);

    const char* next_tab_label = "Switch to the next tab";
    nextTabAction = new QAction(tr("&Next Tab"), this);
    nextTabAction->setShortcuts(
        {QKeySequence(tr("Ctrl+Tab")), QKeySequence(tr("Ctrl+PageDown"))});
    nextTabAction->setStatusTip(tr(next_tab_label));
    nextTabAction->setToolTip(tr(next_tab_label));
    connect(nextTabAction, &QAction::triggered, this,
            &MainWindow::switchToNextTab);

    const char* previous_tab_label = "Switch to the previous tab";
    previousTabAction = new QAction(tr("&Previous Tab"), this);
    previousTabAction->setShortcuts(
        {QKeySequence(tr("Ctrl+Shift+Tab")), QKeySequence(tr("Ctrl+PageUp"))});
    previousTabAction->setStatusTip(tr(previous_tab_label));
    previousTabAction->setToolTip(tr(previous_tab_label));
    connect(previousTabAction, &QAction::triggered, this,
            &MainWindow::switchToPreviousTab);

    closeTabAction = new QAction(tr("&Close Tab"), this);
    closeTabAction->setShortcut(QKeySequence(tr("Ctrl+W")));
    closeTabAction->setStatusTip(tr("Close current tab"));
    connect(closeTabAction, &QAction::triggered, this,
            &MainWindow::closeCurrentTab);

    closeAllTabsAction = new QAction(tr("Close &All Tabs"), this);
    closeAllTabsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+W")));
    closeAllTabsAction->setStatusTip(tr("Close all tabs"));
    connect(closeAllTabsAction, &QAction::triggered, this,
            &MainWindow::closeAllTabs);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    undoAction = new QAction(
        iconWithFallback("edit-undo", QStyle::SP_ArrowBack), tr("&Undo"), this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setStatusTip(tr("Undo the last operation"));
    undoAction->setToolTip(tr("Undo"));
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->undo();
    });

    redoAction =
        new QAction(iconWithFallback("edit-redo", QStyle::SP_ArrowForward),
                    tr("&Redo"), this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setStatusTip(tr("Redo the last operation"));
    redoAction->setToolTip(tr("Redo"));
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->redo();
    });

    cutAction =
        new QAction(iconWithFallback("edit-cut", QStyle::SP_DialogCloseButton),
                    tr("Cu&t"), this);
    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setStatusTip(tr("Cut the current selection"));
    cutAction->setToolTip(tr("Cut"));
    cutAction->setEnabled(false);
    connect(cutAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->cut();
    });

    copyAction =
        new QAction(iconWithFallback("edit-copy", QStyle::SP_DialogOkButton),
                    tr("&Copy"), this);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy the current selection"));
    copyAction->setToolTip(tr("Copy"));
    copyAction->setEnabled(false);
    connect(copyAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->copy();
    });

    pasteAction = new QAction(
        iconWithFallback("edit-paste", QStyle::SP_DialogApplyButton),
        tr("&Paste"), this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste the clipboard contents"));
    pasteAction->setToolTip(tr("Paste"));
    connect(pasteAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->paste();
    });

    findAction = new QAction(
        iconWithFallback("edit-find", QStyle::SP_FileDialogDetailedView),
        tr("&Find..."), this);
    findAction->setShortcuts(QKeySequence::Find);
    findAction->setStatusTip(tr("Find text"));
    findAction->setToolTip(tr("Find"));
    connect(findAction, &QAction::triggered, this, &MainWindow::find);

    findReplaceAction =
        new QAction(iconWithFallback("edit-find-replace",
                                     QStyle::SP_FileDialogDetailedView),
                    tr("Find and &Replace..."), this);
    findReplaceAction->setShortcut(QKeySequence(tr("Ctrl+H")));
    findReplaceAction->setStatusTip(tr("Find and replace text"));
    findReplaceAction->setToolTip(tr("Find and replace"));
    connect(findReplaceAction, &QAction::triggered, this,
            &MainWindow::findAndReplace);

    searchInFilesAction = new QAction(
        iconWithFallback("system-search", QStyle::SP_FileDialogContentsView),
        tr("Search in &Files..."), this);
    searchInFilesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+F")));
    searchInFilesAction->setStatusTip(tr("Search across all files"));
    searchInFilesAction->setToolTip(tr("Search in all files"));
    connect(searchInFilesAction, &QAction::triggered, this,
            &MainWindow::searchInFiles);

    aiAssistAction = new QAction(
        iconWithFallback("tools-wizard", QStyle::SP_MessageBoxInformation),
        tr("AI &Assist..."), this);
    aiAssistAction->setShortcut(QKeySequence(tr("Alt+A")));
    aiAssistAction->setStatusTip(tr("AI-powered writing assistance"));
    aiAssistAction->setToolTip(tr("AI Assist"));
    connect(aiAssistAction, &QAction::triggered, this,
            &MainWindow::openAIAssist);

    quickOpenAction = new QAction(
        iconWithFallback("document-open-recent", QStyle::SP_DialogOpenButton),
        tr("&Quick Open..."), this);
    quickOpenAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    quickOpenAction->setStatusTip(tr("Quickly open a file"));
    quickOpenAction->setToolTip(tr("Quick open"));
    connect(quickOpenAction, &QAction::triggered, this, &MainWindow::quickOpen);

    breakLinesAction = new QAction(tr("Break Lines"), this);
    breakLinesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+B")));
    breakLinesAction->setStatusTip(
        tr("Break lines according to preferences (Ctrl+Shift+B)"));
    connect(breakLinesAction, &QAction::triggered, this,
            &MainWindow::breakLines);

    joinLinesAction = new QAction(tr("Join Lines"), this);
    joinLinesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+J")));
    joinLinesAction->setStatusTip(
        tr("Join lines back into paragraphs (Ctrl+Shift+J)"));
    connect(joinLinesAction, &QAction::triggered, this, &MainWindow::joinLines);

    toggleSidebarAction =
        new QAction(iconWithFallback("view-sidetree", QStyle::SP_DirIcon),
                    tr("&Sidebar"), this);
    toggleSidebarAction->setCheckable(true);
    toggleSidebarAction->setChecked(true);
    toggleSidebarAction->setShortcut(QKeySequence(tr("Ctrl+B")));
    toggleSidebarAction->setStatusTip(tr("Toggle sidebar (Files/Outline)"));
    toggleSidebarAction->setToolTip(tr("Toggle sidebar"));
    connect(toggleSidebarAction, &QAction::triggered, this,
            &MainWindow::toggleSidebar);

    togglePreviewAction = new QAction(
        iconWithFallback("document-preview", QStyle::SP_FileDialogContentsView),
        tr("&Preview"), this);
    togglePreviewAction->setCheckable(true);
    togglePreviewAction->setChecked(true);
    togglePreviewAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    togglePreviewAction->setStatusTip(tr("Toggle preview panel"));
    togglePreviewAction->setToolTip(tr("Toggle preview panel"));
    connect(togglePreviewAction, &QAction::triggered, this,
            &MainWindow::togglePreview);

    cycleViewModeAction =
        new QAction(iconWithFallback("view-split-left-right",
                                     QStyle::SP_FileDialogContentsView),
                    tr("Cycle View Mode (Next: Editor Only)"), this);
    cycleViewModeAction->setShortcut(QKeySequence(tr("Ctrl+Shift+P")));
    cycleViewModeAction->setStatusTip(
        tr("Cycle between Editor+Preview / Editor Only / Preview Only"));
    cycleViewModeAction->setToolTip(tr("Cycle view mode (Ctrl+Shift+P)"));
    connect(cycleViewModeAction, &QAction::triggered, this,
            &MainWindow::cycleViewMode);

    /* TODO: to be removed. theme is general for all the app.
    previewThemeLightAction = new QAction(tr("Light Theme"), this);
    previewThemeLightAction->setCheckable(true);
    previewThemeLightAction->setChecked(true);
    connect(previewThemeLightAction, &QAction::triggered, this,
            &MainWindow::setPreviewThemeLight);

    previewThemeDarkAction = new QAction(tr("Dark Theme"), this);
    previewThemeDarkAction->setCheckable(true);
    connect(previewThemeDarkAction, &QAction::triggered, this,
            &MainWindow::setPreviewThemeDark);

    previewThemeSepiaAction = new QAction(tr("Sepia Theme"), this);
    previewThemeSepiaAction->setCheckable(true);
    connect(previewThemeSepiaAction, &QAction::triggered, this,
            &MainWindow::setPreviewThemeSepia);

    QActionGroup* themeGroup = new QActionGroup(this);
    themeGroup->addAction(previewThemeLightAction);
    themeGroup->addAction(previewThemeDarkAction);
    themeGroup->addAction(previewThemeSepiaAction);
    */

    settingsAction = new QAction(tr("&Settings..."), this);
    settingsAction->setShortcut(QKeySequence(tr("Ctrl+,")));
    settingsAction->setStatusTip(tr("Open settings dialog"));
    settingsAction->setToolTip(tr("Configure application settings"));
    connect(settingsAction, &QAction::triggered, this,
            &MainWindow::openSettings);

    keyboardShortcutsAction = new QAction(tr("&Keyboard Shortcuts"), this);
    keyboardShortcutsAction->setShortcut(QKeySequence(tr("F1")));
    keyboardShortcutsAction->setStatusTip(
        tr("Show keyboard shortcuts reference"));
    keyboardShortcutsAction->setToolTip(
        tr("Display all available keyboard shortcuts"));
    connect(keyboardShortcutsAction, &QAction::triggered, this,
            &MainWindow::showKeyboardShortcuts);

    userGuideAction = new QAction(tr("User &Guide"), this);
    userGuideAction->setShortcut(QKeySequence(tr("Ctrl+F1")));
    userGuideAction->setStatusTip(tr("Open the user guide"));
    userGuideAction->setToolTip(tr("View the TreeMk user guide"));
    connect(userGuideAction, &QAction::triggered, this,
            &MainWindow::showUserGuide);

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    aboutAction->setToolTip(tr("About the application"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::breakLines() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    QSettings settings(APP_LABEL, APP_LABEL);
    bool enabled = settings.value("editor/lineBreakEnabled", false).toBool();
    if (!enabled) return;

    int columns = settings.value("editor/lineBreakColumns", 80).toInt();

    QTextCursor cursor = tab->editor()->textCursor();
    bool hasSelection = cursor.hasSelection();
    int start = hasSelection ? cursor.selectionStart() : 0;
    int end = hasSelection ? cursor.selectionEnd()
                           : tab->editor()->document()->characterCount() - 1;

    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    QString text = cursor.selectedText();
    text.replace(QChar(0x2029), '\n');

    QStringList lines = text.split('\n');

    QString newText;

    for (const QString& line : lines) {
        QString current = line;

        while (current.length() > columns) {
            int spacePos = current.lastIndexOf(' ', columns - 1);

            if (spacePos > 0) {
                newText += current.left(spacePos) + "\n";

                current = current.mid(spacePos + 1).trimmed();

            } else {
                // No space found, break at column limit

                newText += current.left(columns) + "\n";

                current = current.mid(columns);
            }
        }

        newText += current + "\n";
    }

    newText.chop(1);  // remove last \n

    cursor.insertText(newText);
}

void MainWindow::joinLines() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    QTextCursor cursor = tab->editor()->textCursor();
    bool hasSelection = cursor.hasSelection();
    QString text;

    if (hasSelection) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        text = cursor.selectedText();
        text.replace(QChar(0x2029), '\n');
        // For selected text, join all lines with spaces
        text.replace("\n", " ");
    } else {
        text = tab->editor()->toPlainText();
        // For whole document, join lines within paragraphs
        text.replace(QRegularExpression("([^\n])\n([^\n])"), "\\1 \\2");
    }

    if (hasSelection) {
        cursor.insertText(text);
    } else {
        tab->editor()->setPlainText(text);
    }
}
