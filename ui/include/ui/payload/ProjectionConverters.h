#pragma once

#include <QStringList>

#include <string>
#include <vector>

namespace ui::payload::projection {

inline QStringList toQStringList(const std::vector<std::string>& values)
{
    QStringList out;
    out.reserve(static_cast<int>(values.size()));
    for (const auto& value : values) {
        out.push_back(QString::fromStdString(value));
    }
    return out;
}

} // namespace ui::payload::projection
