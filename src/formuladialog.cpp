#include "formuladialog.h"
#include "ui_formuladialog.h"
#include <QFont>

FormulaDialog::FormulaDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::FormulaDialog) {
  ui->setupUi(this);

  // Set monospace font for formula editor
  ui->formulaEdit->setFont(QFont("Monospace", 10));

  loadTemplates();

  // Connect signals
  connect(ui->templateCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &FormulaDialog::insertTemplate);
}

FormulaDialog::~FormulaDialog() { delete ui; }

void FormulaDialog::loadTemplates() {
  templates["Empty"] = "";
  templates["Fraction"] = "\\frac{a}{b}";
  templates["Square Root"] = "\\sqrt{x}";
  templates["Power"] = "x^{n}";
  templates["Subscript"] = "x_{i}";
  templates["Sum"] = "\\sum_{i=1}^{n} x_i";
  templates["Integral"] = "\\int_{a}^{b} f(x) dx";
  templates["Limit"] = "\\lim_{x \\to \\infty} f(x)";
  templates["Matrix"] = "\\begin{bmatrix} a & b \\\\ c & d \\end{bmatrix}";
  templates["Cases"] = "\\begin{cases} x, & \\text{if } x > 0 \\\\ 0, & "
                       "\\text{otherwise} \\end{cases}";
  templates["Quadratic Formula"] = "x = \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}";
  templates["Binomial"] = "\\binom{n}{k} = \\frac{n!}{k!(n-k)!}";
  templates["Partial Derivative"] = "\\frac{\\partial f}{\\partial x}";

  for (auto it = templates.constBegin(); it != templates.constEnd(); ++it) {
    ui->templateCombo->addItem(it.key(), it.value());
  }
}

void FormulaDialog::insertTemplate() {
  QString formula = ui->templateCombo->currentData().toString();
  if (!formula.isEmpty()) {
    ui->formulaEdit->setText(formula);
  }
}

void FormulaDialog::updatePreview() {
  // Preview functionality could be added here
  // For now, we rely on the main preview pane
}

QString FormulaDialog::getFormula() const {
  return ui->formulaEdit->toPlainText().trimmed();
}

bool FormulaDialog::isBlockFormula() const {
  return ui->blockRadio->isChecked();
}
