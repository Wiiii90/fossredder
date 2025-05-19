#include "pch.h"
#include "models/Word.h"

Word::Word(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) throw std::invalid_argument("Null XML element passed to Word constructor.");
    const char* content = element->Attribute("CONTENT");
    rawXml = content ? content : "";
}
Word::~Word() {}

std::string Word::getRawText() const { return rawXml; }
std::string Word::getFormattedText() const { return rawXml; }
