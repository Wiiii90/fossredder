#pragma once
#include <string>
#include "models/TextElement.h"

class Word : public TextElement {
public:
    Word(tinyxml2::XMLElement* element, Page* page);
    Word(tinyxml2::XMLElement* element);
    Word(const Word& other);
    ~Word() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    // New unified splitting method
    std::vector<Word> splitAt(SplitDirection direction, int coordinate) const;

    // Optional: For backward compatibility 
    [[deprecated("Use splitAt(SplitDirection::HORIZONTAL, y) instead")]]
    std::vector<Word> splitByYRecursive(int y) const;

    [[deprecated("Use splitAt(SplitDirection::VERTICAL, x) instead")]]
    std::vector<Word> splitByXRecursive(int x) const;

    //static Line merge(const std::vector<Line>& lines);

private:
    std::string rawXml;
};

