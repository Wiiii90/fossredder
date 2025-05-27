#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Word.h"

class Line : public TextElement {
public:
    Line(tinyxml2::XMLElement* element);
    ~Line() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Word> words;

    std::vector<Line> splitAtWord(size_t wordIdx) const;
    static Line mergeLines(const std::vector<Line>& lines);

    std::pair<Line, Line> splitByY(int y) const;
    std::pair<Line, Line> splitByX(int x) const;
    std::vector<Line> splitByXRecursive(int x) const;
    std::vector<Line> splitByYRecursive(int y) const;

    void updateBoundingBox();

private:
    std::string rawXml;
};