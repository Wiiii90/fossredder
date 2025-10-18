#include "pch.h"
#include "models/Paragraph.h"

Paragraph::Paragraph(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
	for (tinyxml2::XMLElement* lineElem = element->FirstChildElement("String");
		lineElem != nullptr;
		lineElem = lineElem->NextSiblingElement("String")) {
		lines.emplace_back(lineElem, page);
	}
	if (lines.empty()) {
		lines.emplace_back(element, page);
	}
}

Paragraph::Paragraph(const Paragraph& other)
	: TextElement(other),
	lines(other.lines),
	rawXml(other.rawXml)
{
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

std::vector<Paragraph> Paragraph::splitAt(SplitDirection direction, int coordinate) const {
	std::vector<Line> firstLines, secondLines;
	bool isVertical = (direction == SplitDirection::VERTICAL);

	for (const auto& line : lines) {
		auto splitLines = isVertical ?
			line.splitAt(SplitDirection::VERTICAL, coordinate) :
			line.splitAt(SplitDirection::HORIZONTAL, coordinate);

		if (splitLines.size() == 2) {
			firstLines.push_back(splitLines[0]);
			secondLines.push_back(splitLines[1]);
		}
		else if (splitLines.size() == 1) {
			if (isVertical) {
				if (splitLines[0].getX2() <= coordinate) {
					firstLines.push_back(splitLines[0]);
				}
				else {
					secondLines.push_back(splitLines[0]);
				}
			}
			else {
				if (!splitLines[0].words.empty() && splitLines[0].words.front().getY1() <= coordinate) {
					firstLines.push_back(splitLines[0]);
				}
				else {
					secondLines.push_back(splitLines[0]);
				}
			}
		}
	}

	std::vector<Paragraph> result;
	if (!firstLines.empty()) {
		Paragraph first = *this;
		first.lines = firstLines;
		first.updateBoundingBox();
		result.push_back(first);
	}
	if (!secondLines.empty()) {
		Paragraph second = *this;
		second.lines = secondLines;
		second.updateBoundingBox();
		result.push_back(second);
	}
	return result;
}

void Paragraph::updateBoundingBox() {
	if (lines.empty()) {
		x1 = x2 = y1 = y2 = width = height = 0;
		return;
	}
	x1 = lines.front().getX1();
	x2 = lines.front().getX2();
	y1 = lines.front().getY1();
	y2 = lines.front().getY2();
	for (const auto& l : lines) {
		if (l.getX1() < x1) x1 = l.getX1();
		if (l.getX2() > x2) x2 = l.getX2();
		if (l.getY1() < y1) y1 = l.getY1();
		if (l.getY2() > y2) y2 = l.getY2();
	}
	width = x2 - x1;
	height = y2 - y1;
}

Paragraph Paragraph::merge(const std::vector<Paragraph>& paragraphs) {
	if (paragraphs.empty()) throw std::invalid_argument("No paragraphs to merge.");
	Paragraph merged = paragraphs.front();
	merged.lines.clear();
	for (const auto& para : paragraphs) {
		merged.lines.insert(merged.lines.end(), para.lines.begin(), para.lines.end());
	}
	return merged;
}
