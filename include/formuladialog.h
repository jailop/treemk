#ifndef FORMULADIALOG_H
#define FORMULADIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class FormulaDialog;
}

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
    void loadTemplates();
    
    Ui::FormulaDialog *ui;
    QMap<QString, QString> templates;
};

#endif // FORMULADIALOG_H
