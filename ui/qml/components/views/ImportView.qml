import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root

    property bool hasUiImport: typeof uiImport !== 'undefined'

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: qsTr("Import")
            font.pointSize: 18
        }

        ComboBox {
            id: typeCombo
            Layout.fillWidth: true
            model: [ qsTr("Statement (PDF)") ]
            currentIndex: 0
            enabled: !(hasUiImport && uiImport.isRunning)
        }

        ComboBox {
            id: profileCombo
            Layout.fillWidth: true
            model: hasUiImport ? uiImport.profiles : [ qsTr("Default") ]
            enabled: !(hasUiImport && uiImport.isRunning)
            Component.onCompleted: {
                if (hasUiImport) currentIndex = Math.max(0, model.indexOf(uiImport.selectedProfile))
            }
            onActivated: {
                if (hasUiImport) uiImport.selectedProfile = currentText
            }
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: fileField
                Layout.fillWidth: true
                placeholderText: qsTr("PDF file path")
                enabled: !(hasUiImport && uiImport.isRunning)
                text: hasUiImport ? (uiImport.selectedFile && uiImport.selectedFile.length > 0 ? uiImport.selectedFile : "P:/.data/fossredder/Januar 2025.pdf") : "P:/.data/fossredder/Januar 2025.pdf"
                Component.onCompleted: {
                    if (hasUiImport && (!uiImport.selectedFile || uiImport.selectedFile.length === 0)) uiImport.selectedFile = "P:/.data/fossredder/Januar 2025.pdf"
                }
                onTextChanged: {
                    if (hasUiImport) uiImport.selectedFile = text
                }
            }

            Button {
                text: qsTr("Browse")
                enabled: !(hasUiImport && uiImport.isRunning)
                onClicked: {
                    if (uiActions) uiActions.browseImportPdf()
                }
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            from: 0
            to: 1
            value: hasUiImport ? uiImport.progress : 0
            visible: hasUiImport && (uiImport.isRunning || uiImport.progress > 0)
        }

        Label {
            Layout.fillWidth: true
            text: hasUiImport ? (uiImport.error && uiImport.error.length > 0 ? uiImport.error : uiImport.phase) : ""
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Start")
                enabled: hasUiImport && !uiImport.isRunning && fileField.text.length > 0
                onClicked: {
                    if (hasUiImport) uiImport.selectedFile = fileField.text
                    uiImport.startStatementImport()
                }
            }

            Button {
                text: qsTr("Open Result")
                enabled: hasUiImport && uiImport.lastResultStatementId && uiImport.lastResultStatementId.length > 0
                onClicked: {
                    if (!uiNav || !uiData) return
                    uiNav.section = UiNavigation.Booking
                    uiNav.bookingView = UiNavigation.Statements
                    uiData.selectedStatementId = uiImport.lastResultStatementId
                    uiData.selectedTransactionId = ""
                }
            }

            Button {
                text: qsTr("Reset")
                enabled: hasUiImport && !uiImport.isRunning
                onClicked: uiImport.resetStatus()
            }

            Item { Layout.fillWidth: true }
        }

        Connections {
            target: uiActions
            function onImportFileSelected(path) {
                if (hasUiImport) uiImport.selectedFile = path
            }
        }
    }
}
