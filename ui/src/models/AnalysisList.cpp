#include "ui/models/AnalysisList.h"

#include "ui/config/Defaults.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include <exception>

namespace ui {

AnalysisList::AnalysisList(QObject* parent) : QAbstractListModel(parent) {}

int AnalysisList::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(analyses_.size());
}

QVariant AnalysisList::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(analyses_.size())) return {};
    const auto& a = analyses_[row];
    if (!a) return {};

    QString adjustmentsJson = ui::config::kJsonEmptyObject;
    try {
        QJsonObject obj;
        for (const auto& p : a->adjustments) obj.insert(QString::fromStdString(p.first), p.second);
        adjustmentsJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      core::errors::codes::ExceptionError,
                                      "ui::AnalysisList::data::adjustmentsJson",
                                      std::current_exception());
        adjustmentsJson = ui::config::kJsonEmptyObject;
    }

    switch (role) {
    case IdRole: return QString::fromStdString(a->id);
    case NameRole: return QString::fromStdString(a->name);
    case TypeRole: return QString::fromStdString(a->type);
    case ConfigRole: return QString::fromStdString(a->configJson);
    case FilterRole: return QString::fromStdString(a->filterSpec);
    case AdjustmentsRole: return adjustmentsJson;
    default: return {};
    }
}

bool AnalysisList::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(analyses_.size())) return false;
    auto& a = analyses_[row];
    if (!a) return false;

    bool changed = false;
    switch (role) {
    case NameRole: {
        const auto v = value.toString().toStdString();
        if (a->name != v) { a->name = v; changed = true; }
        break;
    }
    case TypeRole: {
        const auto v = value.toString().toStdString();
        if (a->type != v) { a->type = v; changed = true; }
        break;
    }
    case ConfigRole: {
        const auto v = value.toString().toStdString();
        if (a->configJson != v) { a->configJson = v; changed = true; }
        break;
    }
    case FilterRole: {
        const auto v = value.toString().toStdString();
        if (a->filterSpec != v) { a->filterSpec = v; changed = true; }
        break;
    }
    default:
        return false;
    }

    if (changed) emit dataChanged(index, index, {role});
    return changed;
}

Qt::ItemFlags AnalysisList::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> AnalysisList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[ConfigRole] = "configJson";
    roles[FilterRole] = "filterSpec";
    roles[AdjustmentsRole] = "adjustments";
    return roles;
}

void AnalysisList::setAnalyses(std::vector<std::shared_ptr<Analysis>> analyses) {
    beginResetModel();
    analyses_ = std::move(analyses);
    endResetModel();
}

const std::vector<std::shared_ptr<Analysis>>& AnalysisList::analyses() const {
    return analyses_;
}

int AnalysisList::addAnalysis(const QString& name, const QString& type) {
    auto a = std::make_shared<Analysis>();
    a->name = name.toStdString();
    a->type = type.toStdString();

    const int row = static_cast<int>(analyses_.size());
    beginInsertRows(QModelIndex(), row, row);
    analyses_.push_back(std::move(a));
    endInsertRows();

    return row;
}

void AnalysisList::removeAt(int row) {
    if (row < 0 || row >= static_cast<int>(analyses_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    analyses_.erase(analyses_.begin() + row);
    endRemoveRows();
}

bool AnalysisList::updateAnalysisById(const QString& id, const QString& name, const QString& type, const QString& configJson, const QString& filterSpec) {
    for (int i = 0; i < static_cast<int>(analyses_.size()); ++i) {
        const auto& a = analyses_[i];
        if (!a) continue;
        if (QString::fromStdString(a->id) == id) {
            a->name = name.toStdString();
            a->type = type.toStdString();
            a->configJson = configJson.toStdString();
            a->filterSpec = filterSpec.toStdString();
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {NameRole, TypeRole, ConfigRole, FilterRole, AdjustmentsRole});
            return true;
        }
    }
    return false;
}

void AnalysisList::setAdjustmentsById(const QString& id, const QString& json) {
    for (int i = 0; i < static_cast<int>(analyses_.size()); ++i) {
        const auto& a = analyses_[i];
        if (!a) continue;
        if (QString::fromStdString(a->id) == id) {
            a->adjustments.clear();
            try {
                QJsonParseError parseError;
                const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
                if (parseError.error != QJsonParseError::NoError) {
                    core::errors::report(core::errors::ErrorSeverity::Warning,
                                         core::errors::codes::GenericError,
                                         "ui::AnalysisList::setAdjustmentsById::parseJson",
                                         parseError.errorString().toStdString());
                } else if (doc.isObject()) {
                    const auto obj = doc.object();
                    for (auto it = obj.begin(); it != obj.end(); ++it) {
                        if (!it.value().isDouble()) continue;
                        a->adjustments.emplace(it.key().toStdString(), it.value().toDouble());
                    }
                }
            } catch (...) {
                core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                              core::errors::codes::ExceptionError,
                                              "ui::AnalysisList::setAdjustmentsById",
                                              std::current_exception());
            }
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {AdjustmentsRole});
            return;
        }
    }
}

}
