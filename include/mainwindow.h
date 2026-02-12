#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

enum ViewMode {
  ViewMode_Both,       // Editor + Preview visible (default)
  ViewMode_EditorOnly, // Only Editor visible
  ViewMode_PreviewOnly // Only Preview visible
};

class QAction;
class QMenu;
class QSplitter;
class QWidget;
class QTimer;
class QListWidget;
class QTabWidget;
class QToolBar;
class QProgressBar;
class FileSystemTreeView;
class TabEditor;
class MarkdownEditor;
class MarkdownPreview;
class LinkParser;
class SearchDialog;
class SettingsDialog;
class QuickOpenDialog;
class OutlinePanel;
class AIAssistDialog;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void setStartupArguments(const QString &path, const QString &file);
  void initializeSettings();

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void newFile();
  void openFolder();
  void openRecentFolder();
  void clearRecentFolders();
  void save();
  void saveAs();
  void about();
  void toggleSidebar();
  void togglePreview();
  void cycleViewMode();
  void onFileSelected(const QString &filePath);
  void onFileDoubleClicked(const QString &filePath);
  void onFileModifiedExternally(const QString &filePath);
  void onFolderChanged(const QString &folderPath);
  void onFileDeleted(const QString &filePath);
  void onFileRenamed(const QString &oldPath, const QString &newPath);
  void onDocumentModified();
  void autoSave();
  void find();
  void findAndReplace();
  void updatePreview();
  void setPreviewThemeLight();
  void setPreviewThemeDark();
  void setPreviewThemeSepia();
   void onWikiLinkClicked(const QString &linkTarget);
   void onMarkdownLinkClicked(const QString &linkTarget);
   void onOpenLinkInNewWindow(const QString &linkTarget);
  void updateBacklinks();
  void searchInFiles();
  void openSettings();
  void applySettings();
  void onSearchResultSelected(const QString &filePath, int lineNumber);
  void insertImage();
  void insertFormula();
  void insertWikiLink();
  void attachDocument();
  void insertHeader();
   void insertBold();
   void insertItalic();
   void insertStrikethrough();
  void insertCode();
  void insertCodeBlock();
  void insertList();
  void insertNumberedList();
  void insertBlockquote();
  void insertHorizontalRule();
  void insertLink();
  void insertTable();
  void quickOpen();
  void jumpToLine(int lineNumber);
  void onTabChanged(int index);
  void onTabCloseRequested(int index);
  void closeCurrentTab();
  void closeAllTabs();
  void exportToHtml();
  void exportToPdf();
  void exportToDocx();
  void exportToPlainText();
   void showKeyboardShortcuts();
   void breakLines();
   void joinLines();
   void openAIAssist();
   void processAIWithPrompt(const QString &promptText);

private:
  void createActions();
  void createMenus();
  void createToolbar();
  void createLayout();
  void readSettings();
  void writeSettings();
  void updateRecentFolders(const QString &folder);
  void populateRecentFoldersMenu();
  bool maybeSave();
  bool saveFile(const QString &filePath);
  bool loadFile(const QString &filePath);
  void applyViewMode(ViewMode mode, bool showStatusMessage = true);
  void closeTabsFromOtherFolders();

  TabEditor *currentTabEditor() const;
  TabEditor *createNewTab();
  TabEditor *findTabByPath(const QString &filePath) const;
  int findTabIndexByPath(const QString &filePath) const;
  bool createFileFromLink(const QString &targetFile, const QString &linkTarget);

  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *insertMenu;
  QMenu *viewMenu;
  QMenu *helpMenu;
  QMenu *recentFoldersMenu;

  QAction *newAction;
  QAction *openFolderAction;
  QAction *saveAction;
  QAction *saveAsAction;
  QAction *exportHtmlAction;
  QAction *exportPdfAction;
  QAction *exportDocxAction;
  QAction *exportPlainTextAction;
  QAction *insertImageAction;
  QAction *insertFormulaAction;
  QAction *insertWikiLinkAction;
  QAction *attachDocumentAction;
  QAction *insertHeaderAction;
   QAction *insertBoldAction;
   QAction *insertItalicAction;
   QAction *insertStrikethroughAction;
  QAction *insertCodeAction;
  QAction *insertCodeBlockAction;
  QAction *insertListAction;
  QAction *insertNumberedListAction;
  QAction *insertBlockquoteAction;
  QAction *insertHorizontalRuleAction;
  QAction *insertLinkAction;
  QAction *insertTableAction;
  QAction *exitAction;
  QAction *undoAction;
  QAction *redoAction;
  QAction *cutAction;
  QAction *copyAction;
  QAction *pasteAction;
  QAction *findAction;
  QAction *findReplaceAction;
  QAction *searchInFilesAction;
   QAction *quickOpenAction;
   QAction *breakLinesAction;
   QAction *joinLinesAction;
   QAction *closeTabAction;
  QAction *closeAllTabsAction;
  QAction *toggleSidebarAction;
  QAction *togglePreviewAction;
  QAction *cycleViewModeAction;
  QAction *previewThemeLightAction;
  QAction *previewThemeDarkAction;
  QAction *previewThemeSepiaAction;
  QAction *settingsAction;
  QAction *aboutAction;
  QAction *aboutQtAction;
  QAction *keyboardShortcutsAction;
  QAction *aiAssistAction;

  QToolBar *mainToolbar;
  QProgressBar *progressBar;

  QSplitter *mainSplitter;
  QTabWidget *leftTabWidget;
  QWidget *treePanel;
  QWidget *outlinePanel;
  QWidget *editorPanel;
  QTabWidget *tabWidget;
  QWidget *backlinksPanel;

  FileSystemTreeView *treeView;
  OutlinePanel *outlineView;
  QListWidget *backlinksView;
  LinkParser *linkParser;

   QStringList recentFiles;
   QStringList recentFolders;
   QString currentFolder;
   QString currentFilePath;
   QTimer *autoSaveTimer;
   QTimer *previewUpdateTimer;
   QString m_startupPath;
   QString m_startupFile;

  QSettings *settings;
  ViewMode currentViewMode;
};

#endif // MAINWINDOW_H
