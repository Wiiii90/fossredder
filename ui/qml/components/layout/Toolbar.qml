import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: toolBar
    implicitHeight: 80
        property int iconRowHeight: Math.round(implicitHeight * 0.55)

    Rectangle {
        id: bg
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        color: "#f6fbfd"
        border.width: 1
        border.color: "#e6eef5"
        clip: true

        RowLayout {
            id: rootRow
            anchors.fill: parent
            spacing: 14
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

                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: fileIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/import.svg"; item.label = qsTr("Import"); item.clicked.connect(function(){ if (uiNav) uiNav.section = UiNavigation.Import }) } }
                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: fileIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/export.svg"; item.label = qsTr("Export"); item.clicked.connect(function(){ if (uiNav) uiNav.section = UiNavigation.Export }) } }
                }

                Text {
                    id: groupLabelFile
                    text: qsTr("File")
                    color: (typeof Theme !== 'undefined') ? Theme.textMuted : "#6f7d89"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: 1; Layout.fillHeight: true; color: "#d0e6f1"; Layout.alignment: Qt.AlignVCenter }

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

                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: domainIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/booking.svg"; item.label = qsTr("Booking"); item.clicked.connect(function(){ if (uiData) { uiData.selectedStatementId = ""; uiData.selectedTransactionId = ""; } if (uiNav) { uiNav.bookingView = UiNavigation.Statements; uiNav.section = UiNavigation.Booking; } }) } }
                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: domainIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/actor.svg"; item.label = qsTr("Actor"); item.clicked.connect(function(){ if (uiData) { uiData.selectedActorId = ""; uiData.selectedPropertyId = ""; uiData.selectedContractId = ""; } if (uiNav) uiNav.section = UiNavigation.Actors; }) } }
                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: domainIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/property.svg"; item.label = qsTr("Gebäude"); item.clicked.connect(function(){ if (uiData) { uiData.selectedActorId = ""; uiData.selectedPropertyId = ""; uiData.selectedContractId = ""; } if (uiNav) uiNav.section = UiNavigation.Properties; }) } }
                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: domainIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/contract.svg"; item.label = qsTr("Contract"); item.clicked.connect(function(){ if (uiData) { uiData.selectedActorId = ""; uiData.selectedPropertyId = ""; uiData.selectedContractId = ""; } if (uiNav) uiNav.section = UiNavigation.Contracts; }) } }
                }

                Text {
                    id: groupLabelDomain
                    text: qsTr("Domain")
                    color: (typeof Theme !== 'undefined') ? Theme.textMuted : "#6f7d89"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: 1; Layout.fillHeight: true; color: "#d0e6f1"; Layout.alignment: Qt.AlignVCenter }

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

                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: toolsIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/analysis.svg"; item.label = qsTr("Analysis"); item.clicked.connect(function(){ if (uiData) { uiData.selectedActorId = ""; uiData.selectedPropertyId = ""; uiData.selectedContractId = ""; } if (uiNav) uiNav.section = UiNavigation.Analysis; }) } }
                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: toolsIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/annual.svg"; item.label = qsTr("Annual"); item.clicked.connect(function(){ if (uiData) { uiData.selectedActorId = ""; uiData.selectedPropertyId = ""; uiData.selectedContractId = ""; } if (uiNav) uiNav.section = UiNavigation.Annual; }) } }
                }

                Text {
                    id: groupLabelTools
                    text: qsTr("Tools")
                    color: (typeof Theme !== 'undefined') ? Theme.textMuted : "#6f7d89"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle { width: 1; Layout.fillHeight: true; color: "#d0e6f1"; Layout.alignment: Qt.AlignVCenter }

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

                    Loader { source: "qrc:/qml/components/controls/AppIconButton.qml"; asynchronous: false; Layout.preferredWidth: 72; Layout.preferredHeight: appIcons.height; onLoaded: { item.svgSource = "qrc:/qml/assets/settings.svg"; item.label = qsTr("Settings"); } }
                }

                Text {
                    id: groupLabelApp
                    text: qsTr("Application")
                    color: (typeof Theme !== 'undefined') ? Theme.textMuted : "#6f7d89"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            // separator after Application group
            Rectangle { width: 1; Layout.fillHeight: true; color: "#d0e6f1"; Layout.alignment: Qt.AlignVCenter }

            Item { Layout.fillWidth: true }
        }
    }
}
