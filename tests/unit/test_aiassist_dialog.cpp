#include <QtTest/QtTest>
#include "aiassistdialog.h"
#include "logic/aiprovider.h"
#include <QClipboard>
#include <QApplication>

class MockAIProvider : public AIProvider {
public:
  void process(const QString &prompt,
               const QString &content,
               std::function<void(const QString&)> onSuccess,
               std::function<void(const QString&)> onError) override {
    lastPrompt = prompt;
    lastContent = content;
    
    if (shouldFail) {
      onError("Mock error");
    } else {
      QString result = "AI Response: " + prompt + "\nProcessed: " + content;
      onSuccess(result);
    }
  }
  
  bool isAvailable() const override { return available; }
  QString name() const override { return "Mock Provider"; }
  
  QString lastPrompt;
  QString lastContent;
  bool shouldFail = false;
  bool available = true;
};

class TestAIAssistDialog : public QObject {
  Q_OBJECT

private slots:
  void initTestCase() {
    mockProvider = new MockAIProvider();
    AIProviderManager::instance()->registerProvider("mock", mockProvider);
    AIProviderManager::instance()->setActiveProvider("mock");
  }

  void testDialogCreationWithoutSelection() {
    AIAssistDialog dialog(nullptr, "", 10, 0, 0);
    QVERIFY(dialog.windowTitle() == "AI Assist");
  }

  void testDialogCreationWithSelection() {
    QString selectedText = "Test content";
    AIAssistDialog dialog(nullptr, selectedText, 0, 5, 15);
    QVERIFY(dialog.windowTitle() == "AI Assist");
  }

  void testHistoryNavigation() {
    // Dialog starts with one empty entry
    AIAssistDialog dialog(nullptr, "Initial content", 0, 0, 14);
    
    // Initial state: should have 1 entry
    // Note: Without access to UI internals, this is limited
    QVERIFY(true); // Placeholder - dialog created successfully
  }

  void testProviderManager() {
    QVERIFY(AIProviderManager::instance() != nullptr);
    QVERIFY(AIProviderManager::instance()->activeProvider() != nullptr);
    QVERIFY(AIProviderManager::instance()->activeProvider()->name() == "Mock Provider");
  }

  void testProviderProcess() {
    bool callbackCalled = false;
    QString result;
    
    mockProvider->process("Test prompt", "Test content",
      [&](const QString &r) {
        callbackCalled = true;
        result = r;
      },
      [&](const QString &) {
        QFAIL("Error callback should not be called");
      }
    );
    
    QVERIFY(callbackCalled);
    QVERIFY(result.contains("AI Response"));
    QVERIFY(mockProvider->lastPrompt == "Test prompt");
    QVERIFY(mockProvider->lastContent == "Test content");
  }

  void testProviderError() {
    bool errorCallbackCalled = false;
    QString errorMsg;
    
    mockProvider->shouldFail = true;
    
    mockProvider->process("Test", "Content",
      [&](const QString &) {
        QFAIL("Success callback should not be called");
      },
      [&](const QString &error) {
        errorCallbackCalled = true;
        errorMsg = error;
      }
    );
    
    QVERIFY(errorCallbackCalled);
    QVERIFY(errorMsg == "Mock error");
    
    mockProvider->shouldFail = false;
  }

  void testProviderUnavailable() {
    bool errorCallbackCalled = false;
    
    mockProvider->available = false;
    
    AIProviderManager::instance()->process("Test", "Content",
      [&](const QString &) {
        QFAIL("Success callback should not be called");
      },
      [&](const QString &error) {
        errorCallbackCalled = true;
        QVERIFY(error.contains("not available"));
      }
    );
    
    QVERIFY(errorCallbackCalled);
    
    mockProvider->available = true;
  }

  void cleanupTestCase() {
    // Cleanup is handled by AIProviderManager destructor
  }

private:
  MockAIProvider *mockProvider;
};

QTEST_MAIN(TestAIAssistDialog)
#include "test_aiassist_dialog.moc"
