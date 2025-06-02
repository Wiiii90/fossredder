#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Word.h"

class Line : public TextElement {
public:
    Line(tinyxml2::XMLElement* element, Page* page);
    Line(tinyxml2::XMLElement* element);
    Line(const Line& other);
    ~Line() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Word> words;

    // New unified splitting method
    std::vector<Line> splitAt(SplitDirection direction, int coordinate) const;

    // Optional: For backward compatibility 
    [[deprecated("Use splitAt(SplitDirection::HORIZONTAL, y) instead")]]
    std::vector<Line> splitByYRecursive(int y) const;

    [[deprecated("Use splitAt(SplitDirection::VERTICAL, x) instead")]]
    std::vector<Line> splitByXRecursive(int x) const;

    void updateBoundingBox();

    static Line merge(const std::vector<Line>& lines);

private:
    std::string rawXml;
};