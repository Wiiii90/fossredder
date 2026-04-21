import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var exportCtrl: root.appContext ? root.appContext.exportController : null
    readonly property var actions: root.appContext ? root.appContext.actions : null
    readonly property var fileSystemController: root.appContext ? root.appContext.fileSystemController : null
    readonly property int csvContractValue: 0
    readonly property int xlsxContractValue: 1
    readonly property string defaultLocale: Qt.locale().name.replace("_", "-")
    readonly property var formatOptions: [
        {
            contract: csvContractValue,
            extension: Constants.FileFormats.exportFormats.csv.extension,
            label: Constants.FileFormats.exportFormats.csv.label
        },
        {
            contract: xlsxContractValue,
            extension: Constants.FileFormats.exportFormats.xlsx.extension,
            label: Constants.FileFormats.exportFormats.xlsx.label
        }
    ]
    readonly property var formatLabels: formatOptions.map(function(option) { return option.label })
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageMargin
        spacing: root.theme.spacingMedium

        Flickable {
            id: exportScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: exportContent.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: exportContent
                width: exportScroll.width
                spacing: root.theme.spacingMedium

                RowLayout {
                    spacing: root.theme.spacingMedium
                    Layout.fillWidth: true

                    Label { text: qsTr("Format:"); Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.ComboBox {
                        id: formatBox
                        objectName: "exportFormatBox"
                        model: root.formatLabels
                        Layout.preferredWidth: root.theme.formFieldWidth
                        onCurrentIndexChanged: {
                            if (pathField && (!pathField.text || pathField.text.length === 0 || pathField.text.indexOf(Constants.FileFormats.exportDefaults.baseName + ".") !== -1)) {
                                const base = root.fileSystemController ? root.fileSystemController.appDir() : ""
                                const selectedOption = root.formatOptions[currentIndex]
                                if (!selectedOption) return
                                const ext = selectedOption.extension
                                if (base && base.length > 0) pathField.text = base + "/" + Constants.FileFormats.exportDefaults.baseName + "." + ext
                            }
                        }
                    }
                }

                RowLayout {
                    spacing: root.theme.spacingMedium
                    Layout.fillWidth: true
                    Label { text: qsTr("Include formulas (XLSX):"); Layout.preferredWidth: root.theme.formFieldWidth }
                    Controls.CheckBox { id: formulas; objectName: "exportIncludeFormulasCheckBox"; checked: true }
                }

                RowLayout {
                    spacing: root.theme.spacingMedium
                    Layout.fillWidth: true
                    Label { text: qsTr("Locale:"); Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField { id: localeField; objectName: "exportLocaleField"; text: root.defaultLocale; Layout.preferredWidth: root.theme.formFieldWidth }
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Label { text: qsTr("Save to:"); Layout.preferredWidth: root.theme.formLabelWidth }
            Controls.TextField { id: pathField; objectName: "exportPathField"; placeholderText: qsTr("e.g. C:/Users/You/Documents/export.xlsx"); Layout.fillWidth: true }
            Controls.Button { objectName: "exportBrowseButton"; text: qsTr("Browse..."); onClicked: if (root.actions) root.actions.browseExportFile() }
            Controls.Button {
                objectName: "exportSubmitButton"
                text: qsTr("Export")
                enabled: pathField.text.length > 0
                onClicked: {
                    const path = pathField.text
                    if (!path) return
                    const selectedOption = root.formatOptions[formatBox.currentIndex]
                    if (!selectedOption) return
                    if (root.exportCtrl) root.exportCtrl.exportData(selectedOption.contract, path, formulas.checked, localeField.text)
                }
            }
        }

        Connections {
            target: root.actions
            function onExportFileSelected(path) {
                if (!path) return
                pathField.text = path
            }
        }
    }
}
