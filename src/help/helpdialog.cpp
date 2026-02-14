#include "helpdialog.h"

#include <QFile>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextStream>

#include "markdownpreview.h"
#include "searchengine.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::HelpDialog), historyIndex(-1) {
    ui->setupUi(this);

    // Make it a normal window that can go behind the main window
    setWindowFlags(Qt::Window);

    searchEngine = new SearchEngine();
    searchEngine->setContextSize(80);
    searchEngine->setMaxResultsPerFile(5);

    initializeTopics();
    setupContent();

    // Connect signals
    connect(ui->homeButton, &QPushButton::clicked, this,
            &HelpDialog::onHomeClicked);
    connect(ui->backButton, &QPushButton::clicked, this,
            &HelpDialog::onBackClicked);
    connect(ui->forwardButton, &QPushButton::clicked, this,
            &HelpDialog::onForwardClicked);
    connect(ui->tableOfContents, &QListWidget::currentRowChanged, this,
            &HelpDialog::onTableOfContentsItemClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this,
            &HelpDialog::onSearchTextChanged);
    connect(ui->searchResultsList, &QListWidget::itemClicked, this,
            &HelpDialog::onSearchResultClicked);
    connect(ui->printButton, &QPushButton::clicked, this,
            &HelpDialog::onPrintClicked);

    // Load the index page by default
    showTopic("index");
}

HelpDialog::~HelpDialog() {
    delete searchEngine;
    delete ui;
}

void HelpDialog::initializeTopics() {
    // Define help topics with metadata
    // These should match the files in resources/help/
    topics = {
        {"index", tr("User Guide"), tr("Main help page and overview")},
        {"getting-started", tr("Getting Started"),
         tr("Quick introduction to TreeMk")},
        {"editor", tr("Editor Features"),
         tr("Editing capabilities and shortcuts")},
        {"navigation", tr("Navigation"), tr("Wiki links, outline, and search")},
        {"preview", tr("Preview Features"),
         tr("Rendering, formulas, and themes")},
        {"formulas", tr("Math Formulas"), tr("LaTeX mathematical notation")},
        {"code-blocks", tr("Code Blocks"), tr("Syntax highlighting for code")},
        {"diagrams", tr("Mermaid Diagrams"),
         tr("Creating diagrams with Mermaid")},
        {"ai-assistant", tr("AI Assistant"), tr("AI-powered writing help")},
        {"keyboard-shortcuts", tr("Keyboard Shortcuts"),
         tr("Complete shortcuts reference")}};
}

void HelpDialog::setupContent() {
    // Create and add the MarkdownPreview widget to the container
    contentView = new MarkdownPreview(this);
    ui->contentViewContainer->layout()->addWidget(contentView);

    // Set base path for help resources (for resolving images)
    contentView->setBasePath("qrc:/help/help/");

    // Connect link clicks for navigation
    connect(contentView, &MarkdownPreview::wikiLinkClicked, this,
            &HelpDialog::onLinkClicked);
    connect(contentView, &MarkdownPreview::markdownLinkClicked, this,
            &HelpDialog::onLinkClicked);

    // Populate table of contents
    for (const HelpTopic& topic : topics) {
        QListWidgetItem* item = new QListWidgetItem(topic.displayName);
        item->setToolTip(topic.description);
        item->setData(Qt::UserRole, topic.fileName);
        ui->tableOfContents->addItem(item);
    }

    // Set splitter stretch factors
    ui->splitter->setStretchFactor(1, 1);  // Content view gets more space
}

