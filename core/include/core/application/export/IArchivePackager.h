/**
 * @file core/include/core/application/export/IArchivePackager.h
 * @brief Declares the default archive packager factory.
 */

#pragma once

#include <memory>

#include "core/application/export/IArchive.h"

namespace core::application::exporting {

/**
 * @brief Creates the default archive packager implementation.
 * @return Archive packager instance.
 */
std::unique_ptr<IArchive> createArchivePackager();

} // namespace core::application::exporting
