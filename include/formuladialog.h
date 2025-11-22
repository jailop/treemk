#ifndef FORMULADIALOG_H
#define FORMULADIALOG_H

#include <QDialog>

class QTextEdit;
class QRadioButton;
class QPushButton;
class QComboBox;

class FormulaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormulaDialog(QWidget *parent = nullptr);
    ~FormulaDialog();
    
    QString getFormula() const;
    bool isBlockFormula() const;

private slots:
    void insertTemplate();
    void updatePreview();

private:
    void setupUI();
    void loadTemplates();
    
    QTextEdit *formulaEdit;
    QRadioButton *inlineRadio;
    QRadioButton *blockRadio;
    QComboBox *templateCombo;
    QPushButton *insertButton;
    QPushButton *cancelButton;
    
    QMap<QString, QString> templates;
};

#endif // FORMULADIALOG_H
