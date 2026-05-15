/**
 * @file ui/include/ui/state/StatusState.h
 * @brief Declarations for the UI StatusState component.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace ui {

class StatusState : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(StatusState)
    QML_UNCREATABLE("StatusState is provided by the application context")
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit StatusState(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    QString text() const { return text_; }

    void setText(const QString& value)
    {
        if (text_ == value) return;
        text_ = value;
        emit textChanged();
    }

signals:
    void textChanged();

private:
    QString text_;
};

}
