#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Word.h"

class Line : public TextElement {
public:
    Line(tinyxml2::XMLElement* element, Page* page);
    Line(const Line& other);
    ~Line() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Word> words;

    std::vector<Line> splitAt(SplitDirection direction, int coordinate) const;

    void updateBoundingBox();

    static Line merge(const std::vector<Line>& lines);

private:
    std::string rawXml;
};