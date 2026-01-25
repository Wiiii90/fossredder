import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    property var model

    ListView {
        anchors.fill: parent
        clip: true
        spacing: 6
        model: root.model

        delegate: Rectangle {
            width: ListView.view ? ListView.view.width : 200
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

                    Button {
                        text: qsTr("Delete")
                        onClicked: {
                            if (root.model) root.model.removeAt(index)
                        }
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
