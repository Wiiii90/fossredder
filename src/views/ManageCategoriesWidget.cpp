#include "views/ManageCategoriesWidget.h"
#include "managers/ICategoryManager.h"
#include "models/Category.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

namespace {
struct CategoryDialogResult {
    QString name;
    bool isAllocatable = true;
    QString description;
    bool accepted = false;
};

CategoryDialogResult showCategoryDialog(QWidget* parent, const QString& title, const Category* existing = nullptr) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QCheckBox* allocatableBox = new QCheckBox(QObject::tr("Umlegbar"), &dialog);
    QLineEdit* descEdit = new QLineEdit(&dialog);

    if (existing) {
        nameEdit->setText(QString::fromStdString(existing->name));
        allocatableBox->setChecked(existing->isAllocatable);
        descEdit->setText(QString::fromStdString(existing->description));
    } else {
        allocatableBox->setChecked(true);
    }

    form.addRow(QObject::tr("Name:"), nameEdit);
    form.addRow(QObject::tr("Umlegbar:"), allocatableBox);
    form.addRow(QObject::tr("Beschreibung:"), descEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    CategoryDialogResult result;
    result.accepted = dialog.exec() == QDialog::Accepted;
    result.name = nameEdit->text();
    result.isAllocatable = allocatableBox->isChecked();
    result.description = descEdit->text();
    return result;
}
}

ManageCategoriesWidget::ManageCategoriesWidget(std::shared_ptr<ICategoryManager> manager, QWidget* parent)
    : QWidget(parent), manager_(std::move(manager))
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Categories"), this);
    categoriesList_ = new QListWidget(this);

    QHBoxLayout* crudLayout = new QHBoxLayout();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    crudLayout->addWidget(addBtn_);
    crudLayout->addWidget(editBtn_);
    crudLayout->addWidget(deleteBtn_);
    crudLayout->addStretch();

    mainLayout->addWidget(label);
    mainLayout->addWidget(categoriesList_, 1);
    mainLayout->addLayout(crudLayout);

    setLayout(mainLayout);

    refreshList();

    connect(addBtn_, &QPushButton::clicked, this, [this]() {
        auto result = showCategoryDialog(this, tr("Add Category"));
        if (result.accepted && !result.name.isEmpty()) {
            manager_->addCategory(result.name.toStdString(), result.isAllocatable, result.description.toStdString());
            refreshList();
        }
    });

    connect(editBtn_, &QPushButton::clicked, this, [this]() {
        int row = categoriesList_->currentRow();
        if (row < 0) return;
        auto cats = manager_->getCategories();
        if (row >= static_cast<int>(cats.size())) return;
        const auto& cat = cats[row];
        auto result = showCategoryDialog(this, tr("Edit Category"), cat.get());
        if (result.accepted && !result.name.isEmpty()) {
            manager_->updateCategory(cat->name, result.name.toStdString(), result.isAllocatable, result.description.toStdString());
            refreshList();
        }
    });

    connect(deleteBtn_, &QPushButton::clicked, this, [this]() {
        int row = categoriesList_->currentRow();
        if (row < 0) return;
        auto cats = manager_->getCategories();
        if (row >= static_cast<int>(cats.size())) return;
        const auto& cat = cats[row];
        if (QMessageBox::question(this, tr("Delete Category"), tr("Delete '%1'?").arg(QString::fromStdString(cat->name))) == QMessageBox::Yes) {
            manager_->removeCategory(cat->name);
            refreshList();
        }
    });
}

void ManageCategoriesWidget::refreshList()
{
    categoriesList_->clear();
    for (const auto& cat : manager_->getCategories()) {
        QString text = QString::fromStdString(cat->name) +
            (cat->isAllocatable ? " [umlegbar]" : " [nicht umlegbar]");
        if (!cat->description.empty())
            text += " - " + QString::fromStdString(cat->description);
        categoriesList_->addItem(text);
    }
}