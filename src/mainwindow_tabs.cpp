#include "defs.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "markdownpreview.h"
#include "outlinepanel.h"
#include "tabeditor.h"
#include "thememanager.h"
#include <QFileInfo>
#include <QMessageBox>

TabEditor *MainWindow::currentTabEditor() const {
  return qobject_cast<TabEditor *>(tabWidget->currentWidget());
}

TabEditor *MainWindow::createNewTab() {
  TabEditor *tab = new TabEditor(this);

  QString fontFamily = settings->value("editor/font", "Sans Serif").toString();
  int fontSize = settings->value("editor/fontSize", 11).toInt();
  int tabWidth = settings->value("editor/tabWidth", 4).toInt();
  bool wordWrap = settings->value("editor/wordWrap", true).toBool();

   QFont font(fontFamily, fontSize);
   tab->editor()->setFont(font);
   tab->editor()->setTabStopDistance(QFontMetrics(font).horizontalAdvance(' ') *
                                     tabWidth);
   tab->editor()->setLineWrapMode(wordWrap ? QTextEdit::WidgetWidth
                                           : QTextEdit::NoWrap);

  tab->editor()->setPalette(ThemeManager::instance()->getEditorPalette());
  tab->editor()->setStyleSheet(ThemeManager::instance()->getEditorStyleSheet());

  if (tab->editor()->highlighter()) {
    QString resolvedScheme = ThemeManager::instance()->getResolvedEditorColorSchemeName();
    tab->editor()->highlighter()->setColorScheme(resolvedScheme);
    bool codeSyntaxEnabled =
        settings->value("editor/enableCodeSyntax", false).toBool();
    tab->editor()->highlighter()->setCodeSyntaxEnabled(codeSyntaxEnabled);
  }

  QString previewTheme = settings->value("previewTheme", "light").toString();
  tab->preview()->setTheme(previewTheme);

   connect(tab->editor(), &MarkdownEditor::wikiLinkClicked, this,
           &MainWindow::onWikiLinkClicked);
   connect(tab->editor(), &MarkdownEditor::markdownLinkClicked, this,
           &MainWindow::onMarkdownLinkClicked);
   connect(tab->preview(), &MarkdownPreview::wikiLinkClicked, this,
           &MainWindow::onWikiLinkClicked);
   connect(tab->preview(), &MarkdownPreview::markdownLinkClicked, this,
           &MainWindow::onMarkdownLinkClicked);

  if (outlineView) {
    connect(outlineView, &OutlinePanel::headerClicked, this,
            &MainWindow::jumpToLine, Qt::UniqueConnection);
  }

  connect(tab->editor(), &MarkdownEditor::textChanged, this, [this]() {
    TabEditor *currentTab = currentTabEditor();
    if (currentTab && outlineView) {
      QString markdown = currentTab->editor()->toPlainText();
      outlineView->updateOutline(markdown);
    }
  });

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

  connect(tab, &TabEditor::filePathChanged, this, [this, tab](const QString &) {
    int index = tabWidget->indexOf(tab);
    if (index >= 0) {
      tabWidget->setTabText(index, tab->fileName());
      tabWidget->setTabToolTip(index, tab->filePath());
    }
  });

  int index = tabWidget->addTab(tab, tr("Untitled"));
  tabWidget->setCurrentIndex(index);

  if (outlineView) {
    outlineView->updateOutline(tab->editor()->toPlainText());
  }

  // Apply current view mode to the newly created tab
  applyViewMode(currentViewMode);

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

    if (outlineView) {
      QString markdown = tab->editor()->toPlainText();
      outlineView->updateOutline(markdown);
    }

     MarkdownEditor *editor = tab->editor();
     if (editor && cutAction && copyAction && undoAction && redoAction) {
       cutAction->setEnabled(editor->textCursor().hasSelection());
       copyAction->setEnabled(editor->textCursor().hasSelection());
       undoAction->setEnabled(editor->document()->isUndoAvailable());
       redoAction->setEnabled(editor->document()->isRedoAvailable());

       connect(editor, &QTextEdit::copyAvailable, cutAction,
               &QAction::setEnabled, Qt::UniqueConnection);
       connect(editor, &QTextEdit::copyAvailable, copyAction,
               &QAction::setEnabled, Qt::UniqueConnection);
       connect(editor, &QTextEdit::undoAvailable, undoAction,
               &QAction::setEnabled, Qt::UniqueConnection);
       connect(editor, &QTextEdit::redoAvailable, redoAction,
               &QAction::setEnabled, Qt::UniqueConnection);
     }

    if (!tab->filePath().isEmpty()) {
      setWindowTitle(QString("%1 - %2").arg(tab->fileName(), APP_LABEL));
    } else {
      setWindowTitle(APP_LABEL);
    }
    
    // Apply current view mode to the newly active tab
    applyViewMode(currentViewMode);
  }
}

void MainWindow::onTabCloseRequested(int index) {
  TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(index));
  if (!tab) {
    return;
  }

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
      break;
    }
  }
}
