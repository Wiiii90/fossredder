#pragma once
#include <string>
#include "models/TextElement.h"

class Word : public TextElement {
public:
    Word(tinyxml2::XMLElement* element, Page* page);
    Word(const Word& other);
    ~Word() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Word> splitAt(SplitDirection direction, int coordinate) const;

    //static Line merge(const std::vector<Line>& lines);

private:
    std::string rawXml;
};

