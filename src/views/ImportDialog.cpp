#include "views/ImportDialog.h"
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

ImportDialog::ImportDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Import Statement"));
    setMinimumWidth(400);
    setAcceptDrops(true);
    setupUi();
    setupConnections();
}

void ImportDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Type selection
    QHBoxLayout* typeLayout = new QHBoxLayout();
    QLabel* typeLabel = new QLabel(tr("File type:"), this);
    typeCombo_ = new QComboBox(this);
    typeCombo_->addItem("PDF");
    typeCombo_->addItem("PNG");
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(typeCombo_);
    typeLayout->addStretch();

    // Drag & Drop area and file selection
    dropAreaLabel_ = new QLabel(tr("Drop PDF/PNG here or use Browse..."), this);
    dropAreaLabel_->setStyleSheet("QLabel { border: 2px dashed #aaa; min-height: 60px; }");
    dropAreaLabel_->setAlignment(Qt::AlignCenter);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    fileNameEdit_ = new QLineEdit(this);
    fileNameEdit_->setReadOnly(true);
    browseBtn_ = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(fileNameEdit_);
    fileLayout->addWidget(browseBtn_);

    // Config and start
    QHBoxLayout* actionLayout = new QHBoxLayout();
    configBtn_ = new QPushButton(tr("Configure Import..."), this);
    startBtn_ = new QPushButton(tr("Start Processing"), this);
    startBtn_->setEnabled(false);
    actionLayout->addWidget(configBtn_);
    actionLayout->addStretch();
    actionLayout->addWidget(startBtn_);

    // Progress and status
    progressBar_ = new QProgressBar(this);
    progressBar_->setValue(0);
    statusLabel_ = new QLabel(tr("Ready."), this);

    mainLayout->addLayout(typeLayout);
    mainLayout->addWidget(dropAreaLabel_);
    mainLayout->addLayout(fileLayout);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(progressBar_);
    mainLayout->addWidget(statusLabel_);
}

void ImportDialog::setupConnections()
{
    connect(browseBtn_, &QPushButton::clicked, this, [this]() {
        QString filter = typeCombo_->currentText() == "PDF"
            ? tr("PDF Files (*.pdf)")
            : tr("PNG Files (*.png)");
        QString file = QFileDialog::getOpenFileName(this, tr("Select File"), QString(), filter);
        if (!file.isEmpty()) {
            fileNameEdit_->setText(file);
            startBtn_->setEnabled(true);
        }
        });

    connect(startBtn_, &QPushButton::clicked, this, [this]() {
        // Hier würde der Importprozess gestartet werden
        // Für den Platzhalter schließen wir einfach den Dialog mit Accepted
        accept();
        });

    // TODO: connect configBtn_ to open a configuration dialog
}

void ImportDialog::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void ImportDialog::dropEvent(QDropEvent* event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString file = urls.first().toLocalFile();
        if (file.endsWith(".pdf", Qt::CaseInsensitive) || file.endsWith(".png", Qt::CaseInsensitive)) {
            fileNameEdit_->setText(file);
            startBtn_->setEnabled(true);
        }
    }
}

QString ImportDialog::selectedFile() const { return fileNameEdit_->text(); }
QString ImportDialog::selectedType() const { return typeCombo_->currentText(); }