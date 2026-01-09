import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"
import "qrc:/qml/components/utils/FileUtils.js" as FileUtils

Item {
    id: root

    property bool hasUiImport: typeof uiImport !== 'undefined'
    property var draft: (hasUiImport && uiImport.draft) ? uiImport.draft : null

    // Use shared helper
    function toFileUrl(p) { return FileUtils.toFileUrl(p) }

    function currentProofUrl() {
        return toFileUrl(draft && draft.current ? (draft.current.proofImagePath || "") : "")
    }

    // Listen to uiImport changes and update local draft reference
    Connections {
        target: hasUiImport ? uiImport : null
        onStateChanged: {
            root.draft = (hasUiImport && uiImport.draft) ? uiImport.draft : null
            // Debugging aid (remove when fixed):
            // console.log("ImportView: uiImport.stateChanged -> draft:", root.draft)
        }
        onImportFinished: {
            root.draft = (hasUiImport && uiImport.draft) ? uiImport.draft : null
        }
        onImportFailed: {
            root.draft = null
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true

        ColumnLayout {
            id: pageLayout
            anchors.fill: parent
            spacing: Theme.spacing

            Label {
                text: qsTr("Import")
                font.pointSize: 18
            }

            GroupBox {
                id: pdfImporter
                title: qsTr("Import PDF")
                Layout.fillWidth: true
                visible: !draft

                ColumnLayout {

                    AppComboBox {
                        id: typeCombo
                        Layout.fillWidth: true
                        model: [ qsTr("Statement (PDF)") ]
                        currentIndex: 0
                        enabled: !(hasUiImport && uiImport.isRunning)
                    }

                    AppComboBox {
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

                        AppTextField {
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

                        AppButton {
                            text: qsTr("Browse")
                            enabled: !(hasUiImport && uiImport.isRunning)
                            onClicked: {
                                if (uiActions) uiActions.browseImportPdf()
                            }
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: hasUiImport ? uiImport.progress : 0
                            visible: hasUiImport && (uiImport.isRunning || uiImport.progress > 0)
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: hasUiImport ? (uiImport.error && uiImport.error.length > 0 ? uiImport.error : uiImport.phase) : ""
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        AppButton {
                            visible: hasUiImport && uiImport.isRunning
                            text: qsTr("Cancel")
                            enabled: hasUiImport && uiImport.isRunning
                            onClicked: {
                                if (hasUiImport) uiImport.cancelImport()
                            }
                        }

                        AppButton {
                            visible: hasUiImport && !uiImport.isRunning
                            text: qsTr("Start")
                            enabled: hasUiImport && !uiImport.isRunning && fileField.text.length > 0
                            onClicked: {
                                if (hasUiImport) uiImport.selectedFile = fileField.text
                                uiImport.startStatementImport()
                            }
                        }

                        AppButton {
                            visible: hasUiImport && !uiImport.isRunning
                            text: qsTr("Reset")
                            enabled: hasUiImport && !uiImport.isRunning
                            onClicked: uiImport.resetStatus()
                        }

                        BusyIndicator {
                            running: hasUiImport && uiImport.phase === "Stopping..."
                            visible: running
                            width: 24
                            height: 24
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            Loader {
                id: stmtLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                active: !!draft
                source: "StatementDraftView.qml"
                onLoaded: {
                    if (item) item.draft = draft
                }
                onActiveChanged: {
                    if (active && item) item.draft = draft
                }
            }
        }
    }
}