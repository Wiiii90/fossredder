#include "pch.h"
#include "poppler/PopplerPdfRenderer.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>

std::vector<std::string> PopplerPdfRenderer::renderToImages(const std::string& pdfPath, const std::string& outputPrefix) {
    std::string command = "pdftoppm -png -r 256 \"" + pdfPath + "\" " + outputPrefix;
    if (std::system(command.c_str()) != 0) {
        throw std::runtime_error("Failed to execute pdftoppm. Ensure it is installed and in your PATH.");
    }
    std::vector<std::string> imageFiles;
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        std::string fileName = entry.path().filename().string();
        if (fileName.find(outputPrefix) == 0 && fileName.find(".png") != std::string::npos) {
            imageFiles.push_back(fileName);
        }
    }
    return imageFiles;
}