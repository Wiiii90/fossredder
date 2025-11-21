#include "tesseract/pch.h"
#include "tesseract/TesseractEngine.h"
#include "debug/IDebugger.h"
#include "tesseract/TesseractDTO.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

static Pix* pixFromBytes(const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    Pix* p = pixReadMem(data.data(), static_cast<size_t>(data.size()));
    return p;
}

static Text recognizeTextFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, std::shared_ptr<IDebugger> dbg) {
    Text out;
    tesseract::TessBaseAPI ocr;
    ocr.SetVariable("TESSDATA_PREFIX", tessdataPath.c_str());
    if (ocr.Init(tessdataPath.c_str(), "deu") != 0) return out;
    Pix* pix = pixFromBytes(data);
    if (!pix) return out;
    ocr.SetImage(pix);
    ocr.Recognize(0);
    char* txt = ocr.GetUTF8Text();
    if (txt) { out.text = string(txt); delete[] txt; }
    out.meanConfidence = ocr.MeanTextConf();
    if (dbg && dbg->enabled()) dbg->writeText(string("tesseract/output/text.txt"), out.text);
    ocr.End(); pixDestroy(&pix);
    return out;
}

static vector<Word> getWordsFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, std::shared_ptr<IDebugger> dbg) {
    vector<Word> out;
    Pix* pix = pixFromBytes(data);
    if (!pix) return out;
    tesseract::TessBaseAPI ocr; ocr.SetVariable("TESSDATA_PREFIX", tessdataPath.c_str()); if (ocr.Init(tessdataPath.c_str(), "deu") != 0) { pixDestroy(&pix); return out; }
    ocr.SetImage(pix); ocr.Recognize(0);
    char* tsv = ocr.GetTSVText(0); if (!tsv) { ocr.End(); pixDestroy(&pix); return out; }
    string s(tsv);
    if (dbg && dbg->enabled()) dbg->writeText(string("tesseract/tsv/words.tsv"), s);
    istringstream iss(s);
    string line; bool first = true;
    while (getline(iss, line)) {
        if (first) { first = false; continue; }
        if (line.empty()) continue;
        vector<string> cols; string cell; istringstream ls(line);
        while (getline(ls, cell, '\t')) cols.push_back(cell);
        if (cols.size() < 12) continue;
        int level = stoi(cols[0]); if (level != 5) continue;
        int left = stoi(cols[6]); int top = stoi(cols[7]); int width = stoi(cols[8]); int height = stoi(cols[9]); int conf = stoi(cols[10]); string text = cols[11];
        Word w; w.bbox.x = left; w.bbox.y = top; w.bbox.width = width; w.bbox.height = height; w.text = text; w.confidence = conf;
        out.push_back(move(w));
    }
    delete[] tsv; ocr.End(); pixDestroy(&pix); return out;
}

std::pair<Text, vector<Word>> TesseractEngine::extractFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, std::shared_ptr<IDebugger> dbg) {
    Text t = recognizeTextFromBytes(data, tessdataPath, dbg);
    vector<Word> w = getWordsFromBytes(data, tessdataPath, dbg);
    return {t, w};
}
