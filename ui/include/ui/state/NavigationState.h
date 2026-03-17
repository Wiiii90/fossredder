#pragma once

#include <QObject>

namespace ui {

class NavigationState : public QObject {
    Q_OBJECT
    Q_PROPERTY(Section section READ section WRITE setSection NOTIFY sectionChanged)
    Q_PROPERTY(BookingView bookingView READ bookingView WRITE setBookingView NOTIFY bookingViewChanged)

public:
    enum class Section {
        Actors,
        Properties,
        Contracts,
        Booking,
        Import,
        Export,
        Settings,
        Analysis,
        Annual
    };
    Q_ENUM(Section)

    enum class BookingView {
        Statements,
        Calendar,
        Transactions
    };
    Q_ENUM(BookingView)

    Q_PROPERTY(SettingsCategory settingsCategory READ settingsCategory WRITE setSettingsCategory NOTIFY settingsCategoryChanged)

    enum class SettingsCategory {
        General,
        Appearance,
        Import,
        Export,
        Storage,
        Notifications,
        Advanced,
        Updates
    };
    // Do not register SettingsCategory with Q_ENUM to avoid name clashes in QML

    explicit NavigationState(QObject* parent = nullptr);
    SettingsCategory settingsCategory() const noexcept { return settingsCategory_; }
    void setSettingsCategory(SettingsCategory c);
    
    Section section() const noexcept { return section_; }
    void setSection(Section s);

    BookingView bookingView() const noexcept { return bookingView_; }
    void setBookingView(BookingView v);

signals:
    void sectionChanged();
    void bookingViewChanged();
    void settingsCategoryChanged();

private:
    Section section_ = Section::Import;
    BookingView bookingView_ = BookingView::Statements;
    SettingsCategory settingsCategory_ = SettingsCategory::General;
    bool sectionChangeQueued_ = false;
    bool bookingViewChangeQueued_ = false;
    bool settingsCategoryChangeQueued_ = false;
};

}
