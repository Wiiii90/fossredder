import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"
import "qrc:/qml/components/utils/FileUtils.js" as FileUtils

Item {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    anchors.margins: 8

    property bool hasUiImport: typeof uiImport !== 'undefined'

    StackLayout {
        id: contentStack
        anchors.fill: parent
        currentIndex: (hasUiImport && uiImport.draft) ? 1 : 0

        // Import page
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: pageLayout.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            ColumnLayout {
                id: pageLayout
                width: parent.width
                spacing: 8

                Label {
                    text: qsTr("PDF-Bankauszug-Einleser")
                    Layout.fillWidth: true
                    font.pointSize: 18
                }

                GroupBox {
                    id: pdfImporter
                    Layout.fillWidth: true
                    width: parent.width
                    visible: !(hasUiImport && uiImport.draft)

                    ColumnLayout {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true

                            AppTextField {
                                id: fileField
                                Layout.fillWidth: true
                                placeholderText: qsTr("PDF Dateipfad")
                                enabled: !(hasUiImport && uiImport.isRunning)
                                // avoid binding loop: keep text local and initialize from uiImport.selectedFile
                                text: ""
                                Component.onCompleted: {
                                    var def = "P:/.data/fossredder/April 2025.pdf"
                                    if (hasUiImport) {
                                        if (uiImport.selectedFile && uiImport.selectedFile.length > 0) {
                                            text = uiImport.selectedFile
                                        } else {
                                            text = def
                                            uiImport.selectedFile = def
                                        }
                                    } else {
                                        text = def
                                    }
                                }

                                onTextChanged: {
                                    if (hasUiImport) {
                                        // update model but avoid reassigning identical value
                                        if (uiImport.selectedFile !== text) uiImport.selectedFile = text
                                    }
                                }
                            }

                            AppButton {
                                text: qsTr("Durchstöbern")
                                enabled: !(hasUiImport && uiImport.isRunning)
                                onClicked: {
                                    if (uiActions) uiActions.browseImportPdf()
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }

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
                    }
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true

                    AppButton {
                        visible: hasUiImport && uiImport.isRunning
                        text: qsTr("Abbrechen")
                        enabled: hasUiImport && uiImport.isRunning
                        onClicked: {
                            if (hasUiImport) uiImport.cancelImport()
                        }
                    }

                    AppButton {
                        visible: hasUiImport && !uiImport.isRunning
                        text: qsTr("Starten")
                        // compute enabled from local flag and current field text without creating binding loops
                        enabled: (hasUiImport && !uiImport.isRunning) ? (fileField.text && fileField.text.length > 0) : false
                        onClicked: {
                            if (hasUiImport) uiImport.selectedFile = fileField.text
                            uiImport.startStatementImport()
                        }
                    }

                    AppButton {
                        visible: hasUiImport && !uiImport.isRunning
                        text: qsTr("Zurücksetzen")
                        enabled: hasUiImport && !uiImport.isRunning
                        onClicked: uiImport.resetStatus()
                    }

                    BusyIndicator {
                        running: hasUiImport && uiImport.phase === "Anhalten..."
                        visible: running
                        width: 24
                        height: 24
                    }
                }
            }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: stmtLayout.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            ColumnLayout {
                id: stmtLayout
                width: parent.width

                StatementDraftView {
                    id: stmtView
                    Layout.fillWidth: true
                    draft: (hasUiImport ? uiImport.draft : null)
                }
            }
        }
    }
}
