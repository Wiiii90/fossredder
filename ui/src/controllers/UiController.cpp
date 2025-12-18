#include "ui/controllers/UiController.h"
#include <QFileDialog>
#include <QMessageBox>

UiController::UiController(QObject* parent)
    : QObject(parent)
{
}

void UiController::import()
{
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    QString file = QFileDialog::getOpenFileName(parentWidget, tr("Import File"));
    if (file.isEmpty()) return;

    emit importRequested(file);
}

void UiController::exportPath()
{
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    QString file = QFileDialog::getSaveFileName(parentWidget, tr("Export File"));
    if (!file.isEmpty()) {
        QMessageBox::information(parentWidget, tr("Export"), tr("Export path: %1").arg(file));
    }
}

void UiController::about()
{
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    QMessageBox::about(parentWidget, tr("About FOSSRedder"), tr("FOSSRedder - demo"));
}
