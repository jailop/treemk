#include "mainwindow.h"
#include "tabeditor.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "markdownpreview.h"
#include "outlinepanel.h"
#include "thememanager.h"
#include <QMessageBox>
#include <QFileInfo>

TabEditor *MainWindow::currentTabEditor() const {
  return qobject_cast<TabEditor *>(tabWidget->currentWidget());
}

TabEditor *MainWindow::createNewTab() {
  TabEditor *tab = new TabEditor(this);

  // Apply font settings to new tab
  QString fontFamily = settings->value("editor/font", "Sans Serif").toString();
  int fontSize = settings->value("editor/fontSize", 11).toInt();
  int tabWidth = settings->value("editor/tabWidth", 4).toInt();
  bool wordWrap = settings->value("editor/wordWrap", true).toBool();

  QFont font(fontFamily, fontSize);
  tab->editor()->setFont(font);
  tab->editor()->setTabStopDistance(QFontMetrics(font).horizontalAdvance(' ') *
                                    tabWidth);
  tab->editor()->setLineWrapMode(wordWrap ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);

  // Apply editor color scheme
  QString editorScheme =
      settings->value("appearance/editorColorScheme", "auto").toString();
  tab->editor()->setPalette(ThemeManager::instance()->getEditorPalette());
  tab->editor()->setStyleSheet(ThemeManager::instance()->getEditorStyleSheet());

  if (tab->editor()->highlighter()) {
    tab->editor()->highlighter()->setColorScheme(editorScheme);
    bool codeSyntaxEnabled =
        settings->value("editor/enableCodeSyntax", false).toBool();
    tab->editor()->highlighter()->setCodeSyntaxEnabled(codeSyntaxEnabled);
  }

  // Apply preview theme
  QString previewTheme = settings->value("previewTheme", "light").toString();
  tab->preview()->setTheme(previewTheme);

  // Connect wiki link clicks from editor
  connect(tab->editor(), &MarkdownEditor::wikiLinkClicked, this,
          &MainWindow::onWikiLinkClicked);

  // Connect wiki link clicks from preview
  connect(tab->preview(), &MarkdownPreview::wikiLinkClicked, this,
          &MainWindow::onWikiLinkClicked);

  // Connect shared outline clicks to jump to line in current editor
  if (outlineView) {
    connect(outlineView, &OutlinePanel::headerClicked, this,
            &MainWindow::jumpToLine, Qt::UniqueConnection);
  }

  // Connect editor text changes to update outline
  connect(tab->editor(), &MarkdownEditor::textChanged, this, [this]() {
    TabEditor *currentTab = currentTabEditor();
    if (currentTab && outlineView) {
      QString markdown = currentTab->editor()->toPlainText();
      outlineView->updateOutline(markdown);
    }
  });

  // Connect modification signal
  connect(tab, &TabEditor::modificationChanged, this,
          [this, tab](bool modified) {
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
  connect(tab, &TabEditor::filePathChanged, this, [this, tab](const QString &) {
    int index = tabWidget->indexOf(tab);
    if (index >= 0) {
      tabWidget->setTabText(index, tab->fileName());
      tabWidget->setTabToolTip(index, tab->filePath());
    }
  });

  int index = tabWidget->addTab(tab, tr("Untitled"));
  tabWidget->setCurrentIndex(index);

  // Initialize outline for the new tab (empty content)
  if (outlineView) {
    outlineView->updateOutline(tab->editor()->toPlainText());
  }

  return tab;
}

TabEditor *MainWindow::findTabByPath(const QString &filePath) const {
  for (int i = 0; i < tabWidget->count(); ++i) {
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && tab->filePath() == filePath) {
      return tab;
    }
  }
  return nullptr;
}

int MainWindow::findTabIndexByPath(const QString &filePath) const {
  for (int i = 0; i < tabWidget->count(); ++i) {
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && tab->filePath() == filePath) {
      return i;
    }
  }
  return -1;
}

void MainWindow::onTabChanged(int index) {
  TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(index));
  if (tab) {
    currentFilePath = tab->filePath();
    updateBacklinks();

    // Update the shared outline view with current tab's content
    if (outlineView) {
      QString markdown = tab->editor()->toPlainText();
      outlineView->updateOutline(markdown);
    }

    // Connect editor signals to enable/disable actions
    MarkdownEditor *editor = tab->editor();
    if (editor && cutAction && copyAction && undoAction && redoAction) {
      // Update action states based on current editor state
      cutAction->setEnabled(editor->textCursor().hasSelection());
      copyAction->setEnabled(editor->textCursor().hasSelection());
      undoAction->setEnabled(editor->document()->isUndoAvailable());
      redoAction->setEnabled(editor->document()->isRedoAvailable());
      
      // Connect signals to keep actions updated (use UniqueConnection to avoid duplicates)
      connect(editor, &QPlainTextEdit::copyAvailable, cutAction,
              &QAction::setEnabled, Qt::UniqueConnection);
      connect(editor, &QPlainTextEdit::copyAvailable, copyAction,
              &QAction::setEnabled, Qt::UniqueConnection);
      connect(editor, &QPlainTextEdit::undoAvailable, undoAction,
              &QAction::setEnabled, Qt::UniqueConnection);
      connect(editor, &QPlainTextEdit::redoAvailable, redoAction,
              &QAction::setEnabled, Qt::UniqueConnection);
    }

    // Update window title
    if (!tab->filePath().isEmpty()) {
      setWindowTitle(QString("%1 - TreeMk").arg(tab->fileName()));
    } else {
      setWindowTitle("TreeMk");
    }
  }
}

void MainWindow::onTabCloseRequested(int index) {
  TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(index));
  if (!tab) {
    return;
  }

  // If the content is empty and unmodified, close without prompt
  bool shouldSave =
      !tab->editor()->toPlainText().isEmpty() && tab->isModified();
  if (shouldSave) {
    tabWidget->setCurrentIndex(index);
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Unsaved Changes"),
        tr("Do you want to save changes to '%1'?").arg(tab->fileName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (reply == QMessageBox::Save) {
      if (tab->filePath().isEmpty()) {
        saveAs();
      } else {
        tab->saveFile();
      }
      if (tab->isModified()) {
        return;
      }
    } else if (reply == QMessageBox::Cancel) {
      return;
    }
  }

  tabWidget->removeTab(index);
  delete tab;

  if (tabWidget->count() == 0) {
    createNewTab();
  }
}

void MainWindow::closeCurrentTab() {
  int index = tabWidget->currentIndex();
  if (index >= 0) {
    onTabCloseRequested(index);
  }
}

void MainWindow::closeAllTabs() {
  while (tabWidget->count() > 0) {
    onTabCloseRequested(0);
    if (tabWidget->count() > 0 &&
        qobject_cast<TabEditor *>(tabWidget->widget(0))->isModified()) {
      // User cancelled, stop closing
      break;
    }
  }
}
