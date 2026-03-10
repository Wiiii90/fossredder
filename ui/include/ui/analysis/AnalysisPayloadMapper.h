#pragma once

#include <QVariantMap>

class Analysis;

namespace ui::analysis {

QVariantMap toPayload(const Analysis& result);

}
