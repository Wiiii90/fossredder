/**
 * @file core/include/core/export/IArchive.h
 * @brief Declares archive abstraction used by export services.
 */

#pragma once

#include "core/export/ExportTypes.h"

#include <filesystem>

namespace core::exporting {

/**
 * @brief Abstraction for writing exported artifacts into archive files.
 */
class IArchive {
public:
    virtual ~IArchive() = default;

    /**
     * @brief Create an archive from files inside source directory.
     * @return true if the archive was created successfully.
     */
    virtual bool create(const std::filesystem::path& sourceDirectory,
                        const std::filesystem::path& outputArchive,
                        PackageFormat format) const = 0;
};

} // namespace core::exporting
