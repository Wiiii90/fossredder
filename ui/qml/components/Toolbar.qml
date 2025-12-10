import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: toolBar

    Rectangle {
        anchors.fill: parent
        color: "#f6fbfd"

        RowLayout {
            anchors.fill: parent
            spacing: 12

            RowLayout { spacing: 12; Layout.alignment: Qt.AlignVCenter
                Loader { source: "IconButton.qml"; asynchronous: false; onLoaded: { item.icon = "\u2191"; item.label = qsTr("Import"); item.width = 100; item.height = toolBar.implicitHeight - 12; item.clicked.connect(function(){ if (typeof actionImport !== 'undefined' && actionImport) actionImport.trigger(); }); } }
                Loader { source: "IconButton.qml"; asynchronous: false; onLoaded: { item.icon = "\u2193"; item.label = qsTr("Export"); item.width = 100; item.height = toolBar.implicitHeight - 12; item.clicked.connect(function(){ if (typeof actionExport !== 'undefined' && actionExport) actionExport.trigger(); }); } }
                Loader { source: "IconButton.qml"; asynchronous: false; onLoaded: { item.icon = "\u2699"; item.label = qsTr("Settings"); item.width = 96; item.height = toolBar.implicitHeight - 12; item.clicked.connect(function(){ /* settings */ }); } }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
