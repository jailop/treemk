#include "outlinepanel.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>

OutlinePanel::OutlinePanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

OutlinePanel::~OutlinePanel()
{
}

void OutlinePanel::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    outlineTree = new QTreeWidget(this);
    outlineTree->setHeaderHidden(true);
    outlineTree->setIndentation(15);
    outlineTree->setAnimated(true);
    outlineTree->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Install event filter to handle keyboard shortcuts
    outlineTree->installEventFilter(this);
    
    layout->addWidget(outlineTree);
    
    connect(outlineTree, &QTreeWidget::itemClicked,
            this, &OutlinePanel::onItemClicked);
    connect(outlineTree, &QTreeWidget::customContextMenuRequested,
            this, &OutlinePanel::showContextMenu);
}

void OutlinePanel::updateOutline(const QString &markdown)
{
    outlineTree->clear();
    
    QList<OutlineItem> headers = parseHeaders(markdown);
    
    if (headers.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(outlineTree);
        item->setText(0, tr("No headers found"));
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        return;
    }
    
    buildTree(headers);
}

void OutlinePanel::buildTree(const QList<OutlineItem> &headers)
{
    QList<QTreeWidgetItem*> levelStack;
    levelStack.append(nullptr); // Root level
    
    for (const OutlineItem &header : headers) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, header.text);
        item->setData(0, Qt::UserRole, header.lineNumber);
        item->setToolTip(0, QString("Line %1: %2").arg(header.lineNumber).arg(header.text));
        
        // Remove bold fonts - use normal font
        QFont font = item->font(0);
        font.setBold(false);
        item->setFont(0, font);
        
        // Adjust the stack to the correct level
        while (levelStack.size() > header.level) {
            levelStack.removeLast();
        }
        while (levelStack.size() < header.level) {
            levelStack.append(levelStack.last());
        }
        
        // Add item to the appropriate parent
        if (header.level == 1 || levelStack[header.level - 1] == nullptr) {
            outlineTree->addTopLevelItem(item);
        } else {
            levelStack[header.level - 1]->addChild(item);
        }
        
        // Update the stack for this level
        if (levelStack.size() == header.level) {
            levelStack.append(item);
        } else {
            levelStack[header.level] = item;
        }
    }
    
    // Expand all items by default
    outlineTree->expandAll();
}

bool OutlinePanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == outlineTree && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QTreeWidgetItem *currentItem = outlineTree->currentItem();
        
        if (!currentItem) {
            return QWidget::eventFilter(obj, event);
        }
        
        // Ctrl+Right or + : Expand current item
        if ((keyEvent->key() == Qt::Key_Right && keyEvent->modifiers() & Qt::ControlModifier) ||
            keyEvent->key() == Qt::Key_Plus) {
            currentItem->setExpanded(true);
            return true;
        }
        // Ctrl+Left or - : Collapse current item
        else if ((keyEvent->key() == Qt::Key_Left && keyEvent->modifiers() & Qt::ControlModifier) ||
                 keyEvent->key() == Qt::Key_Minus) {
            currentItem->setExpanded(false);
            return true;
        }
        // Ctrl+Shift+Right or * : Expand all
        else if ((keyEvent->key() == Qt::Key_Right && 
                  keyEvent->modifiers() & Qt::ControlModifier && 
                  keyEvent->modifiers() & Qt::ShiftModifier) ||
                 keyEvent->key() == Qt::Key_Asterisk) {
            outlineTree->expandAll();
            return true;
        }
        // Ctrl+Shift+Left or / : Collapse all
        else if ((keyEvent->key() == Qt::Key_Left && 
                  keyEvent->modifiers() & Qt::ControlModifier && 
                  keyEvent->modifiers() & Qt::ShiftModifier) ||
                 keyEvent->key() == Qt::Key_Slash) {
            outlineTree->collapseAll();
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}

void OutlinePanel::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = outlineTree->itemAt(pos);
    
    QMenu contextMenu;
    
    if (item) {
        QAction *expandAction = contextMenu.addAction(tr("Expand"));
        QAction *collapseAction = contextMenu.addAction(tr("Collapse"));
        contextMenu.addSeparator();
        
        connect(expandAction, &QAction::triggered, [item]() {
            item->setExpanded(true);
        });
        
        connect(collapseAction, &QAction::triggered, [item]() {
            item->setExpanded(false);
        });
    }
    
    QAction *expandAllAction = contextMenu.addAction(tr("Expand All"));
    QAction *collapseAllAction = contextMenu.addAction(tr("Collapse All"));
    
    connect(expandAllAction, &QAction::triggered, outlineTree, &QTreeWidget::expandAll);
    connect(collapseAllAction, &QAction::triggered, outlineTree, &QTreeWidget::collapseAll);
    
    contextMenu.exec(outlineTree->mapToGlobal(pos));
}

void OutlinePanel::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (item) {
        int lineNumber = item->data(0, Qt::UserRole).toInt();
        if (lineNumber > 0) {
            emit headerClicked(lineNumber);
        }
    }
}

QList<OutlineItem> OutlinePanel::parseHeaders(const QString &markdown)
{
    QList<OutlineItem> headers;
    
    QStringList lines = markdown.split('\n');
    
    // Regex for ATX-style headers: # Header
    QRegularExpression headerPattern("^(#{1,6})\\s+(.+)$");
    
    bool inCodeBlock = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        // Check for code block fences
        if (line.trimmed().startsWith("```")) {
            inCodeBlock = !inCodeBlock;
            continue;
        }
        
        // Skip lines inside code blocks
        if (inCodeBlock) {
            continue;
        }
        
        QString trimmedLine = line.trimmed();
        
        QRegularExpressionMatch match = headerPattern.match(trimmedLine);
        if (match.hasMatch()) {
            int level = match.captured(1).length();
            QString text = match.captured(2).trimmed();
            
            // Remove any trailing #
            text = text.remove(QRegularExpression("#*$")).trimmed();
            
            headers.append(OutlineItem(level, text, i + 1));
        }
    }
    
    return headers;
}
