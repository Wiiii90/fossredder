#pragma once

#include "IOpenCvService.h"
#include "OpenCvDTO.h"
#include <memory>

class IDebugger;

// Public header for the OpenCV based adapter/service implementation.
// Provides factory function to create the concrete OpenCV adapter instance.

std::shared_ptr<IOpenCvService> createOpenCvAdapter(std::shared_ptr<IDebugger> debugger = nullptr);
