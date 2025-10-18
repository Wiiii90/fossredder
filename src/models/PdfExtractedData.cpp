#include "pch.h"
#include "models/PdfExtractedData.h"
#include "models/BookingGroup.h"
#include "models/Page.h"

PdfExtractedData::PdfExtractedData(const std::string& sourceFile,
	const std::vector<std::shared_ptr<Page>>& pages,
	const std::vector<BookingGroup>& bookingGroups)
	: sourceFile(sourceFile), pages(pages), bookingGroups(bookingGroups)
{
}

const std::string& PdfExtractedData::getSourceFile() const { return sourceFile; }
const std::vector<std::shared_ptr<Page>>& PdfExtractedData::getPages() const { return pages; }
const std::vector<BookingGroup>& PdfExtractedData::getBookingGroups() const { return bookingGroups; }