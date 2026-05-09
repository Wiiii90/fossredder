/**
 * @file core/include/core/application/export/IArchive.h
 * @brief Declares archive abstraction used by export services.
 */

#pragma once

#include "core/application/export/ExportTypes.h"

#include <filesystem>

namespace core::application::exporting {

/**
 * @brief Abstraction for writing exported artifacts into archive files.
 */
class IArchive {
public:
    /**
     * @brief Destroy the archive abstraction.
     */
    virtual ~IArchive() = default;

    /**
     * @brief Creates an archive from files inside a source directory.
     * @param sourceDirectory Directory containing the files to package.
     * @param outputArchive Target archive file path.
     * @param format Requested archive format.
     * @return True when the archive was created successfully.
     */
    virtual bool create(const std::filesystem::path& sourceDirectory,
                        const std::filesystem::path& outputArchive,
                        PackageFormat format) const = 0;
};

} // namespace core::application::exporting
