#include "pch.h"
#include "models/Line.h"
#include "models/Word.h"

Line::Line(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) throw std::invalid_argument("Null XML element passed to Line constructor.");
    for (tinyxml2::XMLElement* wordElem = element->FirstChildElement("String");
        wordElem != nullptr;
        wordElem = wordElem->NextSiblingElement("String")) {
        words.emplace_back(wordElem);
    }
}
Line::~Line() {}

std::string Line::getRawText() const {
    std::string result;
    for (const auto& word : words) {
        result += word.getRawText() + " ";
    }
    return result;
}
std::string Line::getFormattedText() const { return getRawText(); }

std::vector<Line> Line::splitAtWord(size_t wordIdx) const {
    std::vector<Line> result;
    if (wordIdx >= words.size()) return { *this };
    if (wordIdx > 0) {
        Line first = *this;
        first.words = std::vector<Word>(words.begin(), words.begin() + wordIdx);
        result.push_back(first);
    }
    if (wordIdx < words.size()) {
        Line second = *this;
        second.words = std::vector<Word>(words.begin() + wordIdx, words.end());
        result.push_back(second);
    }
    return result;
}

Line Line::mergeLines(const std::vector<Line>& lines) {
    if (lines.empty()) throw std::invalid_argument("No lines to merge.");
    Line merged = lines.front();
    merged.words.clear();
    for (const auto& line : lines) {
        merged.words.insert(merged.words.end(), line.words.begin(), line.words.end());
    }
    return merged;
}

std::pair<Line, Line> Line::splitByY(int y) const {
    Line upper = *this; upper.words.clear();
    Line lower = *this; lower.words.clear();
    for (const auto& word : words) {
        if (word.getY1() <= y)
            upper.words.push_back(word);
        else
            lower.words.push_back(word);
    }
    return { upper, lower };
}

std::pair<Line, Line> Line::splitByX(int x) const {
    Line left = *this; left.words.clear();
    Line right = *this; right.words.clear();
    for (const auto& word : words) {
        if (word.getX1() <= x)
            left.words.push_back(word);
        else
            right.words.push_back(word);
    }
    return { left, right };
}

std::vector<Line> Line::splitByXRecursive(int x) const {
    std::vector<Line> result;
    std::vector<Word> leftWords, rightWords;

    for (const auto& word : words) {
        int wx1 = word.getX1();
        int wx2 = word.getX2();
        if (wx2 <= x) {
            leftWords.push_back(word);
        } else if (wx1 >= x) {
            rightWords.push_back(word);
        } else {
            leftWords.push_back(word);
            rightWords.push_back(word);
        }
    }

    if (!leftWords.empty()) {
        Line leftLine = *this;
        leftLine.words = leftWords;
        leftLine.updateBoundingBox();
        result.push_back(leftLine);
    }
    if (!rightWords.empty()) {
        Line rightLine = *this;
        rightLine.words = rightWords;
        rightLine.updateBoundingBox();
        result.push_back(rightLine);
    }

    return result;
}

std::vector<Line> Line::splitByYRecursive(int y) const {
    std::vector<Line> result;
    std::vector<Word> upperWords, lowerWords;
    for (const auto& word : words) {
        int wy1 = word.getY1();
        int wy2 = word.getY2();
        if (wy2 <= y) {
            upperWords.push_back(word);
        } else if (wy1 >= y) {
            lowerWords.push_back(word);
        } else {
            upperWords.push_back(word);
            lowerWords.push_back(word);
        }
    }
    if (!upperWords.empty()) {
        Line upperLine = *this;
        upperLine.words = upperWords;
        result.push_back(upperLine);
    }
    if (!lowerWords.empty()) {
        Line lowerLine = *this;
        lowerLine.words = lowerWords;
        result.push_back(lowerLine);
    }
    return result;
}

void Line::updateBoundingBox() {
    if (words.empty()) {
        x1 = x2 = y1 = y2 = width = height = 0;
        return;
    }
    x1 = words.front().getX1();
    x2 = words.front().getX2();
    y1 = words.front().getY1();
    y2 = words.front().getY2();
    for (const auto& w : words) {
        if (w.getX1() < x1) x1 = w.getX1();
        if (w.getX2() > x2) x2 = w.getX2();
        if (w.getY1() < y1) y1 = w.getY1();
        if (w.getY2() > y2) y2 = w.getY2();
    }
    width = x2 - x1;
    height = y2 - y1;
}