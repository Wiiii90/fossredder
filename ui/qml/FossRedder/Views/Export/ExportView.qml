import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    property var exportCtrl: (typeof exportController !== 'undefined' ? exportController : null)
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label { text: qsTr("Export"); font.pointSize: 18 }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Label { text: qsTr("Format:"); Layout.preferredWidth: 120 }
            Controls.ComboBox {
                id: formatBox
                model: [ qsTr("CSV"), qsTr("XLSX") ]
                Layout.preferredWidth: 200
                onCurrentIndexChanged: {
                    if (pathField && (!pathField.text || pathField.text.length === 0 || pathField.text.indexOf("export.") !== -1)) {
                        var base = (typeof fileSystemController !== 'undefined' && fileSystemController) ? fileSystemController.appDir() : ""
                        var ext = (currentIndex === 0) ? "csv" : "xlsx"
                        if (base && base.length > 0) pathField.text = base + "/export." + ext
                    }
                }
            }
        }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true
            Label { text: qsTr("Include formulas (XLSX):"); Layout.preferredWidth: 200 }
            Controls.CheckBox { id: formulas; checked: true }
        }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true
            Label { text: qsTr("Locale:"); Layout.preferredWidth: 120 }
            Controls.TextField { id: localeField; text: "de-DE"; Layout.preferredWidth: 200 }
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Label { text: qsTr("Save to:"); Layout.preferredWidth: 120 }
            Controls.TextField { id: pathField; text: ""; placeholderText: qsTr("e.g. C:/Users/You/Documents/export.xlsx"); Layout.fillWidth: true }
            Controls.Button { text: qsTr("Durchstöbern"); onClicked: if (uiActions) uiActions.browseExportFile() }
            Controls.Button {
                text: qsTr("Export")
                enabled: pathField.text.length > 0
                onClicked: {
                    var path = pathField.text
                    if (!path) return
                    if (exportCtrl) exportCtrl.exportData((formatBox.currentIndex === 0) ? 0 : 1, path, formulas.checked, localeField.text)
                }
            }
        }

        Connections {
            target: uiActions
            function onExportFileSelected(path) {
                if (!path) return
                pathField.text = path
            }
        }
    }
}
