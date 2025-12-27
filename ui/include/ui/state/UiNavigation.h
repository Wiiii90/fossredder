#pragma once

#include <QObject>

class UiNavigation : public QObject {
    Q_OBJECT
    Q_PROPERTY(Section section READ section WRITE setSection NOTIFY sectionChanged)

public:
    enum class Section {
        Actors,
        Properties,
        Contracts
    };
    Q_ENUM(Section)

    explicit UiNavigation(QObject* parent = nullptr);

    Section section() const noexcept { return section_; }
    void setSection(Section s);

signals:
    void sectionChanged();

private:
    Section section_ = Section::Actors;
};
