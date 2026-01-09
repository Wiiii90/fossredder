import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"
import "qrc:/qml/components/utils/FileUtils.js" as FileUtils

Item {
    id: root
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
                    text: qsTr("Import")
                    Layout.fillWidth: true
                    font.pointSize: 18
                }

                GroupBox {
                    id: pdfImporter
                    Layout.fillWidth: true
                    width: parent.width
                    title: qsTr("Import PDF")
                    visible: !(hasUiImport && uiImport.draft)

                    ColumnLayout {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 8

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
                                // avoid binding loop: keep text local and initialize from uiImport.selectedFile
                                text: ""
                                Component.onCompleted: {
                                    var def = "P:/.data/fossredder/Januar 2025.pdf"
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
                    }
                }

                Item { Layout.fillHeight: true }

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
                        // compute enabled from local flag and current field text without creating binding loops
                        enabled: (hasUiImport && !uiImport.isRunning) ? (fileField.text && fileField.text.length > 0) : false
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
