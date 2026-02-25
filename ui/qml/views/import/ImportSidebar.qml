import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: Theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing
            Layout.alignment: Qt.AlignVCenter

            Label {
                text: qsTr("Import-Protokolle")
                font.pointSize: 14
                Layout.fillWidth: true
                color: Theme.textPrimary
            }

            AppButton {
                text: qsTr("Leeren")
                enabled: typeof uiImport !== 'undefined' && uiImport && uiImport.runs && uiImport.runs.count > 0
                implicitHeight: 32
                implicitWidth: 88
                // subtle secondary style: surface fill with primary text
                fillColor: Theme.surface
                textColor: Theme.textPrimary
                onClicked: if (typeof uiImport !== 'undefined' && uiImport && uiImport.runs) uiImport.runs.clear()
            }
        }

        ListView {
            id: runsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: (typeof uiImport !== 'undefined' && uiImport) ? uiImport.runs : null

            delegate: Rectangle {
                width: runsList.width
                color: "transparent"

                ColumnLayout {
                    id: content
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 4
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: time
                            font.pointSize: 10
                            opacity: 0.7
                            Layout.fillWidth: true
                            elide: Label.ElideRight
                        }

                        Label {
                            text: status
                            font.pointSize: 10
                            color: status === "Success" ? "#1b7f1b" : "#a11"
                        }

                        AppButton {
                            text: qsTr("Delete")
                            onClicked: {
                                if (runsList.model) runsList.model.removeAt(index)
                            }
                            implicitHeight: 28
                            implicitWidth: 80
                        }
                    }

                    Label {
                        text: type + ": " + file
                        Layout.fillWidth: true
                        elide: Label.ElideMiddle
                    }

                    Label {
                        visible: message && message.length > 0
                        text: message
                        wrapMode: Text.WordWrap
                        opacity: 0.8
                        Layout.fillWidth: true
                    }
                }

                implicitHeight: content.implicitHeight + 8
            }
        }
    }
}
