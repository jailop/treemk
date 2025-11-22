#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class QAction;
class QMenu;
class QSplitter;
class QWidget;
class QTimer;
class QListWidget;
class QTabWidget;
class FileSystemTreeView;
class TabEditor;
class MarkdownEditor;
class MarkdownPreview;
class LinkParser;
class SearchDialog;
class SettingsDialog;
class QuickOpenDialog;
class OutlinePanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFolder();
    void save();
    void saveAs();
    void about();
    void toggleTreeView();
    void togglePreview();
    void toggleBacklinks();
    void toggleOutline();
    void onFileSelected(const QString &filePath);
    void onFileDoubleClicked(const QString &filePath);
    void onFileModifiedExternally(const QString &filePath);
    void onDocumentModified();
    void autoSave();
    void find();
    void findAndReplace();
    void updatePreview();
    void setPreviewThemeLight();
    void setPreviewThemeDark();
    void setPreviewThemeSepia();
    void onWikiLinkClicked(const QString &linkTarget);
    void updateBacklinks();
    void searchInFiles();
    void openSettings();
    void onSearchResultSelected(const QString &filePath, int lineNumber);
    void insertImage();
    void insertFormula();
    void insertWikiLink();
    void insertHeader();
    void insertBold();
    void insertItalic();
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

private:
    void createActions();
    void createMenus();
    void createLayout();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &filePath);
    bool loadFile(const QString &filePath);
    
    TabEditor* currentTabEditor() const;
    TabEditor* createNewTab();
    TabEditor* findTabByPath(const QString &filePath) const;
    int findTabIndexByPath(const QString &filePath) const;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *insertMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QAction *newAction;
    QAction *openFolderAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *insertImageAction;
    QAction *insertFormulaAction;
    QAction *insertWikiLinkAction;
    QAction *insertHeaderAction;
    QAction *insertBoldAction;
    QAction *insertItalicAction;
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
    QAction *closeTabAction;
    QAction *closeAllTabsAction;
    QAction *toggleTreeViewAction;
    QAction *togglePreviewAction;
    QAction *toggleBacklinksAction;
    QAction *toggleOutlineAction;
    QAction *previewThemeLightAction;
    QAction *previewThemeDarkAction;
    QAction *previewThemeSepiaAction;
    QAction *settingsAction;
    QAction *aboutAction;
    QAction *aboutQtAction;

    QSplitter *mainSplitter;
    QWidget *treePanel;
    QWidget *editorPanel;
    QTabWidget *tabWidget;
    QWidget *backlinksPanel;
    
    FileSystemTreeView *treeView;
    QListWidget *backlinksView;
    LinkParser *linkParser;
    
    QStringList recentFiles;
    QString currentFolder;
    QString currentFilePath;
    QTimer *autoSaveTimer;
    QTimer *previewUpdateTimer;

    QSettings *settings;
};

#endif // MAINWINDOW_H
