#pragma once
#include <QDialog>

class QComboBox;
class QPushButton;
class QLabel;
class QProgressBar;
class QLineEdit;

class ImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportDialog(QWidget* parent = nullptr);

    QString selectedFile() const;
    QString selectedType() const;

private:
    QComboBox* typeCombo_;
    QLabel* dropAreaLabel_;
    QLineEdit* fileNameEdit_;
    QPushButton* browseBtn_;
    QPushButton* configBtn_;
    QPushButton* startBtn_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;

    void setupUi();
    void setupConnections();
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};