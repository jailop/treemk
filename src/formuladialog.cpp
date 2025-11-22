#include "formuladialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>

FormulaDialog::FormulaDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Insert LaTeX Formula"));
    setMinimumSize(500, 400);
    
    setupUI();
    loadTemplates();
}

FormulaDialog::~FormulaDialog()
{
}

void FormulaDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Type selection
    QGroupBox *typeGroup = new QGroupBox(tr("Formula Type"), this);
    QHBoxLayout *typeLayout = new QHBoxLayout(typeGroup);
    
    inlineRadio = new QRadioButton(tr("Inline ($...$)"), this);
    blockRadio = new QRadioButton(tr("Block ($$...$$)"), this);
    inlineRadio->setChecked(true);
    
    QButtonGroup *typeButtonGroup = new QButtonGroup(this);
    typeButtonGroup->addButton(inlineRadio);
    typeButtonGroup->addButton(blockRadio);
    
    typeLayout->addWidget(inlineRadio);
    typeLayout->addWidget(blockRadio);
    typeLayout->addStretch();
    
    mainLayout->addWidget(typeGroup);
    
    // Template selection
    QHBoxLayout *templateLayout = new QHBoxLayout();
    QLabel *templateLabel = new QLabel(tr("Template:"), this);
    templateCombo = new QComboBox(this);
    
    templateLayout->addWidget(templateLabel);
    templateLayout->addWidget(templateCombo, 1);
    
    mainLayout->addLayout(templateLayout);
    
    // Formula editor
    QLabel *formulaLabel = new QLabel(tr("LaTeX Formula:"), this);
    mainLayout->addWidget(formulaLabel);
    
    formulaEdit = new QTextEdit(this);
    formulaEdit->setPlaceholderText(tr("Enter LaTeX formula here..."));
    formulaEdit->setFont(QFont("Monospace", 10));
    mainLayout->addWidget(formulaEdit);
    
    // Info label
    QLabel *infoLabel = new QLabel(
        tr("<small><b>Examples:</b><br>"
           "Inline: x^2 + y^2 = z^2<br>"
           "Block: \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}<br>"
           "Greek: \\alpha, \\beta, \\gamma<br>"
           "Symbols: \\sum, \\int, \\infty</small>"), this);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    insertButton = new QPushButton(tr("Insert"), this);
    insertButton->setDefault(true);
    cancelButton = new QPushButton(tr("Cancel"), this);
    
    buttonLayout->addWidget(insertButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(templateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormulaDialog::insertTemplate);
    connect(insertButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void FormulaDialog::loadTemplates()
{
    templates["Empty"] = "";
    templates["Fraction"] = "\\frac{a}{b}";
    templates["Square Root"] = "\\sqrt{x}";
    templates["Power"] = "x^{n}";
    templates["Subscript"] = "x_{i}";
    templates["Sum"] = "\\sum_{i=1}^{n} x_i";
    templates["Integral"] = "\\int_{a}^{b} f(x) dx";
    templates["Limit"] = "\\lim_{x \\to \\infty} f(x)";
    templates["Matrix"] = "\\begin{bmatrix} a & b \\\\ c & d \\end{bmatrix}";
    templates["Cases"] = "\\begin{cases} x, & \\text{if } x > 0 \\\\ 0, & \\text{otherwise} \\end{cases}";
    templates["Quadratic Formula"] = "x = \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}";
    templates["Binomial"] = "\\binom{n}{k} = \\frac{n!}{k!(n-k)!}";
    templates["Partial Derivative"] = "\\frac{\\partial f}{\\partial x}";
    
    for (auto it = templates.constBegin(); it != templates.constEnd(); ++it) {
        templateCombo->addItem(it.key(), it.value());
    }
}

void FormulaDialog::insertTemplate()
{
    QString formula = templateCombo->currentData().toString();
    if (!formula.isEmpty()) {
        formulaEdit->setText(formula);
    }
}

void FormulaDialog::updatePreview()
{
    // Preview functionality could be added here
    // For now, we rely on the main preview pane
}

QString FormulaDialog::getFormula() const
{
    QString formula = formulaEdit->toPlainText().trimmed();
    
    if (formula.isEmpty()) {
        return QString();
    }
    
    if (isBlockFormula()) {
        return "$$" + formula + "$$";
    } else {
        return "$" + formula + "$";
    }
}

bool FormulaDialog::isBlockFormula() const
{
    return blockRadio->isChecked();
}
