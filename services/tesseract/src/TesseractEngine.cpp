#include "tesseract/pch.h"
#include "tesseract/TesseractEngine.h"
#include "debug/IDebugger.h"
#include "api/tesseract/Types.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cstdlib>

using namespace std;

static Pix* pixFromBytes(const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    Pix* p = pixReadMem(data.data(), static_cast<size_t>(data.size()));
    return p;
}

static std::string resolveTessdataPath(const std::string& provided) {
    try {
        if (!provided.empty()) {
            std::filesystem::path p(provided);
            if (std::filesystem::exists(p)) return p.string();
        }
    } catch (...) {}

    if (const char* env = std::getenv("TESSDATA_PREFIX")) {
        try { if (std::filesystem::exists(env)) return std::string(env); } catch(...) {}
    }

    std::vector<std::filesystem::path> candidates;
    try {
        candidates.push_back(std::filesystem::current_path() / "res" / "tessdata");
        candidates.push_back(std::filesystem::current_path() / "tessdata");
        // relative to source tree (use file location as hint)
        candidates.push_back(std::filesystem::path(__FILE__).parent_path().parent_path() / "res" / "tessdata");
        candidates.push_back(std::filesystem::path(__FILE__).parent_path().parent_path() / "tessdata");
    } catch (...) {}

    for (const auto &c : candidates) {
        try { if (!c.empty() && std::filesystem::exists(c)) return c.string(); } catch(...) {}
    }

    return std::string();
}

static void configureForStatements(tesseract::TessBaseAPI& ocr) {
    try { ocr.SetVariable("tessedit_ocr_engine_mode", "1"); } catch (...) {}
    try { ocr.SetVariable("preserve_interword_spaces", "1"); } catch (...) {}

    // Keep this conservative: allow letters/digits and common punctuation found in statements.
    try { ocr.SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜabcdefghijklmnopqrstuvwxyzäöüß0123456789.,:-/()'“”‘’+&% "); } catch (...) {}
}

static void applyPsm(tesseract::TessBaseAPI& ocr, int psm) {
    // Map request value to tesseract enum; ignore invalid values.
    try {
        const int maxPsm = static_cast<int>(tesseract::PSM_COUNT) - 1;
        if (psm < 0 || psm > maxPsm) {
            psm = 3;
        }
        ocr.SetPageSegMode(static_cast<tesseract::PageSegMode>(psm));
    } catch (...) {}
}

static api::tesseract::Text recognizeTextFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, int psm, std::shared_ptr<IDebugger> dbg) {
    api::tesseract::Text out;
    tesseract::TessBaseAPI ocr;
    std::string resolved = resolveTessdataPath(tessdataPath);
    if (!resolved.empty()) ocr.SetVariable("TESSDATA_PREFIX", resolved.c_str());
    if (resolved.empty() || ocr.Init(resolved.c_str(), "deu") != 0) {
        // last resort: try default Init without explicit path
        if (ocr.Init(nullptr, "deu") != 0) return out;
    }

    configureForStatements(ocr);
    applyPsm(ocr, psm);

    Pix* pix = pixFromBytes(data);
    if (!pix) { ocr.End(); return out; }
    ocr.SetImage(pix);
    ocr.Recognize(0);
    char* txt = ocr.GetUTF8Text();
    if (txt) { out.text = string(txt); delete[] txt; }
    out.meanConfidence = ocr.MeanTextConf();
    if (dbg && dbg->enabled()) dbg->writeText(string("tesseract/output/text.txt"), out.text);
    ocr.End(); pixDestroy(&pix);
    return out;
}

static vector<api::tesseract::Word> getWordsFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, int psm, std::shared_ptr<IDebugger> dbg) {
    std::vector<api::tesseract::Word> out;
    Pix* pix = pixFromBytes(data);
    if (!pix) return out;
    tesseract::TessBaseAPI ocr;
    std::string resolved = resolveTessdataPath(tessdataPath);
    if (!resolved.empty()) ocr.SetVariable("TESSDATA_PREFIX", resolved.c_str());
    if (resolved.empty() || ocr.Init(resolved.c_str(), "deu") != 0) {
        if (ocr.Init(nullptr, "deu") != 0) { pixDestroy(&pix); return out; }
    }

    configureForStatements(ocr);
    applyPsm(ocr, psm);

    ocr.SetImage(pix);
    ocr.Recognize(0);

    // Stable extraction via TSV (word level == 5).
    char* tsv = ocr.GetTSVText(0);
    if (!tsv) { ocr.End(); pixDestroy(&pix); return out; }
    string s(tsv);
    if (dbg && dbg->enabled()) dbg->writeText(string("tesseract/tsv/words.tsv"), s);

    istringstream iss(s);
    string line;
    bool first = true;
    while (getline(iss, line)) {
        if (first) { first = false; continue; }
        if (line.empty()) continue;
        vector<string> cols;
        string cell;
        istringstream ls(line);
        while (getline(ls, cell, '\t')) cols.push_back(cell);
        if (cols.size() < 12) continue;
        int level = 0;
        try { level = stoi(cols[0]); } catch (...) { continue; }
        if (level != 5) continue;
        int left = 0, top = 0, width = 0, height = 0, conf = 0;
        try {
            left = stoi(cols[6]);
            top = stoi(cols[7]);
            width = stoi(cols[8]);
            height = stoi(cols[9]);
            conf = stoi(cols[10]);
        } catch (...) {
            continue;
        }
        string text = cols[11];
        api::tesseract::Word w;
        w.bbox.x = left;
        w.bbox.y = top;
        w.bbox.width = width;
        w.bbox.height = height;
        w.text = text;
        w.confidence = conf;
        out.push_back(std::move(w));
    }

    delete[] tsv;
    ocr.End();
    pixDestroy(&pix);
    return out;
}

std::pair<api::tesseract::Text, std::vector<api::tesseract::Word>> TesseractEngine::extractFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, std::shared_ptr<IDebugger> dbg) {
    // Backward-compatible default.
    return extractFromBytes(data, tessdataPath, /*psm*/3, std::move(dbg));
}

std::pair<api::tesseract::Text, std::vector<api::tesseract::Word>> TesseractEngine::extractFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, int psm, std::shared_ptr<IDebugger> dbg) {
    api::tesseract::Text t = recognizeTextFromBytes(data, tessdataPath, psm, dbg);
    std::vector<api::tesseract::Word> w = getWordsFromBytes(data, tessdataPath, psm, dbg);
    return {t, w};
}
