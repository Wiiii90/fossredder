#include "pch.h"
#include "models/Paragraph.h"

Paragraph::Paragraph(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) throw std::invalid_argument("Null XML element passed to Paragraph constructor.");
    for (tinyxml2::XMLElement* lineElem = element->FirstChildElement("Line");
        lineElem != nullptr;
        lineElem = lineElem->NextSiblingElement("Line")) {
        lines.emplace_back(lineElem);
    }
}
Paragraph::~Paragraph() {}

std::string Paragraph::getRawText() const {
    std::string result;
    for (const auto& line : lines) {
        result += line.getRawText() + "\n";
    }
    return result;
}
std::string Paragraph::getFormattedText() const { return getRawText(); }

std::vector<Paragraph> Paragraph::splitAtLine(size_t lineIdx) const {
    std::vector<Paragraph> result;
    if (lineIdx >= lines.size()) return { *this };
    if (lineIdx > 0) {
        Paragraph first = *this;
        first.lines = std::vector<Line>(lines.begin(), lines.begin() + lineIdx);
        result.push_back(first);
    }
    if (lineIdx < lines.size()) {
        Paragraph second = *this;
        second.lines = std::vector<Line>(lines.begin() + lineIdx, lines.end());
        result.push_back(second);
    }
    return result;
}

Paragraph Paragraph::mergeParagraphs(const std::vector<Paragraph>& paragraphs) {
    if (paragraphs.empty()) throw std::invalid_argument("No paragraphs to merge.");
    Paragraph merged = paragraphs.front();
    merged.lines.clear();
    for (const auto& para : paragraphs) {
        merged.lines.insert(merged.lines.end(), para.lines.begin(), para.lines.end());
    }
    return merged;
}

std::pair<Paragraph, Paragraph> Paragraph::splitByY(int y) const {
    Paragraph upper = *this; upper.lines.clear();
    Paragraph lower = *this; lower.lines.clear();
    for (const auto& line : lines) {
        if (line.getY1() <= y)
            upper.lines.push_back(line);
        else
            lower.lines.push_back(line);
    }
    return { upper, lower };
}

std::pair<Paragraph, Paragraph> Paragraph::splitByX(int x) const {
    Paragraph left = *this; left.lines.clear();
    Paragraph right = *this; right.lines.clear();
    for (const auto& line : lines) {
        if (line.getX1() <= x)
            left.lines.push_back(line);
        else
            right.lines.push_back(line);
    }
    return { left, right };
}

std::vector<Paragraph> Paragraph::splitByXRecursive(int x) const {
    std::vector<Line> leftLines, rightLines;
    for (const auto& line : lines) {
        auto splitLines = line.splitByXRecursive(x);
        for (const auto& l : splitLines) {
            if (!l.words.empty() && l.words.front().getX1() <= x)
                leftLines.push_back(l);
            else
                rightLines.push_back(l);
        }
    }
    std::vector<Paragraph> result;
    if (!leftLines.empty()) {
        Paragraph left = *this;
        left.lines = leftLines;
        result.push_back(left);
    }
    if (!rightLines.empty()) {
        Paragraph right = *this;
        right.lines = rightLines;
        result.push_back(right);
    }
    return result;
}

std::vector<Paragraph> Paragraph::splitByYRecursive(int y) const {
    std::vector<Line> upperLines, lowerLines;
    for (const auto& line : lines) {
        auto splitLines = line.splitByYRecursive(y);
        for (const auto& l : splitLines) {
            if (!l.words.empty() && l.words.front().getY1() <= y)
                upperLines.push_back(l);
            else
                lowerLines.push_back(l);
        }
    }
    std::vector<Paragraph> result;
    if (!upperLines.empty()) {
        Paragraph upper = *this;
        upper.lines = upperLines;
        result.push_back(upper);
    }
    if (!lowerLines.empty()) {
        Paragraph lower = *this;
        lower.lines = lowerLines;
        result.push_back(lower);
    }
    return result;
}