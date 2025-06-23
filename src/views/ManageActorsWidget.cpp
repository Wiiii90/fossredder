#include "views/ManageActorsWidget.h"
#include "managers/IActorManager.h"
#include "models/Actor.h"
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
struct ActorDialogResult {
    QString name;
    QString type;
    QString description;
    bool accepted;
};

ActorDialogResult showActorDialog(QWidget* parent, const QString& title, const Actor* existing = nullptr) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* typeEdit = new QLineEdit(&dialog);
    QLineEdit* descEdit = new QLineEdit(&dialog);

    if (existing) {
        nameEdit->setText(QString::fromStdString(existing->name));
        typeEdit->setText(QString::fromStdString(existing->type));
        descEdit->setText(QString::fromStdString(existing->description));
    } else {
        typeEdit->setText(QObject::tr("Mieter"));
    }

    form.addRow(QObject::tr("Name:"), nameEdit);
    form.addRow(QObject::tr("Typ:"), typeEdit);
    form.addRow(QObject::tr("Beschreibung:"), descEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    ActorDialogResult result;
    result.accepted = dialog.exec() == QDialog::Accepted;
    result.name = nameEdit->text();
    result.type = typeEdit->text();
    result.description = descEdit->text();
    return result;
}
}

ManageActorsWidget::ManageActorsWidget(std::shared_ptr<IActorManager> manager, QWidget* parent)
    : QWidget(parent), manager_(std::move(manager))
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Actors (Counterparties, Payees, Partners, etc.)"), this);
    actorsList_ = new QListWidget(this);

    QHBoxLayout* crudLayout = new QHBoxLayout();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    crudLayout->addWidget(addBtn_);
    crudLayout->addWidget(editBtn_);
    crudLayout->addWidget(deleteBtn_);
    crudLayout->addStretch();

    mainLayout->addWidget(label);
    mainLayout->addWidget(actorsList_, 1);
    mainLayout->addLayout(crudLayout);

    setLayout(mainLayout);

    refreshList();

    connect(addBtn_, &QPushButton::clicked, this, [this]() {
        auto result = showActorDialog(this, tr("Add Actor"));
        if (result.accepted && !result.name.isEmpty()) {
            manager_->addActor(result.name.toStdString(), result.type.toStdString(), result.description.toStdString());
            refreshList();
        }
    });

    connect(editBtn_, &QPushButton::clicked, this, [this]() {
        int row = actorsList_->currentRow();
        if (row < 0) return;
        auto actors = manager_->getActors();
        if (row >= static_cast<int>(actors.size())) return;
        const auto& actor = actors[row];
        auto result = showActorDialog(this, tr("Edit Actor"), actor.get());
        if (result.accepted && !result.name.isEmpty()) {
            manager_->updateActor(actor->name, result.name.toStdString(), result.type.toStdString(), result.description.toStdString());
            refreshList();
        }
    });

    connect(deleteBtn_, &QPushButton::clicked, this, [this]() {
        int row = actorsList_->currentRow();
        if (row < 0) return;
        auto actors = manager_->getActors();
        if (row >= static_cast<int>(actors.size())) return;
        const auto& actor = actors[row];
        if (QMessageBox::question(this, tr("Delete Actor"), tr("Delete '%1'?").arg(QString::fromStdString(actor->name))) == QMessageBox::Yes) {
            manager_->removeActor(actor->name);
            refreshList();
        }
    });
}

void ManageActorsWidget::refreshList()
{
    actorsList_->clear();
    for (const auto& actor : manager_->getActors()) {
        QString text = QString::fromStdString(actor->name);
        if (!actor->type.empty())
            text += " (" + QString::fromStdString(actor->type) + ")";
        if (!actor->description.empty())
            text += " - " + QString::fromStdString(actor->description);
        actorsList_->addItem(text);
    }
}