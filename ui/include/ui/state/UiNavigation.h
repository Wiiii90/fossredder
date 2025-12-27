#pragma once

#include <QObject>

class UiNavigation : public QObject {
    Q_OBJECT
    Q_PROPERTY(Section section READ section WRITE setSection NOTIFY sectionChanged)
    Q_PROPERTY(BookingView bookingView READ bookingView WRITE setBookingView NOTIFY bookingViewChanged)

public:
    enum class Section {
        Actors,
        Properties,
        Contracts,
        Booking
    };
    Q_ENUM(Section)

    enum class BookingView {
        Statements,
        Calendar,
        Transactions
    };
    Q_ENUM(BookingView)

    explicit UiNavigation(QObject* parent = nullptr);

    Section section() const noexcept { return section_; }
    void setSection(Section s);

    BookingView bookingView() const noexcept { return bookingView_; }
    void setBookingView(BookingView v);

signals:
    void sectionChanged();
    void bookingViewChanged();

private:
    Section section_ = Section::Actors;
    BookingView bookingView_ = BookingView::Statements;
};
