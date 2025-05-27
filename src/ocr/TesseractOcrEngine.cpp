#include "pch.h"
#include "ocr/TesseractOcrEngine.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>

std::string TesseractOcrEngine::recognizeAltoXml(const std::string& imageFilePath, const std::string& tessdataPath) {
    tesseract::TessBaseAPI ocr;
    ocr.SetVariable("TESSDATA_PREFIX", tessdataPath.c_str());
    if (ocr.Init(tessdataPath.c_str(), "deu") != 0) {
        throw std::runtime_error("Failed to initialize Tesseract OCR with German language.");
    }
    Pix* image = pixRead(imageFilePath.c_str());
    if (!image) {
        throw std::runtime_error("Failed to read image file: " + imageFilePath);
    }
    ocr.SetImage(image);
    ocr.SetPageSegMode(tesseract::PSM_AUTO);

    int conf = ocr.MeanTextConf();
    std::cout << "[TESSERACT DEBUG] MeanTextConf für " << imageFilePath << ": " << conf << std::endl;

    std::string extractedText = ocr.GetAltoText(0);
    pixDestroy(&image);
    ocr.End();
    return extractedText;
}