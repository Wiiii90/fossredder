#include "pch.h"
#include "models/Page.h"
#include "tinyxml2.h"
#include "models/Header.h"
#include <sstream>
#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <cctype>
#include <algorithm>

// Hilfsfunktionen (könnten auch in ein Util-File)
namespace {
    std::string trim(const std::string& s) {
        if (s.empty()) return s;
        size_t start = s.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\n\r\f\v");
        return s.substr(start, end - start + 1);
    }

    bool normalizedEquals(const std::string& a, const std::string& b) {
        UErrorCode error = U_ZERO_ERROR;
        const icu::Normalizer2* normalizer = icu::Normalizer2::getNFCInstance(error);
        if (U_FAILURE(error)) return false;
        icu::UnicodeString ua = icu::UnicodeString::fromUTF8(a);
        icu::UnicodeString ub = icu::UnicodeString::fromUTF8(b);
        icu::UnicodeString na, nb;
        normalizer->normalize(ua, na, error);
        normalizer->normalize(ub, nb, error);
        return na.caseCompare(nb, 0) == 0;
    }
}

Page::Page(const std::string& altoXml) {
    tinyxml2::XMLDocument doc;
    if (doc.Parse(altoXml.c_str()) != tinyxml2::XML_SUCCESS)
        throw std::runtime_error("Failed to parse ALTO XML.");

    tinyxml2::XMLElement* pageElem = doc.FirstChildElement("Page");
    if (!pageElem)
        throw std::runtime_error("ALTO XML does not contain a valid Page element.");
    pageElem->QueryIntAttribute("WIDTH", &width);

    tinyxml2::XMLElement* printSpace = pageElem->FirstChildElement("PrintSpace");
    if (!printSpace)
        throw std::runtime_error("ALTO XML does not contain a valid PrintSpace element.");

    for (tinyxml2::XMLElement* composedBlock = printSpace->FirstChildElement("ComposedBlock");
         composedBlock != nullptr;
         composedBlock = composedBlock->NextSiblingElement("ComposedBlock")) {
        for (tinyxml2::XMLElement* textBlock = composedBlock->FirstChildElement("TextBlock");
             textBlock != nullptr;
             textBlock = textBlock->NextSiblingElement("TextBlock")) {
            try {
                blocks.emplace_back(std::make_shared<Block>(textBlock));
            } catch (...) {}
        }
    }
}

const std::vector<std::shared_ptr<Block>>& Page::getBlocks() const {
    return blocks;
}

int Page::getWidth() const {
    return width;
}

std::vector<Header> Page::extractHeaders(const std::vector<std::string>& headerKeywords) const {
    std::vector<Header> headers;

    for (const auto& block : blocks) {
        // Wir gehen jede Zeile (Line) im Block durch
        for (const auto& para : block->paragraphs) {
            for (const auto& line : para.lines) {
                int vpos = line.getY1();

                // Sammle alle Wörter der Zeile
                struct WordInfo { std::string text; int hpos; int width; };
                std::vector<WordInfo> words;
                for (const auto& word : line.words) {
                    words.push_back({word.getFormattedText(), word.getX1(), word.getWidth()});
                }

                // Für jede Header-Kombination prüfen
                for (const auto& header : headerKeywords) {
                    std::istringstream iss(header);
                    std::vector<std::string> headerWords;
                    std::string word;
                    while (iss >> word) headerWords.push_back(word);

                    for (size_t i = 0; i + headerWords.size() <= words.size(); ++i) {
                        bool match = true;
                        for (size_t j = 0; j < headerWords.size(); ++j) {
                            std::string ocrWord = trim(words[i + j].text);
                            std::string headerWord = trim(headerWords[j]);
                            if (!normalizedEquals(ocrWord, headerWord)) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            int xmin = words[i].hpos;
                            int xmax = words[i + headerWords.size() - 1].hpos + words[i + headerWords.size() - 1].width;
                            // Duplikate vermeiden
                            bool duplicate = false;
                            for (const auto& h : headers) {
                                if (h.getName() == header && std::abs(h.getXmin() - xmin) < 5 && std::abs(h.getVpos() - vpos) < 5) {
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                Header h(header, xmin, vpos);
                                h.setXmin(xmin);
                                h.setXmax(xmax);
                                headers.push_back(h);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return headers;
}