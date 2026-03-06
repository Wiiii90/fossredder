import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: toolBar
    implicitHeight: 80
        property int iconRowHeight: Math.round(implicitHeight * 0.55)
        

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
                spacing: 6
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: fileIcons
                    spacing: 12
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: fileIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/import.svg"
                        label: qsTr("Import")
                        active: uiNav && uiNav.section === UiNavigation.Import
                        onClicked: if (uiNav) uiNav.section = UiNavigation.Import
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: fileIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/export.svg"
                        label: qsTr("Export")
                        active: uiNav && uiNav.section === UiNavigation.Export
                        onClicked: if (uiNav) uiNav.section = UiNavigation.Export
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
                spacing: 6
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: domainIcons
                    spacing: 12
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/booking.svg"
                        label: qsTr("Booking")
                        active: uiNav && uiNav.section === UiNavigation.Booking
                        onClicked: {
                            if (uiData) {
                                uiData.selectedStatementId = ""
                                uiData.selectedTransactionId = ""
                            }
                            if (uiNav) {
                                uiNav.bookingView = UiNavigation.Statements
                                uiNav.section = UiNavigation.Booking
                            }
                        }
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/actor.svg"
                        label: qsTr("Actor")
                        active: uiNav && uiNav.section === UiNavigation.Actors
                        onClicked: {
                            if (uiData) {
                                uiData.selectedActorId = ""
                                uiData.selectedPropertyId = ""
                                uiData.selectedContractId = ""
                            }
                            if (uiNav) uiNav.section = UiNavigation.Actors
                        }
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/property.svg"
                        label: qsTr("Property")
                        active: uiNav && uiNav.section === UiNavigation.Properties
                        onClicked: {
                            if (uiData) {
                                uiData.selectedActorId = ""
                                uiData.selectedPropertyId = ""
                                uiData.selectedContractId = ""
                            }
                            if (uiNav) uiNav.section = UiNavigation.Properties
                        }
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: domainIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/contract.svg"
                        label: qsTr("Contract")
                        active: uiNav && uiNav.section === UiNavigation.Contracts
                        onClicked: {
                            if (uiData) {
                                uiData.selectedActorId = ""
                                uiData.selectedPropertyId = ""
                                uiData.selectedContractId = ""
                            }
                            if (uiNav) uiNav.section = UiNavigation.Contracts
                        }
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
                spacing: 6
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: toolsIcons
                    spacing: 12
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/analysis.svg"
                        label: qsTr("Analysis")
                        active: uiNav && uiNav.section === UiNavigation.Analysis
                        onClicked: {
                            if (uiData) {
                                uiData.selectedActorId = ""
                                uiData.selectedPropertyId = ""
                                uiData.selectedContractId = ""
                            }
                            if (uiNav) uiNav.section = UiNavigation.Analysis
                        }
                    }
                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/annual.svg"
                        label: qsTr("Annual")
                        active: uiNav && uiNav.section === UiNavigation.Annual
                        onClicked: {
                            if (uiData) {
                                uiData.selectedActorId = ""
                                uiData.selectedPropertyId = ""
                                uiData.selectedContractId = ""
                            }
                            if (uiNav) uiNav.section = UiNavigation.Annual
                        }
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
                spacing: 6
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: appIcons
                    spacing: 12
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: appIcons.height
                        svgSource: "qrc:/qml/FossRedder/Assets/settings.svg"
                        label: qsTr("Settings")
                        active: uiNav && uiNav.section === UiNavigation.Settings
                        onClicked: if (uiNav) uiNav.section = UiNavigation.Settings
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
            target: uiNav
            function onSectionChanged() {
                var s = uiNav ? uiNav.section : null;
                if (typeof loaderImport !== 'undefined' && loaderImport.item) loaderImport.item.active = (s === UiNavigation.Import);
                if (typeof loaderExport !== 'undefined' && loaderExport.item) loaderExport.item.active = (s === UiNavigation.Export);
                if (typeof loaderBooking !== 'undefined' && loaderBooking.item) loaderBooking.item.active = (s === UiNavigation.Booking);
                if (typeof loaderActor !== 'undefined' && loaderActor.item) loaderActor.item.active = (s === UiNavigation.Actors);
                if (typeof loaderProperty !== 'undefined' && loaderProperty.item) loaderProperty.item.active = (s === UiNavigation.Properties);
                if (typeof loaderContract !== 'undefined' && loaderContract.item) loaderContract.item.active = (s === UiNavigation.Contracts);
                if (typeof loaderAnalysis !== 'undefined' && loaderAnalysis.item) loaderAnalysis.item.active = (s === UiNavigation.Analysis);
                if (typeof loaderAnnual !== 'undefined' && loaderAnnual.item) loaderAnnual.item.active = (s === UiNavigation.Annual);
                if (typeof loaderSettings !== 'undefined' && loaderSettings.item) loaderSettings.item.active = (s === UiNavigation.Settings);
            }
        }
    }
}
