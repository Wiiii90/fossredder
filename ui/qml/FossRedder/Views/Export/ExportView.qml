import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import "../../Constants/Export.js" as ExportConfig

Item {
    id: root
    property var exportCtrl: (typeof exportController !== 'undefined' ? exportController : null)
    readonly property string defaultLocale: Qt.locale().name.replace("_", "-")
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pageMargin
        spacing: Theme.spacingMedium

        Label { text: qsTr("Export"); font.pointSize: Theme.fontSizeTitle + Theme.margins }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true

            Label { text: qsTr("Format:"); Layout.preferredWidth: Theme.formLabelWidth }
            Controls.ComboBox {
                id: formatBox
                model: [ qsTr("CSV"), qsTr("XLSX") ]
                Layout.preferredWidth: Theme.formFieldWidth
                onCurrentIndexChanged: {
                    if (pathField && (!pathField.text || pathField.text.length === 0 || pathField.text.indexOf(ExportConfig.fileNames.exportBaseName + ".") !== -1)) {
                        var base = (typeof fileSystemController !== 'undefined' && fileSystemController) ? fileSystemController.appDir() : ""
                        var ext = ExportConfig.extensionForFormat(currentIndex)
                        if (base && base.length > 0) pathField.text = base + "/" + ExportConfig.fileNames.exportBaseName + "." + ext
                    }
                }
            }
        }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true
            Label { text: qsTr("Include formulas (XLSX):"); Layout.preferredWidth: Theme.formFieldWidth }
            Controls.CheckBox { id: formulas; checked: true }
        }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true
            Label { text: qsTr("Locale:"); Layout.preferredWidth: Theme.formLabelWidth }
            Controls.TextField { id: localeField; text: root.defaultLocale; Layout.preferredWidth: Theme.formFieldWidth }
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true

            Label { text: qsTr("Save to:"); Layout.preferredWidth: Theme.formLabelWidth }
            Controls.TextField { id: pathField; placeholderText: qsTr("e.g. C:/Users/You/Documents/export.xlsx"); Layout.fillWidth: true }
            Controls.Button { text: qsTr("Browse..."); onClicked: if (uiActions) uiActions.browseExportFile() }
            Controls.Button {
                text: qsTr("Export")
                enabled: pathField.text.length > 0
                onClicked: {
                    var path = pathField.text
                    if (!path) return
                    if (exportCtrl) exportCtrl.exportData((formatBox.currentIndex === ExportConfig.formatIndexes.csv) ? ExportConfig.formatIndexes.csv : ExportConfig.formatIndexes.xlsx, path, formulas.checked, localeField.text)
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
