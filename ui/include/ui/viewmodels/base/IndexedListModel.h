/**
 * @file ui/include/ui/models/IndexedListModel.h
 * @brief Declarations for the UI IndexedListModel component.
 */

#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>

#include <memory>
#include <vector>

namespace ui::models {

template <typename Item>
class IndexedListModel : public QAbstractListModel {
public:
    explicit IndexedListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return static_cast<int>(items_.size());
    }

    const std::vector<std::shared_ptr<Item>>& items() const noexcept
    {
        return items_;
    }

    int findIndexedRow(const QString& id) const
    {
        if (id.isEmpty()) return -1;
        const auto it = idToRow_.find(id);
        return it == idToRow_.end() ? -1 : it.value();
    }

protected:
    void setItems(std::vector<std::shared_ptr<Item>> items)
    {
        beginResetModel();
        items_ = std::move(items);
        rebuildIdIndex();
        endResetModel();
    }

    int appendItem(std::shared_ptr<Item> item)
    {
        const int row = static_cast<int>(items_.size());
        beginInsertRows(QModelIndex(), row, row);
        items_.push_back(std::move(item));
        if (items_.back()) idToRow_.insert(QString::fromStdString(items_.back()->id()), row);
        endInsertRows();
        return row;
    }

    void removeItemAt(int row)
    {
        if (row < 0 || row >= static_cast<int>(items_.size())) return;
        beginRemoveRows(QModelIndex(), row, row);
        items_.erase(items_.begin() + row);
        rebuildIdIndex();
        endRemoveRows();
    }

    const std::shared_ptr<Item>& itemAtRow(int row) const
    {
        static const std::shared_ptr<Item> nullItem;
        if (row < 0 || row >= static_cast<int>(items_.size())) return nullItem;
        return items_[static_cast<size_t>(row)];
    }

private:
    void rebuildIdIndex()
    {
        idToRow_.clear();
        idToRow_.reserve(static_cast<int>(items_.size()));
        for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
            const auto& item = items_[static_cast<size_t>(i)];
            if (!item) continue;
            idToRow_.insert(QString::fromStdString(item->id()), i);
        }
    }

    std::vector<std::shared_ptr<Item>> items_;
    QHash<QString, int> idToRow_;
};

}
