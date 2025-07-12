#include "views/ManagePropertiesWidget.h"
#include "managers/IPropertyManager.h"
#include "models/Property.h"
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
#include <QDialogButtonBox>

namespace {
struct PropertyDialogResult {
    QString name;
    QString address;
    QString description;
    bool accepted = false;
};

PropertyDialogResult showPropertyDialog(QWidget* parent, const QString& title, const Property* existing = nullptr) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* addressEdit = new QLineEdit(&dialog);
    QLineEdit* descEdit = new QLineEdit(&dialog);

    if (existing) {
        nameEdit->setText(QString::fromStdString(existing->name));
        addressEdit->setText(QString::fromStdString(existing->address));
        descEdit->setText(QString::fromStdString(existing->description));
    }

    form.addRow(QObject::tr("Name:"), nameEdit);
    form.addRow(QObject::tr("Adresse:"), addressEdit);
    form.addRow(QObject::tr("Beschreibung:"), descEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    PropertyDialogResult result;
    result.accepted = dialog.exec() == QDialog::Accepted;
    result.name = nameEdit->text();
    result.address = addressEdit->text();
    result.description = descEdit->text();
    return result;
}
}

ManagePropertiesWidget::ManagePropertiesWidget(std::shared_ptr<IPropertyManager> manager, QWidget* parent)
    : QWidget(parent), manager_(std::move(manager))
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Properties"), this);
    propertiesList_ = new QListWidget(this);

    QHBoxLayout* crudLayout = new QHBoxLayout();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    crudLayout->addWidget(addBtn_);
    crudLayout->addWidget(editBtn_);
    crudLayout->addWidget(deleteBtn_);
    crudLayout->addStretch();

    mainLayout->addWidget(label);
    mainLayout->addWidget(propertiesList_, 1);
    mainLayout->addLayout(crudLayout);

    setLayout(mainLayout);

    refreshList();

    connect(addBtn_, &QPushButton::clicked, this, [this]() {
        auto result = showPropertyDialog(this, tr("Add Property"));
        if (result.accepted && !result.name.isEmpty()) {
            manager_->addProperty(result.name.toStdString(), result.address.toStdString(), result.description.toStdString());
            refreshList();
        }
    });

    connect(editBtn_, &QPushButton::clicked, this, [this]() {
        int row = propertiesList_->currentRow();
        if (row < 0) return;
        auto props = manager_->getProperties();
        if (row >= static_cast<int>(props.size())) return;
        const auto& prop = props[row];
        auto result = showPropertyDialog(this, tr("Edit Property"), prop.get());
        if (result.accepted && !result.name.isEmpty()) {
            manager_->updateProperty(prop->name, result.address.toStdString(), result.description.toStdString());
            refreshList();
        }
    });

    connect(deleteBtn_, &QPushButton::clicked, this, [this]() {
        int row = propertiesList_->currentRow();
        if (row < 0) return;
        auto props = manager_->getProperties();
        if (row >= static_cast<int>(props.size())) return;
        const auto& prop = props[row];
        if (QMessageBox::question(this, tr("Delete Property"), tr("Delete '%1'?").arg(QString::fromStdString(prop->name))) == QMessageBox::Yes) {
            manager_->removeProperty(prop->name);
            refreshList();
        }
    });
}

void ManagePropertiesWidget::refreshList()
{
    propertiesList_->clear();
    for (const auto& prop : manager_->getProperties()) {
        QString text = QString::fromStdString(prop->name);
        if (!prop->address.empty())
            text += " (" + QString::fromStdString(prop->address) + ")";
        if (!prop->description.empty())
            text += " - " + QString::fromStdString(prop->description);
        propertiesList_->addItem(text);
    }
}