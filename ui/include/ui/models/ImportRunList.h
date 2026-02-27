#pragma once

#include <QAbstractListModel>
#include <QString>

#include <vector>

class ImportRunList : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        TypeRole,
        FileRole,
        StatusRole,
        MessageRole
    };

    explicit ImportRunList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addRun(QString time, QString type, QString file, QString status, QString message);

    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();

private:
    struct Run {
        QString time;
        QString type;
        QString file;
        QString status;
        QString message;
    };

    std::vector<Run> runs_;
};
