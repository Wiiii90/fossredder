#include "pch.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Transaction.h"
#include "models/Page.h"

Header::Header(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
    if (element) {
        const char* content = element->Attribute("CONTENT");
        name = content ? content : "";
    }
}

Header::Header(tinyxml2::XMLElement* element)
    : TextElement(element) {
    if (element) {
        const char* content = element->Attribute("CONTENT");
        name = content ? content : "";
    }
}

Header::Header(const Header& other) : TextElement(other),
    name(other.name)
{}

Header::~Header() {}

void Header::addBlock(const std::shared_ptr<Block>& block) { blocks.push_back(block); }

const std::vector<std::shared_ptr<Block>>& Header::getBlocks() const { return blocks; }

void Header::clearBlocks() { blocks.clear(); }

void Header::sortBlocks() {
    std::sort(blocks.begin(), blocks.end(), [](const std::shared_ptr<Block>& a, const std::shared_ptr<Block>& b) {
        if (a->getY1() != b->getY1())
            return a->getY1() < b->getY1();
        return a->getX1() < b->getX1();
        });
}

const std::string& Header::getName() const { return name; }

std::string Header::getRawText() const { return name; }

std::string Header::getFormattedText() const { return name; }

void Header::sortHeaders(std::vector<Header>& headers) {
    std::sort(headers.begin(), headers.end(), [](const Header& a, const Header& b) {
        return a.getX1() < b.getX1();
    });
}