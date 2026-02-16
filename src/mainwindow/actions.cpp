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

static QAction* createAction(QObject* parent, const QIcon& icon, 
                             const QString& text, const QString& statusTip,
                             const QString& toolTip = QString()) {
    QAction* action = new QAction(icon, text, parent);
    action->setStatusTip(statusTip);
    action->setToolTip(toolTip.isEmpty() ? statusTip : toolTip);
    return action;
}

static QAction* createAction(QObject* parent, const QString& text,
                             const QString& statusTip,
                             const QString& toolTip = QString()) {
    QAction* action = new QAction(text, parent);
    action->setStatusTip(statusTip);
    action->setToolTip(toolTip.isEmpty() ? statusTip : toolTip);
    return action;
}

void MainWindow::createActions() {
    newAction = createAction(
        this, iconWithFallback("document-new", QStyle::SP_FileIcon),
        tr("&New"), tr("Create a new file"));
    newAction->setShortcuts(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    openFolderAction = createAction(
        this, iconWithFallback("folder-open", QStyle::SP_DirIcon),
        tr("Open &Folder..."), tr("Open an existing file"));
    openFolderAction->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    connect(openFolderAction, &QAction::triggered, this,
            &MainWindow::openFolder);

    saveAction = createAction(
        this, iconWithFallback("document-save", QStyle::SP_DialogSaveButton),
        tr("&Save"), tr("Save the document"));
    saveAction->setShortcuts(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);

    saveAsAction = createAction(
        this, iconWithFallback("document-save-as", QStyle::SP_DialogSaveButton),
        tr("Save &As..."), tr("Save the document under a new name"),
        tr("Save document with a new name"));
    saveAsAction->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);

    printAction = createAction(
        this, iconWithFallback("document-print", QStyle::SP_DialogSaveButton),
        tr("&Print..."), tr("Print the document preview"),
        tr("Print preview"));
    printAction->setShortcuts(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MainWindow::print);

    exportHtmlAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/export-html.svg"),
        tr("Export to &HTML..."), tr("Export the document to HTML format"),
        tr("Export to HTML"));
    connect(exportHtmlAction, &QAction::triggered, this,
            &MainWindow::exportToHtml);

    exportPdfAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/export-pdf.svg"),
        tr("Export to &PDF..."), tr("Export the document to PDF format"),
        tr("Export to PDF"));
    connect(exportPdfAction, &QAction::triggered, this,
            &MainWindow::exportToPdf);

    exportDocxAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/export-docx.svg"),
        tr("Export to &Word..."), 
        tr("Export the document to Microsoft Word format"),
        tr("Export to Word"));
    connect(exportDocxAction, &QAction::triggered, this,
            &MainWindow::exportToDocx);

    exportPlainTextAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/export-text.svg"),
        tr("Export to Plain &Text..."),
        tr("Export the document to plain text format"),
        tr("Export to plain text"));
    connect(exportPlainTextAction, &QAction::triggered, this,
            &MainWindow::exportToPlainText);

    insertImageAction = createAction(
        this, iconWithFallback("insert-image", QStyle::SP_FileDialogInfoView),
        tr("Insert &Image..."), tr("Insert an image into the document"),
        tr("Insert an image"));
    insertImageAction->setShortcut(QKeySequence(tr("Ctrl+Shift+I")));
    connect(insertImageAction, &QAction::triggered, this,
            &MainWindow::insertImage);

    insertFormulaAction = createAction(
        this, iconWithFallback("preferences-desktop-font",
                             QStyle::SP_FileDialogInfoView),
        tr("Insert &Formula..."), tr("Insert a LaTeX formula"));
    insertFormulaAction->setShortcut(QKeySequence(tr("Ctrl+Shift+M")));
    connect(insertFormulaAction, &QAction::triggered, this,
            &MainWindow::insertFormula);

    insertWikiLinkAction = createAction(
        this, iconWithFallback("insert-link", QStyle::SP_FileLinkIcon),
        tr("Insert &Wiki Link..."), tr("Insert a wiki-style link"));
    insertWikiLinkAction->setShortcut(QKeySequence(tr("Ctrl+K")));
    connect(insertWikiLinkAction, &QAction::triggered, this,
            &MainWindow::insertWikiLink);

    attachDocumentAction = createAction(
        this, iconWithFallback("edit-paste", QStyle::SP_DriveFDIcon),
        tr("Attach &Document..."), tr("Attach a document file"),
        tr("Attach a document"));
    attachDocumentAction->setShortcut(QKeySequence(tr("Ctrl+Shift+D")));
    connect(attachDocumentAction, &QAction::triggered, this,
            &MainWindow::attachDocument);

    insertHeaderAction = createAction(
        this, iconWithFallback("format-text-bold", QStyle::SP_DialogYesButton),
        tr("Insert &Header"), tr("Insert a header"));
    insertHeaderAction->setShortcut(QKeySequence(tr("Ctrl+1")));
    connect(insertHeaderAction, &QAction::triggered, this,
            &MainWindow::insertHeader);

    insertBoldAction = createAction(
        this, iconWithFallback("format-text-bold", QStyle::SP_DialogYesButton),
        tr("&Bold"), tr("Make text bold"));
    insertBoldAction->setShortcut(QKeySequence::Bold);
    connect(insertBoldAction, &QAction::triggered, this,
            &MainWindow::insertBold);

    insertItalicAction = createAction(
        this, iconWithFallback("format-text-italic", QStyle::SP_DialogApplyButton),
        tr("&Italic"), tr("Make text italic"));
    insertItalicAction->setShortcut(QKeySequence::Italic);
    connect(insertItalicAction, &QAction::triggered, this,
            &MainWindow::insertItalic);

    insertStrikethroughAction = createAction(
        this, tr("Strikethrough"), tr("Strike out selected text"));
    connect(insertStrikethroughAction, &QAction::triggered, this,
            &MainWindow::insertStrikethrough);

    insertCodeAction = createAction(
        this, iconWithFallback("text-x-generic", QStyle::SP_ComputerIcon),
        tr("Inline &Code"), tr("Insert inline code"));
    insertCodeAction->setShortcut(QKeySequence(tr("Ctrl+`")));
    connect(insertCodeAction, &QAction::triggered, this,
            &MainWindow::insertCode);

    insertCodeBlockAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/code-block.svg"),
        tr("Code &Block"), tr("Insert code block"));
    insertCodeBlockAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
    connect(insertCodeBlockAction, &QAction::triggered, this,
            &MainWindow::insertCodeBlock);

    insertListAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/list-bullet.svg"),
        tr("Bulleted &List"), tr("Insert bulleted list"));
    insertListAction->setShortcut(QKeySequence(tr("Ctrl+Shift+8")));
    connect(insertListAction, &QAction::triggered, this,
            &MainWindow::insertList);

    insertNumberedListAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/list-numbered.svg"),
        tr("&Numbered List"), tr("Insert numbered list"));
    insertNumberedListAction->setShortcut(QKeySequence(tr("Ctrl+Shift+7")));
    connect(insertNumberedListAction, &QAction::triggered, this,
            &MainWindow::insertNumberedList);

    insertBlockquoteAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/blockquote.svg"),
        tr("Block&quote"), tr("Insert blockquote"));
    insertBlockquoteAction->setShortcut(QKeySequence(tr("Ctrl+Shift+.")));
    connect(insertBlockquoteAction, &QAction::triggered, this,
            &MainWindow::insertBlockquote);

    insertHorizontalRuleAction = createAction(
        this, adaptiveSvgIcon(":/icons/icons/horizontal-rule.svg"),
        tr("Horizontal &Rule"), tr("Insert horizontal rule"));
    insertHorizontalRuleAction->setShortcut(QKeySequence(tr("Ctrl+Shift+-")));
    connect(insertHorizontalRuleAction, &QAction::triggered, this,
            &MainWindow::insertHorizontalRule);

    insertLinkAction = createAction(
        this, iconWithFallback("insert-link", QStyle::SP_FileLinkIcon),
        tr("Insert Lin&k..."), tr("Insert hyperlink"));
    insertLinkAction->setShortcut(QKeySequence(tr("Ctrl+L")));
    connect(insertLinkAction, &QAction::triggered, this,
            &MainWindow::insertLink);

    insertTableAction = createAction(
        this, iconWithFallback("insert-table", QStyle::SP_FileDialogListView),
        tr("Insert &Table"), tr("Insert table"));
    insertTableAction->setShortcut(QKeySequence(tr("Ctrl+Shift+T")));
    connect(insertTableAction, &QAction::triggered, this,
            &MainWindow::insertTable);

    insertTocAction = createAction(
        this, tr("Insert Table of &Contents"), 
        tr("Generate or update table of contents"));
    insertTocAction->setShortcut(QKeySequence(tr("Ctrl+Shift+G")));
    connect(insertTocAction, &QAction::triggered, this,
            &MainWindow::insertToc);

    insertDateAction = createAction(
        this, tr("Insert Date"), 
        tr("Insert current date with last used format"),
        tr("Insert date (Alt+D)"));
    insertDateAction->setShortcut(QKeySequence(tr("Alt+D")));
    connect(insertDateAction, &QAction::triggered, this,
            &MainWindow::insertDate);

    insertTimeAction = createAction(
        this, tr("Insert Time"),
        tr("Insert current time with last used format"),
        tr("Insert time (Alt+T)"));
    insertTimeAction->setShortcut(QKeySequence(tr("Alt+T")));
    connect(insertTimeAction, &QAction::triggered, this,
            &MainWindow::insertTime);

    // Navigation actions
    backAction = createAction(
        this, iconWithFallback("go-previous", QStyle::SP_FileDialogBack),
        tr("&Back"), tr("Go back to the previous file"),
        tr("Go back"));
    backAction->setShortcut(QKeySequence(tr("Alt+Left")));
    backAction->setEnabled(false);
    connect(backAction, &QAction::triggered, this, &MainWindow::navigateBack);

    forwardAction = createAction(
        this, iconWithFallback("go-next", QStyle::SP_ArrowRight),
        tr("&Forward"), tr("Go forward to the next file"),
        tr("Go forward"));
    forwardAction->setShortcut(QKeySequence(tr("Alt+Right")));
    forwardAction->setEnabled(false);
    connect(forwardAction, &QAction::triggered, this,
            &MainWindow::navigateForward);

    nextTabAction = createAction(
        this, tr("&Next Tab"), tr("Switch to the next tab"));
    nextTabAction->setShortcuts(
        {QKeySequence(tr("Ctrl+Tab")), QKeySequence(tr("Ctrl+PageDown"))});
    connect(nextTabAction, &QAction::triggered, this,
            &MainWindow::switchToNextTab);

    previousTabAction = createAction(
        this, tr("&Previous Tab"), tr("Switch to the previous tab"));
    previousTabAction->setShortcuts(
        {QKeySequence(tr("Ctrl+Shift+Tab")), QKeySequence(tr("Ctrl+PageUp"))});
    connect(previousTabAction, &QAction::triggered, this,
            &MainWindow::switchToPreviousTab);

    closeTabAction = createAction(
        this, tr("&Close Tab"), tr("Close current tab"));
    closeTabAction->setShortcut(QKeySequence(tr("Ctrl+W")));
    connect(closeTabAction, &QAction::triggered, this,
            &MainWindow::closeCurrentTab);

    closeAllTabsAction = createAction(
        this, tr("Close &All Tabs"), tr("Close all tabs"));
    closeAllTabsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+W")));
    connect(closeAllTabsAction, &QAction::triggered, this,
            &MainWindow::closeAllTabs);

    exitAction = createAction(this, tr("E&xit"), tr("Exit the application"));
    exitAction->setShortcuts(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    undoAction = createAction(
        this, iconWithFallback("edit-undo", QStyle::SP_ArrowBack),
        tr("&Undo"), tr("Undo the last operation"), tr("Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->undo();
    });

    redoAction = createAction(
        this, iconWithFallback("edit-redo", QStyle::SP_ArrowForward),
        tr("&Redo"), tr("Redo the last operation"), tr("Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->redo();
    });

    cutAction = createAction(
        this, iconWithFallback("edit-cut", QStyle::SP_DialogCloseButton),
        tr("Cu&t"), tr("Cut the current selection"), tr("Cut"));
    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setEnabled(false);
    connect(cutAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->cut();
    });

    copyAction = createAction(
        this, iconWithFallback("edit-copy", QStyle::SP_DialogOkButton),
        tr("&Copy"), tr("Copy the current selection"), tr("Copy"));
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setEnabled(false);
    connect(copyAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->copy();
    });

    pasteAction = createAction(
        this, iconWithFallback("edit-paste", QStyle::SP_DialogApplyButton),
        tr("&Paste"), tr("Paste the clipboard contents"), tr("Paste"));
    pasteAction->setShortcuts(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, [this]() {
        TabEditor* tab = currentTabEditor();
        if (tab) tab->editor()->paste();
    });

    findAction = createAction(
        this, iconWithFallback("edit-find", QStyle::SP_FileDialogDetailedView),
        tr("&Find..."), tr("Find text"), tr("Find"));
    findAction->setShortcuts(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::find);

    findReplaceAction = createAction(
        this, iconWithFallback("edit-find-replace",
                             QStyle::SP_FileDialogDetailedView),
        tr("Find and &Replace..."), tr("Find and replace text"),
        tr("Find and replace"));
    findReplaceAction->setShortcut(QKeySequence(tr("Ctrl+H")));
    connect(findReplaceAction, &QAction::triggered, this,
            &MainWindow::findAndReplace);

    searchInFilesAction = createAction(
        this, iconWithFallback("system-search", QStyle::SP_FileDialogContentsView),
        tr("Search in &Files..."), tr("Search across all files"),
        tr("Search in all files"));
    searchInFilesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+F")));
    connect(searchInFilesAction, &QAction::triggered, this,
            &MainWindow::searchInFiles);

    aiAssistAction = createAction(
        this, iconWithFallback("tools-wizard", QStyle::SP_MessageBoxInformation),
        tr("AI &Assist..."), tr("AI-powered writing assistance"),
        tr("AI Assist"));
    aiAssistAction->setShortcut(QKeySequence(tr("Alt+A")));
    connect(aiAssistAction, &QAction::triggered, this,
            &MainWindow::openAIAssist);

    quickOpenAction = createAction(
        this, iconWithFallback("document-open-recent", QStyle::SP_DialogOpenButton),
        tr("&Quick Open..."), tr("Quickly open a file"),
        tr("Quick open"));
    quickOpenAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    connect(quickOpenAction, &QAction::triggered, this, &MainWindow::quickOpen);

    breakLinesAction = createAction(
        this, tr("Break Lines"),
        tr("Break lines according to preferences (Ctrl+Shift+B)"));
    breakLinesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+B")));
    connect(breakLinesAction, &QAction::triggered, this,
            &MainWindow::breakLines);

    joinLinesAction = createAction(
        this, tr("Join Lines"),
        tr("Join lines back into paragraphs (Ctrl+Shift+J)"));
    joinLinesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+J")));
    connect(joinLinesAction, &QAction::triggered, this, &MainWindow::joinLines);

    toggleSidebarAction = createAction(
        this, iconWithFallback("view-sidetree", QStyle::SP_DirIcon),
        tr("&Sidebar"), tr("Toggle sidebar (Files/Outline)"),
        tr("Toggle sidebar"));
    toggleSidebarAction->setCheckable(true);
    toggleSidebarAction->setChecked(true);
    toggleSidebarAction->setShortcut(QKeySequence(tr("Ctrl+B")));
    connect(toggleSidebarAction, &QAction::triggered, this,
            &MainWindow::toggleSidebar);

    togglePreviewAction = createAction(
        this, iconWithFallback("document-preview", QStyle::SP_FileDialogContentsView),
        tr("&Preview"), tr("Toggle preview panel"));
    togglePreviewAction->setCheckable(true);
    togglePreviewAction->setChecked(true);
    togglePreviewAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    connect(togglePreviewAction, &QAction::triggered, this,
            &MainWindow::togglePreview);

    cycleViewModeAction = createAction(
        this, iconWithFallback("view-split-left-right",
                             QStyle::SP_FileDialogContentsView),
        tr("Cycle View Mode (Next: Editor Only)"),
        tr("Cycle between Editor+Preview / Editor Only / Preview Only"),
        tr("Cycle view mode (Ctrl+Shift+P)"));
    cycleViewModeAction->setShortcut(QKeySequence(tr("Ctrl+Shift+P")));
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

    settingsAction = createAction(
        this, tr("&Settings..."), tr("Open settings dialog"),
        tr("Configure application settings"));
    settingsAction->setShortcut(QKeySequence(tr("Ctrl+,")));
    connect(settingsAction, &QAction::triggered, this,
            &MainWindow::openSettings);

    keyboardShortcutsAction = createAction(
        this, tr("&Keyboard Shortcuts"),
        tr("Show keyboard shortcuts reference"),
        tr("Display all available keyboard shortcuts"));
    keyboardShortcutsAction->setShortcut(QKeySequence(tr("F1")));
    connect(keyboardShortcutsAction, &QAction::triggered, this,
            &MainWindow::showKeyboardShortcuts);

    userGuideAction = createAction(
        this, tr("User &Guide"), tr("Open the user guide"),
        tr("View the TreeMk user guide"));
    userGuideAction->setShortcut(QKeySequence(tr("Ctrl+F1")));
    connect(userGuideAction, &QAction::triggered, this,
            &MainWindow::showUserGuide);

    aboutAction = createAction(this, tr("&About"), 
        tr("Show the application's About box"),
        tr("About the application"));
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
