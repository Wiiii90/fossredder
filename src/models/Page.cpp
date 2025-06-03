#include "pch.h"
#include "models/Page.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Word.h"
#include "tinyxml2.h"

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

Page::Page(const std::string& altoXml, int index,
    const std::vector<std::string>& headerKeywords,
    const std::vector<std::string>& footerKeywords) : index(index) {
    extractTextElements(altoXml);

    if (!headerKeywords.empty()) {
        extractHeaders(headerKeywords);

        int maxHeaderY = 0;
        for (const auto& h : headers) {
            maxHeaderY = std::max(maxHeaderY, h->getY2());
        }
        if (maxHeaderY > 0) {
            crop(CropDirection::TOP, maxHeaderY);
        }
    }

    if (!footerKeywords.empty()) {
        std::vector<std::pair<Word, std::shared_ptr<Block>>> allWords;
        for (const auto& block : blocks) {
            for (const auto& para : block->paragraphs) {
                for (const auto& line : para.lines) {
                    for (const auto& word : line.words) {
                        allWords.emplace_back(word, block);
                    }
                }
            }
        }

        std::vector<std::vector<std::string>> footerWordsList;
        footerWordsList.reserve(footerKeywords.size());

        for (const auto& footerText : footerKeywords) {
            std::istringstream iss(footerText);
            std::vector<std::string> words;
            std::string word;
            while (iss >> word) {
                words.push_back(trim(word));
            }
            footerWordsList.push_back(std::move(words));
        }

        int cropY = -1;
        for (size_t i = 0; i < allWords.size(); ++i) {
            for (size_t footerIdx = 0; footerIdx < footerKeywords.size(); ++footerIdx) {
                const auto& footerWords = footerWordsList[footerIdx];

                if (i + footerWords.size() > allWords.size()) {
                    continue;
                }

                bool match = true;
                for (size_t j = 0; j < footerWords.size(); ++j) {
                    std::string ocrWord = trim(allWords[i + j].first.getFormattedText());

                    if (!normalizedEquals(ocrWord, footerWords[j])) {
                        match = false;
                        break;
                    }
                }

                if (match) {
                    const Word& firstWord = allWords[i].first;
                    cropY = firstWord.getY1();
                    break;
                }
            }
            if (cropY != -1) {
                break;
            }
        }

        if (cropY != -1) {
            crop(CropDirection::BOTTOM, cropY);
        }
    }

    if (!headerKeywords.empty()) {
        associateBlocksWithHeaders();

        for (auto& h : headers) {
            h->sortBlocks();
        }
    }
}

Page::~Page() {
    if (xmlDoc) {
        delete xmlDoc;
        xmlDoc = nullptr;
    }
}

void Page::extractTextElements(const std::string& altoXml) {
    xmlDoc = new tinyxml2::XMLDocument();

    if (xmlDoc->Parse(altoXml.c_str()) != tinyxml2::XML_SUCCESS) {
        delete xmlDoc;
        xmlDoc = nullptr;
        throw std::runtime_error("Failed to parse ALTO XML.");
    }

    tinyxml2::XMLElement* pageElem = xmlDoc->FirstChildElement("Page");
    if (!pageElem) {
        delete xmlDoc;
        xmlDoc = nullptr;
        throw std::runtime_error("ALTO XML does not contain a valid Page element.");
    }
    pageElem->QueryIntAttribute("WIDTH", &width);
    pageElem->QueryIntAttribute("HEIGHT", &height);

    tinyxml2::XMLElement* printSpace = pageElem->FirstChildElement("PrintSpace");
    if (!printSpace) {
        delete xmlDoc;
        xmlDoc = nullptr;
        throw std::runtime_error("ALTO XML does not contain a valid PrintSpace element.");
    }

    for (tinyxml2::XMLElement* composedBlock = printSpace->FirstChildElement("ComposedBlock");
        composedBlock != nullptr;
        composedBlock = composedBlock->NextSiblingElement("ComposedBlock")) {
        for (tinyxml2::XMLElement* textBlock = composedBlock->FirstChildElement("TextBlock");
            textBlock != nullptr;
            textBlock = textBlock->NextSiblingElement("TextBlock")) {
            try {
                blocks.emplace_back(std::make_shared<Block>(textBlock, this));
            }
            catch (...) {}
        }
    }
}

