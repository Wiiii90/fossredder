#include "ui/models/AnalysisList.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

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

    std::string adjustmentsJson = "{}";
    try {
        if (!a->adjustments.empty()) {
            std::string s = "{";
            bool first = true;
            for (const auto& p : a->adjustments) {
                if (!first) s += ",";
                first = false;
                s += "\"" + p.first + "\":" + std::to_string(p.second);
            }
            s += "}";
            adjustmentsJson = s;
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      core::errors::codes::ExceptionError,
                                      "ui::AnalysisList::data::adjustmentsJson",
                                      std::current_exception());
        adjustmentsJson = "{}";
    }

    switch (role) {
    case IdRole: return QString::fromStdString(a->id);
    case NameRole: return QString::fromStdString(a->name);
    case TypeRole: return QString::fromStdString(a->type);
    case ConfigRole: return QString::fromStdString(a->configJson);
    case FilterRole: return QString::fromStdString(a->filterSpec);
    case AdjustmentsRole: return QString::fromStdString(adjustmentsJson);
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

std::vector<std::shared_ptr<Analysis>> AnalysisList::analyses() const {
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
                std::string s = json.toStdString();
                size_t pos = 0;
                while (pos < s.size()) {
                    auto q1 = s.find('"', pos);
                    if (q1 == std::string::npos) break;
                    auto q2 = s.find('"', q1 + 1);
                    if (q2 == std::string::npos) break;
                    std::string key = s.substr(q1 + 1, q2 - q1 - 1);
                    auto colon = s.find(':', q2 + 1);
                    if (colon == std::string::npos) break;
                    size_t j = colon + 1;
                    while (j < s.size() && isspace(static_cast<unsigned char>(s[j]))) ++j;
                    size_t start = j;
                    while (j < s.size() && (isdigit(static_cast<unsigned char>(s[j])) || s[j] == '.' || s[j] == '-' || s[j] == '+' || s[j] == 'e' || s[j] == 'E')) ++j;
                    if (start < j) {
                        try {
                            double v = std::stod(s.substr(start, j - start));
                            a->adjustments.emplace(key, v);
                        } catch (...) {
                            core::errors::reportException(core::errors::ErrorSeverity::Warning, "ui::AnalysisList::setAdjustmentsById::parseValue", std::current_exception());
                        }
                    }
                    pos = j;
                }
            } catch (...) {
                core::errors::reportException(core::errors::ErrorSeverity::Warning, "ui::AnalysisList::setAdjustmentsById::parseLoop", std::current_exception());
            }
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {AdjustmentsRole});
            return;
        }
    }
}

}
