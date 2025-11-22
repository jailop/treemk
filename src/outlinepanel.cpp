#include "outlinepanel.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QRegularExpression>

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
    
    QLabel *titleLabel = new QLabel(tr("Document Outline"), this);
    titleLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);
    
    outlineList = new QListWidget(this);
    layout->addWidget(outlineList);
    
    connect(outlineList, &QListWidget::itemClicked,
            this, &OutlinePanel::onItemClicked);
}

void OutlinePanel::updateOutline(const QString &markdown)
{
    outlineList->clear();
    
    QList<OutlineItem> headers = parseHeaders(markdown);
    
    if (headers.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(tr("No headers found"));
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        outlineList->addItem(item);
        return;
    }
    
    for (const OutlineItem &header : headers) {
        QString indent = QString(header.level - 1, ' ') + QString(header.level - 1, ' ');
        QString displayText = indent + header.text;
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, header.lineNumber);
        item->setToolTip(QString("Line %1: %2").arg(header.lineNumber).arg(header.text));
        
        // Style by level
        QFont font = item->font();
        if (header.level == 1) {
            font.setPointSize(font.pointSize() + 2);
            font.setBold(true);
        } else if (header.level == 2) {
            font.setPointSize(font.pointSize() + 1);
            font.setBold(true);
        } else if (header.level == 3) {
            font.setBold(true);
        }
        item->setFont(font);
        
        outlineList->addItem(item);
    }
}

void OutlinePanel::onItemClicked()
{
    QListWidgetItem *item = outlineList->currentItem();
    if (item) {
        int lineNumber = item->data(Qt::UserRole).toInt();
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
