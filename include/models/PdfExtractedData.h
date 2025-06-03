#pragma once
#include <string>
#include <vector>
#include <memory>

class Page;
class BookingGroup;

class PdfExtractedData {
public:
    PdfExtractedData(const std::string& sourceFile,
                     const std::vector<std::shared_ptr<Page>>& pages,
                     const std::vector<BookingGroup>& bookingGroups);

    const std::string& getSourceFile() const;
    const std::vector<std::shared_ptr<Page>>& getPages() const;
    const std::vector<BookingGroup>& getBookingGroups() const;

private:
    std::string sourceFile;
    std::vector<std::shared_ptr<Page>> pages;
    std::vector<BookingGroup> bookingGroups;
};