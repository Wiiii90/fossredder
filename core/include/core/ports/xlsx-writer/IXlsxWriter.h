/**
 * @file core/include/core/ports/xlsx-writer/IXlsxWriter.h
 * @brief Declares the XLSX writing port used by the export layer.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace core::ports::xlsx_writer {

class IXlsxWriter {
public:
    virtual ~IXlsxWriter() = default;

    virtual bool writeTable(const std::filesystem::path& outputPath,
                            const std::vector<std::vector<std::string>>& rows,
                            const std::string& worksheetTitle) const = 0;
};

} // namespace core::ports::xlsx_writer
