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

Page::Page(const std::string& altoXml, int index) : index(index) {
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
                blocks.emplace_back(std::make_shared<Block>(textBlock));
            } catch (...) {}
        }
    }
}

Page::~Page() {
    if (xmlDoc) {
        delete xmlDoc;
        xmlDoc = nullptr;
    }
}

const std::vector<std::shared_ptr<Block>>& Page::getBlocks() const {
    return blocks;
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

std::vector<Header> Page::extractHeaders(const std::vector<std::string>& headerKeywords) const {
    std::vector<Header> headers;
    std::set<std::string> foundHeaders;
    
    for (const auto& block : blocks) {
        for (const auto& para : block->paragraphs) {
            for (const auto& line : para.lines) {
                const auto& words = line.words;
                for (const auto& headerText : headerKeywords) {
                    if (foundHeaders.find(headerText) != foundHeaders.end()) {
                        continue;
                    }
                    
                    std::istringstream iss(headerText);
                    std::vector<std::string> headerWords;
                    std::string word;
                    while (iss >> word) headerWords.push_back(word);
                    
                    for (size_t i = 0; i + headerWords.size() <= words.size(); ++i) {
                        bool match = true;
                        for (size_t j = 0; j < headerWords.size(); ++j) {
                            std::string ocrWord = trim(words[i + j].getFormattedText());
                            std::string headerWord = trim(headerWords[j]);
                            if (!normalizedEquals(ocrWord, headerWord)) {
                                match = false;
                                break;
                            }
                        }
                        
                        if (match) {
                            tinyxml2::XMLElement* headerElement = xmlDoc->NewElement("Header");
                            headerElement->SetAttribute("CONTENT", headerText.c_str());
                            headerElement->SetAttribute("HPOS", words[i].getX1());
                            headerElement->SetAttribute("VPOS", words[i].getY1());
                            headerElement->SetAttribute("WIDTH", 
                                words[i + headerWords.size() - 1].getX2() - words[i].getX1());
                            headerElement->SetAttribute("HEIGHT", words[i].getHeight());
                            
                            Header h(headerElement);
                            h.setPage(const_cast<Page*>(this));
                            headers.push_back(h);
                            
                            foundHeaders.insert(headerText);
                            std::cout << "[INFO] Found header: " << headerText << std::endl;
                            break;
                        }
                    }
                }
            }
        }
    }    
    return headers;
}

void Page::crop(CropDirection direction, int boundary) {
    std::vector<std::shared_ptr<Block>> resultBlocks;
    bool isHorizontalSplit = (direction == CropDirection::LEFT || direction == CropDirection::RIGHT);    
    auto shouldKeepBlock = [direction, boundary](const Block& block) {
        switch (direction) {
            case CropDirection::LEFT:   return block.getX1() >= boundary;
            case CropDirection::RIGHT:  return block.getX2() <= boundary;
            case CropDirection::TOP:    return block.getY1() >= boundary;
            case CropDirection::BOTTOM: return block.getY2() <= boundary;
        }
        return false;
    };
    
    std::cout << "[DEBUG] Cropping " 
              << (isHorizontalSplit ? "X" : "Y") << " at " << boundary 
              << ", direction: " 
              << (direction == CropDirection::LEFT ? "LEFT" :
                  direction == CropDirection::RIGHT ? "RIGHT" :
                  direction == CropDirection::TOP ? "TOP" : "BOTTOM")
              << std::endl;
    
    for (const auto& block : blocks) {
        if (shouldKeepBlock(*block)) {
            resultBlocks.push_back(block);
        } 
        else {
            bool needsSplit = false;
            
            switch (direction) {
                case CropDirection::LEFT:   needsSplit = block->getX2() > boundary; break;
                case CropDirection::RIGHT:  needsSplit = block->getX1() < boundary; break;
                case CropDirection::TOP:    needsSplit = block->getY2() > boundary; break;
                case CropDirection::BOTTOM: needsSplit = block->getY1() < boundary; break;
            }
            
            if (needsSplit) {
                std::vector<Block> splitResults = isHorizontalSplit ? 
                    block->splitByXRecursive(boundary) : 
                    block->splitByYRecursive(boundary);
                
                for (const auto& splitBlock : splitResults) {
                    if (shouldKeepBlock(splitBlock)) {
                        resultBlocks.push_back(std::make_shared<Block>(splitBlock));
                    }
                }
            }
        }
    }
    
    std::cout << "[DEBUG] After cropping: " << resultBlocks.size() 
              << " of " << blocks.size() << " blocks remain" << std::endl;
    
    blocks = std::move(resultBlocks);
}