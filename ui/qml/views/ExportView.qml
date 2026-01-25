import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"

Item {
    id: root
    property var exportController: uiExport

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        Label { text: qsTr("Export") ; font.pointSize: 18 }
        RowLayout {
            spacing: 8
            Label { text: qsTr("Format:"); Layout.preferredWidth: 120 }
            ComboBox {
                id: formatBox
                model: [ "CSV", "XLSX" ]
                Layout.preferredWidth: 200
                onCurrentIndexChanged: {
                    if (pathField && (!pathField.text || pathField.text.length === 0 || pathField.text.indexOf("export.") !== -1)) {
                        var base = (typeof uiFileSystem !== 'undefined' && uiFileSystem) ? uiFileSystem.appDir() : ""
                        var ext = (currentIndex === 0) ? "csv" : "xlsx"
                        if (base && base.length > 0) pathField.text = base + "/export." + ext
                    }
                }
            }
        }
        RowLayout {
            spacing: 8
            Label { text: qsTr("Include formulas (XLSX):"); Layout.preferredWidth: 200 }
            CheckBox { id: formulas; checked: true }
        }
        RowLayout {
            spacing: 8
            Label { text: qsTr("Locale:"); Layout.preferredWidth: 120 }
            TextField { id: localeField; text: "de-DE"; Layout.preferredWidth: 200 }
        }
    }

    RowLayout {
        spacing: 8
        Label { text: qsTr("Save to:"); Layout.preferredWidth: 120 }
        AppTextField { id: pathField; text: ""; placeholderText: qsTr("e.g. C:/Users/You/Documents/export.xlsx"); Layout.fillWidth: true }
        AppButton { text: qsTr("Durchstöbern"); onClicked: picker.open(pathField.text || "") }
        Button {
            text: qsTr("Export")
            enabled: pathField.text.length > 0
            onClicked: {
                var path = pathField.text
                if (!path) return
                if (exportController) exportController.exportData((formatBox.currentIndex === 0) ? 0 : 1, path, formulas.checked, localeField.text)
            }
        }
        Component.onCompleted: {
            if (!pathField.text || pathField.text.length === 0) {
                var base = (typeof uiFileSystem !== 'undefined' && uiFileSystem) ? uiFileSystem.appDir() : ""
                var ext = (formatBox && formatBox.currentIndex === 0) ? "csv" : "xlsx"
                if (base && base.length > 0) pathField.text = base + "/export." + ext
            }
        }
    }

    AppFilePicker { id: picker; onAccepted: function(p) { pathField.text = p } }
}
