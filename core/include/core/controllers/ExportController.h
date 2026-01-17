#pragma once

#include <memory>
#include "core/export/ExportOptions.h"

namespace core::controllers::exporting {

class XlsxController;
class CsvController;

class ExportController {
public:
    explicit ExportController(std::shared_ptr<XlsxController> xlsx, std::shared_ptr<CsvController> csv);

    bool exportData(const ExportOptions& opts);

private:
    std::shared_ptr<XlsxController> xlsx_;
    std::shared_ptr<CsvController> csv_;
};

} // namespace core::controllers::exporting
