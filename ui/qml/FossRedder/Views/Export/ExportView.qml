import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import "../../Constants/FileFormats.js" as FileFormats

Item {
    id: root
    property var exportCtrl: (typeof exportController !== 'undefined' ? exportController : null)
    readonly property int csvContractValue: (typeof UIContracts !== 'undefined' && UIContracts && UIContracts.Csv !== undefined ? UIContracts.Csv : 0)
    readonly property int xlsxContractValue: (typeof UIContracts !== 'undefined' && UIContracts && UIContracts.Xlsx !== undefined ? UIContracts.Xlsx : 1)
    readonly property string defaultLocale: Qt.locale().name.replace("_", "-")
    readonly property var formatOptions: [
        {
            contract: csvContractValue,
            extension: FileFormats.exportFormats.csv.extension,
            label: FileFormats.exportFormats.csv.label
        },
        {
            contract: xlsxContractValue,
            extension: FileFormats.exportFormats.xlsx.extension,
            label: FileFormats.exportFormats.xlsx.label
        }
    ]
    readonly property var formatLabels: formatOptions.map(function(option) { return option.label })
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
                objectName: "exportFormatBox"
                model: root.formatLabels
                Layout.preferredWidth: Theme.formFieldWidth
                onCurrentIndexChanged: {
                    if (pathField && (!pathField.text || pathField.text.length === 0 || pathField.text.indexOf(FileFormats.exportDefaults.baseName + ".") !== -1)) {
                        var base = (typeof fileSystemController !== 'undefined' && fileSystemController) ? fileSystemController.appDir() : ""
                        var selectedOption = root.formatOptions[currentIndex]
                        if (!selectedOption) return
                        var ext = selectedOption.extension
                        if (base && base.length > 0) pathField.text = base + "/" + FileFormats.exportDefaults.baseName + "." + ext
                    }
                }
            }
        }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true
            Label { text: qsTr("Include formulas (XLSX):"); Layout.preferredWidth: Theme.formFieldWidth }
            Controls.CheckBox { id: formulas; objectName: "exportIncludeFormulasCheckBox"; checked: true }
        }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true
            Label { text: qsTr("Locale:"); Layout.preferredWidth: Theme.formLabelWidth }
            Controls.TextField { id: localeField; objectName: "exportLocaleField"; text: root.defaultLocale; Layout.preferredWidth: Theme.formFieldWidth }
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            spacing: Theme.spacingMedium
            Layout.fillWidth: true

            Label { text: qsTr("Save to:"); Layout.preferredWidth: Theme.formLabelWidth }
            Controls.TextField { id: pathField; objectName: "exportPathField"; placeholderText: qsTr("e.g. C:/Users/You/Documents/export.xlsx"); Layout.fillWidth: true }
            Controls.Button { objectName: "exportBrowseButton"; text: qsTr("Browse..."); onClicked: if (actions) actions.browseExportFile() }
            Controls.Button {
                objectName: "exportSubmitButton"
                text: qsTr("Export")
                enabled: pathField.text.length > 0
                onClicked: {
                    var path = pathField.text
                    if (!path) return
                    var selectedOption = root.formatOptions[formatBox.currentIndex]
                    if (!selectedOption) return
                    if (exportCtrl) exportCtrl.exportData(selectedOption.contract, path, formulas.checked, localeField.text)
                }
            }
        }

        Connections {
            target: actions
            function onExportFileSelected(path) {
                if (!path) return
                pathField.text = path
            }
        }
    }
}