void Page::extractHeaders(const std::vector<std::string>& headerKeywords) {
    headers.clear();
    std::set<std::string> foundHeaders;

    std::vector<std::pair<Word, std::shared_ptr<Block>>> allWords;
    size_t totalWordCount = 0;

    for (const auto& block : blocks) {
        for (const auto& para : block->paragraphs) {
            for (const auto& line : para.lines) {
                totalWordCount += line.words.size();
            }
        }
    }
    allWords.reserve(totalWordCount);

    for (const auto& block : blocks) {
        for (const auto& para : block->paragraphs) {
            for (const auto& line : para.lines) {
                for (const auto& word : line.words) {
                    allWords.emplace_back(word, block);
                }
            }
        }
    }

    std::vector<std::vector<std::string>> headerWordsList;
    headerWordsList.reserve(headerKeywords.size());

    for (const auto& headerText : headerKeywords) {
        std::istringstream iss(headerText);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(trim(word));
        }
        headerWordsList.push_back(std::move(words));
    }

    for (size_t i = 0; i < allWords.size(); ++i) {
        for (size_t headerIdx = 0; headerIdx < headerKeywords.size(); ++headerIdx) {
            if (foundHeaders.find(headerKeywords[headerIdx]) != foundHeaders.end()) {
                continue;
            }

            const auto& headerWords = headerWordsList[headerIdx];

            if (i + headerWords.size() > allWords.size()) {
                continue;
            }

            bool match = true;
            for (size_t j = 0; j < headerWords.size(); ++j) {
                std::string ocrWord = trim(allWords[i + j].first.getFormattedText());

                if (!normalizedEquals(ocrWord, headerWords[j])) {
                    match = false;
                    break;
                }
            }

            if (match) {
                const Word& firstWord = allWords[i].first;
                const Word& lastWord = allWords[i + headerWords.size() - 1].first;

                tinyxml2::XMLElement* headerElement = xmlDoc->NewElement("Header");
                headerElement->SetAttribute("CONTENT", headerKeywords[headerIdx].c_str());
                headerElement->SetAttribute("HPOS", firstWord.getX1());
                headerElement->SetAttribute("VPOS", firstWord.getY1());
                headerElement->SetAttribute("WIDTH", lastWord.getX2() - firstWord.getX1());
                headerElement->SetAttribute("HEIGHT", firstWord.getHeight());

                headers.push_back(std::make_shared<Header>(headerElement, this));
                foundHeaders.insert(headerKeywords[headerIdx]);
            }
        }
    }
}

void Page::associateBlocksWithHeaders() {
    if (headers.empty()) return;

    std::sort(headers.begin(), headers.end(),
        [](const std::shared_ptr<Header>& a, const std::shared_ptr<Header>& b) {
            return a->getX1() < b->getX1();
        });

    std::vector<std::shared_ptr<Block>> blocksToProcess = blocks;

    for (size_t i = 0; i < headers.size(); ++i) {
        int xMin = headers[i]->getX1();
        int xMax = (i + 1 < headers.size()) ? headers[i + 1]->getX1() : width;
        headers[i]->clearBlocks();

        std::vector<std::shared_ptr<Block>> nextToProcess;
        for (const auto& block : blocksToProcess) {
            int bx1 = block->getX1();
            int bx2 = block->getX2();

            if (bx2 <= xMin || bx1 >= xMax) {
                nextToProcess.push_back(block);
                continue;
            }
            if (bx1 >= xMin && bx2 <= xMax) {
                headers[i]->addBlock(block);
                continue;
            }
            auto splitBlocks = block->splitAt(TextElement::SplitDirection::VERTICAL, xMax);
            if (splitBlocks.size() == 1) {
                nextToProcess.push_back(std::make_shared<Block>(splitBlocks[0]));
                continue;
            }
            for (const auto& split : splitBlocks) {
                int sx1 = split.getX1();
                int sx2 = split.getX2();
                if (sx1 >= xMin && sx2 <= xMax) {
                    headers[i]->addBlock(std::make_shared<Block>(split));
                }
                else {
                    nextToProcess.push_back(std::make_shared<Block>(split));
                }
            }
        }
        blocksToProcess = std::move(nextToProcess);
    }
}

const std::vector<std::shared_ptr<Block>>& Page::getBlocks() const {
    return blocks;
}

const std::vector<std::shared_ptr<Header>>& Page::getHeaders() const {
    return headers;
}

int Page::getWidth() const {
    return width;
}

int Page::getHeight() const {
    return height;
}

int Page::getIndex() const {
    return index;
}

void Page::crop(CropDirection cropDir, int boundary) {
    std::vector<std::shared_ptr<Block>> resultBlocks;
    bool isHorizontalSplit = (cropDir == CropDirection::LEFT || cropDir == CropDirection::RIGHT);
    auto shouldKeepBlock = [cropDir, boundary](const Block& block) {
        switch (cropDir) {
        case CropDirection::LEFT:   return block.getX1() >= boundary;
        case CropDirection::RIGHT:  return block.getX2() <= boundary;
        case CropDirection::TOP:    return block.getY1() >= boundary;
        case CropDirection::BOTTOM: return block.getY2() <= boundary;
        }
        return false;
        };

    for (const auto& block : blocks) {
        if (shouldKeepBlock(*block)) {
            resultBlocks.push_back(block);
        }
        else {
            bool needsSplit = false;

            switch (cropDir) {
            case CropDirection::LEFT:   needsSplit = block->getX2() > boundary; break;
            case CropDirection::RIGHT:  needsSplit = block->getX1() < boundary; break;
            case CropDirection::TOP:    needsSplit = block->getY2() > boundary; break;
            case CropDirection::BOTTOM: needsSplit = block->getY1() < boundary; break;
            }

            if (needsSplit) {
                TextElement::SplitDirection splitDir = isHorizontalSplit ?
                    TextElement::SplitDirection::VERTICAL :
                    TextElement::SplitDirection::HORIZONTAL;

                std::vector<Block> splitResults = block->splitAt(splitDir, boundary);

                for (const auto& splitBlock : splitResults) {
                    if (shouldKeepBlock(splitBlock)) {
                        resultBlocks.push_back(std::make_shared<Block>(splitBlock));
                    }
                }
            }
        }
    }

    blocks = std::move(resultBlocks);
}