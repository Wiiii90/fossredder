#pragma once

#include <QString>
#include <QStringList>

#include <string>
#include <vector>

namespace ui::strings {

inline std::string toStdString(const QString& s)
{
    const auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}

inline std::vector<std::string> toStdList(const QStringList& ids)
{
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(ids.size()));
    for (const auto& id : ids) out.push_back(id.toStdString());
    return out;
}

inline std::vector<std::string> toStdListTrimmed(const QStringList& values)
{
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(values.size()));
    for (const auto& value : values) {
        const auto t = value.trimmed();
        if (t.isEmpty()) continue;
        out.push_back(t.toStdString());
    }
    return out;
}

}
