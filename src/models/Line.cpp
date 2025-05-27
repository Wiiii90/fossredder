#include "pch.h"
#include "models/Line.h"

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
    std::vector<Word> leftWords, rightWords;
    for (const auto& word : words) {
        if (word.getX1() <= x)
            leftWords.push_back(word);
        else
            rightWords.push_back(word);
    }
    std::vector<Line> result;
    if (!leftWords.empty()) {
        Line leftLine = *this;
        leftLine.words = leftWords;
        result.push_back(leftLine);
    }
    if (!rightWords.empty()) {
        Line rightLine = *this;
        rightLine.words = rightWords;
        result.push_back(rightLine);
    }
    return result;
}

std::vector<Line> Line::splitByYRecursive(int y) const {
    std::vector<Word> upperWords, lowerWords;
    for (const auto& word : words) {
        if (word.getY1() <= y)
            upperWords.push_back(word);
        else
            lowerWords.push_back(word);
    }
    std::vector<Line> result;
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