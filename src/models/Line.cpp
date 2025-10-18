#include "pch.h"
#include "models/Line.h"

Line::Line(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
	for (tinyxml2::XMLElement* wordElem = element->FirstChildElement("Word");
		wordElem != nullptr;
		wordElem = wordElem->NextSiblingElement("Word")) {
		words.emplace_back(wordElem, page);
	}
	if (words.empty()) {
		words.emplace_back(element, page);
	}
}

Line::Line(const Line& other)
	: TextElement(other),
	words(other.words),
	rawXml(other.rawXml)
{
}

Line::~Line() {}

std::string Line::getRawText() const {
	std::string result;
	for (const auto& word : words) {
		result += word.getRawText() + " ";
	}
	return result;
}

std::string Line::getFormattedText() const {
	return getRawText();
}

std::vector<Line> Line::splitAt(SplitDirection direction, int coordinate) const {
	std::vector<Word> firstWords, secondWords;
	bool isVertical = (direction == SplitDirection::VERTICAL);

	for (const auto& word : words) {
		auto splitWords = isVertical ?
			word.splitAt(SplitDirection::VERTICAL, coordinate) :
			word.splitAt(SplitDirection::HORIZONTAL, coordinate);

		if (splitWords.size() == 2) {
			firstWords.push_back(splitWords[0]);
			secondWords.push_back(splitWords[1]);
		}
		else if (splitWords.size() == 1) {
			if (isVertical) {
				if (splitWords[0].getX2() <= coordinate) {
					firstWords.push_back(splitWords[0]);
				}
				else {
					secondWords.push_back(splitWords[0]);
				}
			}
			else {
				if (splitWords[0].getY1() <= coordinate) {
					firstWords.push_back(splitWords[0]);
				}
				else {
					secondWords.push_back(splitWords[0]);
				}
			}
		}
	}

	std::vector<Line> result;
	if (!firstWords.empty()) {
		Line first = *this;
		first.words = firstWords;
		first.updateBoundingBox();
		result.push_back(first);
	}
	if (!secondWords.empty()) {
		Line second = *this;
		second.words = secondWords;
		second.updateBoundingBox();
		result.push_back(second);
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

Line Line::merge(const std::vector<Line>& lines) {
	if (lines.empty()) throw std::invalid_argument("No lines to merge.");
	Line merged = lines.front();
	merged.words.clear();
	for (const auto& line : lines) {
		merged.words.insert(merged.words.end(), line.words.begin(), line.words.end());
	}
	return merged;
}