void HelpDialog::loadHelpContent(const QString& fileName) {
    QString resourcePath = QString(":/help/help/%1.md").arg(fileName);
    QFile file(resourcePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString errorMsg =
            tr("Could not load help topic: %1\n\nResource path: %2")
                .arg(fileName, resourcePath);
        contentView->setMarkdownContent(QString("# Error\n\n%1").arg(errorMsg));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    // Set the markdown content
    contentView->setMarkdownContent(content);

    // Update navigation
    addToHistory(fileName);
    updateNavigationButtons();
    updateBreadcrumb();
}

void HelpDialog::showTopic(const QString& topic) {
    // Find the topic in the list
    for (int i = 0; i < topics.size(); ++i) {
        if (topics[i].fileName == topic) {
            ui->tableOfContents->setCurrentRow(i);
            loadHelpContent(topic);
            return;
        }
    }

    // Topic not found, load anyway (might be a sub-page)
    loadHelpContent(topic);
}

void HelpDialog::onTableOfContentsItemClicked(int index) {
    if (index < 0 || index >= topics.size()) {
        return;
    }

    // Clear search when manually selecting a topic
    ui->searchLineEdit->clear();

    loadHelpContent(topics[index].fileName);
}

void HelpDialog::onHomeClicked() { showTopic("index"); }

void HelpDialog::onBackClicked() {
    if (historyIndex > 0) {
        historyIndex--;
        QString fileName = navigationHistory[historyIndex];

        // Load without adding to history
        QString resourcePath = QString(":/help/help/%1.md").arg(fileName);
        QFile file(resourcePath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            QString content = in.readAll();
            file.close();
            contentView->setMarkdownContent(content);
        }

        updateNavigationButtons();
        updateBreadcrumb();
    }
}

void HelpDialog::onForwardClicked() {
    if (historyIndex < navigationHistory.size() - 1) {
        historyIndex++;
        QString fileName = navigationHistory[historyIndex];

        // Load without adding to history
        QString resourcePath = QString(":/help/help/%1.md").arg(fileName);
        QFile file(resourcePath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            QString content = in.readAll();
            file.close();
            contentView->setMarkdownContent(content);
        }

        updateNavigationButtons();
        updateBreadcrumb();
    }
}

void HelpDialog::onLinkClicked(const QString& link) {
    // Handle internal wiki-style links like [[getting-started]]
    // or markdown links to help files

    QString topic = link;

    // Remove wiki-link brackets if present
    if (topic.startsWith("[[") && topic.endsWith("]]")) {
        topic = topic.mid(2, topic.length() - 4);
    }

    // Remove .md extension if present
    if (topic.endsWith(".md")) {
        topic = topic.left(topic.length() - 3);
    }

    // Handle anchor links (remove #section part)
    int hashPos = topic.indexOf('#');
    if (hashPos > 0) {
        topic = topic.left(hashPos);
    }

    // Check if this is an external link (http://, https://)
    if (topic.startsWith("http://") || topic.startsWith("https://")) {
        // Let the preview widget handle external links
        return;
    }

    // Load the help topic
    showTopic(topic);
}

void HelpDialog::addToHistory(const QString& fileName) {
    // Remove any forward history if we're not at the end
    if (historyIndex < navigationHistory.size() - 1) {
        navigationHistory = navigationHistory.mid(0, historyIndex + 1);
    }

    // Add to history
    navigationHistory.append(fileName);
    historyIndex = navigationHistory.size() - 1;
}

void HelpDialog::updateNavigationButtons() {
    ui->backButton->setEnabled(historyIndex > 0);
    ui->forwardButton->setEnabled(historyIndex < navigationHistory.size() - 1);
}

void HelpDialog::updateBreadcrumb() {
    if (historyIndex >= 0 && historyIndex < navigationHistory.size()) {
        QString currentFileName = navigationHistory[historyIndex];

        // Find the display name for current topic
        QString displayName = currentFileName;
        for (const HelpTopic& topic : topics) {
            if (topic.fileName == currentFileName) {
                displayName = topic.displayName;
                break;
            }
        }

        ui->breadcrumbLabel->setText(displayName);
    }
}

void HelpDialog::onSearchTextChanged(const QString& text) {
    if (text.trimmed().isEmpty()) {
        // Switch back to TOC view
        ui->leftPanelStack->setCurrentIndex(0);
        ui->searchResultsList->clear();
    } else {
        performSearch(text);
    }
}

void HelpDialog::onSearchResultClicked(QListWidgetItem* item) {
    if (!item) return;

    // Get the file path from the item data
    QString filePath = item->data(Qt::UserRole).toString();

    // Extract just the filename without extension
    QString fileName = filePath;
    fileName.replace(":/help/help/", "");
    fileName.replace(".md", "");

    // Clear search and load the topic
    ui->searchLineEdit->clear();
    showTopic(fileName);
}

void HelpDialog::performSearch(const QString& searchTerm) {
    QString term = searchTerm.trimmed();

    // Build list of all help file paths
    QStringList filePaths;
    for (const HelpTopic& topic : topics) {
        filePaths.append(QString(":/help/help/%1.md").arg(topic.fileName));
    }

    // Perform search using the search engine
    QList<SearchEngine::SearchResult> results =
        searchEngine->searchInFiles(filePaths, term);

    // Clear and populate search results list
    ui->searchResultsList->clear();

    if (results.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(tr("No results found"));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        ui->searchResultsList->addItem(item);
    } else {
        for (const SearchEngine::SearchResult& result : results) {
            // Create display text with title and context
            QString displayText =
                QString("%1\n%2")
                    .arg(result.title.isEmpty() ? result.fileName
                                                : result.title)
                    .arg(result.context);

            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, result.filePath);
            item->setToolTip(QString("Line %1: %2")
                                 .arg(result.lineNumber)
                                 .arg(result.context));
            ui->searchResultsList->addItem(item);
        }

        // Update results label
        ui->searchResultsLabel->setText(
            tr("Search Results (%1)").arg(results.size()));
    }

    // Switch to search results view
    ui->leftPanelStack->setCurrentIndex(1);
}

void HelpDialog::onPrintClicked() {
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDialog(&printer, this);
    printDialog.setWindowTitle(tr("Print Help"));

    if (printDialog.exec() == QDialog::Accepted) {
        contentView->print(&printer);
    }
#else
    QMessageBox::information(this, tr("Print"),
                             tr("Printing is not supported in this build."));
#endif
}
