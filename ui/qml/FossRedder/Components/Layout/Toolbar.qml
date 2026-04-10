import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: toolBar
    implicitHeight: Theme.toolbarHeight
    property int iconRowHeight: Math.round(implicitHeight * 0.55)

    function clearDomainSelection() {
        if (!session) return
        session.selectedActorId = ""
        session.selectedPropertyId = ""
        session.selectedContractId = ""
    }

    function clearBookingStateForSection(section) {
        if (!navigation || section === Navigation.Booking) return
        navigation.bookingView = Navigation.Statements
    }

    function navigateTo(section, clearSelection, bookingView) {
        if (!navigation) return
        clearBookingStateForSection(section)
        if (clearSelection) clearDomainSelection()
        if (bookingView !== undefined) navigation.bookingView = bookingView
        navigation.section = section
    }

    Rectangle {
        id: bg
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        color: Theme.toolbarBackground
        border.width: Theme.borderWidthThin
        border.color: Theme.toolbarBorder
            clip: false

        RowLayout {
            id: rootRow
            anchors.fill: parent
            spacing: Theme.spacing + Theme.margins
            Layout.alignment: Qt.AlignVCenter

            ColumnLayout {
                id: fileGroup
                spacing: Theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: fileIcons
                    spacing: Theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: fileIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/import.svg"
                        label: qsTr("Import")
                        active: navigation && navigation.section === Navigation.Import
                        onClicked: navigateTo(Navigation.Import, false)
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: fileIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/export.svg"
                        label: qsTr("Export")
                        active: navigation && navigation.section === Navigation.Export
                        onClicked: navigateTo(Navigation.Export, false)
                    }
                }

                Text {
                    id: groupLabelFile
                    text: qsTr("File")
                    color: Theme.textMuted
                    font.pointSize: Theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: Theme.borderWidthThin; Layout.fillHeight: true; color: Theme.divider; Layout.alignment: Qt.AlignVCenter }

            ColumnLayout {
                id: domainGroup
                spacing: Theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: domainIcons
                    spacing: Theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/booking.svg"
                        label: qsTr("Booking")
                        active: navigation && navigation.section === Navigation.Booking
                        onClicked: {
                            if (session) {
                                session.selectedStatementId = ""
                                session.selectedTransactionId = ""
                            }
                            navigateTo(Navigation.Booking, false, Navigation.Statements)
                        }
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/actor.svg"
                        label: qsTr("Actor")
                        active: navigation && navigation.section === Navigation.Actors
                        onClicked: navigateTo(Navigation.Actors, true)
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/property.svg"
                        label: qsTr("Property")
                        active: navigation && navigation.section === Navigation.Properties
                        onClicked: navigateTo(Navigation.Properties, true)
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/contract.svg"
                        label: qsTr("Contract")
                        active: navigation && navigation.section === Navigation.Contracts
                        onClicked: navigateTo(Navigation.Contracts, true)
                    }
                }

                Text {
                    id: groupLabelDomain
                    text: qsTr("Domain")
                    color: Theme.textMuted
                    font.pointSize: Theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: Theme.borderWidthThin; Layout.fillHeight: true; color: Theme.divider; Layout.alignment: Qt.AlignVCenter }

            ColumnLayout {
                id: toolsGroup
                spacing: Theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: toolsIcons
                    spacing: Theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/analysis.svg"
                        label: qsTr("Analysis")
                        active: navigation && navigation.section === Navigation.Analysis
                        onClicked: navigateTo(Navigation.Analysis, true)
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/annual.svg"
                        label: qsTr("Annual")
                        active: navigation && navigation.section === Navigation.Annual
                        onClicked: navigateTo(Navigation.Annual, true)
                    }
                }

                Text {
                    id: groupLabelTools
                    text: qsTr("Tools")
                    color: Theme.textMuted
                    font.pointSize: Theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: Theme.borderWidthThin; Layout.fillHeight: true; color: Theme.divider; Layout.alignment: Qt.AlignVCenter }

            ColumnLayout {
                id: appGroup
                spacing: Theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: appIcons
                    spacing: Theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: Theme.toolbarIconButtonWidth
                        Layout.preferredHeight: appIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/settings.svg"
                        label: qsTr("Settings")
                        active: navigation && navigation.section === Navigation.Settings
                        onClicked: navigateTo(Navigation.Settings, false)
    }
                }

                Text {
                    id: groupLabelApp
                    text: qsTr("Application")
                    color: Theme.textMuted
                    font.pointSize: Theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: Theme.borderWidthThin; Layout.fillHeight: true; color: Theme.divider; Layout.alignment: Qt.AlignVCenter }

            Item { Layout.fillWidth: true }
        }
        Connections {
            target: navigation
            function onSectionChanged() {
                var s = navigation ? navigation.section : null;
                if (typeof loaderImport !== 'undefined' && loaderImport.item) loaderImport.item.active = (s === Navigation.Import);
                if (typeof loaderExport !== 'undefined' && loaderExport.item) loaderExport.item.active = (s === Navigation.Export);
                if (typeof loaderBooking !== 'undefined' && loaderBooking.item) loaderBooking.item.active = (s === Navigation.Booking);
                if (typeof loaderActor !== 'undefined' && loaderActor.item) loaderActor.item.active = (s === Navigation.Actors);
                if (typeof loaderProperty !== 'undefined' && loaderProperty.item) loaderProperty.item.active = (s === Navigation.Properties);
                if (typeof loaderContract !== 'undefined' && loaderContract.item) loaderContract.item.active = (s === Navigation.Contracts);
                if (typeof loaderAnalysis !== 'undefined' && loaderAnalysis.item) loaderAnalysis.item.active = (s === Navigation.Analysis);
                if (typeof loaderAnnual !== 'undefined' && loaderAnnual.item) loaderAnnual.item.active = (s === Navigation.Annual);
                if (typeof loaderSettings !== 'undefined' && loaderSettings.item) loaderSettings.item.active = (s === Navigation.Settings);
            }
        }
    }
}
