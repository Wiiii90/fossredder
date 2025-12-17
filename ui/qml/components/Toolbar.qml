import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: toolBar
    implicitHeight: 64

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        color: "#f6fbfd"

        RowLayout {
            anchors.fill: parent
            spacing: 12
            Layout.alignment: Qt.AlignVCenter

            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/import.svg"; item.label = qsTr("Import"); item.clicked.connect(function(){ if (typeof actionImport !== 'undefined' && actionImport) actionImport.trigger(); }) } }
            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/export.svg"; item.label = qsTr("Export"); item.clicked.connect(function(){ if (typeof actionExport !== 'undefined' && actionExport) actionExport.trigger(); }) } }
            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/actor.svg"; item.label = qsTr("Actor"); item.clicked.connect(function(){ if (typeof actionActor !== 'undefined' && actionActor) actionActor.trigger(); }) } }
            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/property.svg"; item.label = qsTr("Property"); item.clicked.connect(function(){ if (typeof actionProperty !== 'undefined' && actionProperty) actionProperty.trigger(); }) } }
            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/analysis.svg"; item.label = qsTr("Analysis"); item.clicked.connect(function(){ if (typeof actionAnalysis !== 'undefined' && actionAnalysis) actionAnalysis.trigger(); }) } }
            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/annual.svg"; item.label = qsTr("Annual"); item.clicked.connect(function(){ if (typeof actionAnnual !== 'undefined' && actionAnnual) actionAnnual.trigger(); }) } }
            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/settings.svg"; item.label = qsTr("Settings"); item.clicked.connect(function(){ if (typeof actionSettings !== 'undefined' && actionSettings) actionSettings.trigger(); }) } }

            Item { Layout.fillWidth: true }
        }
    }
}
