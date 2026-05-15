/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsMiscellaneous.qml
 * @brief Provides miscellaneous settings toggles such as toolbar visibility options.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var appContext
    required property var theme
    readonly property var settingsViewModel: root.appContext ? root.appContext.settingsViewModel : null
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    contentWidth: width
    clip: true

    ColumnLayout {
        id: column
        anchors.fill: parent
        width: parent.width
        spacing: root.theme.viewFormSpacing

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Workspace preferences let you reduce toolbar noise without removing navigation from the app menu.")
                    color: root.theme.textPrimary
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        Layout.preferredWidth: root.theme.formLabelWidth
                        text: qsTr("Toolbar visibility")
                        color: root.theme.textPrimary
                    }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Hide entries here to declutter the toolbar. All views remain available through the View menu in the app menu.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: root.theme.spacingLarge
                    rowSpacing: root.theme.spacingSmall

                    Controls.CheckBox {
                        text: qsTr("Booking")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowBooking : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowBooking = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Import")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowImport : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowImport = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Actors")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowActors : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowActors = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Export")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowExport : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowExport = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Properties")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowProperties : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowProperties = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Analysis")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowAnalysis : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowAnalysis = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Contracts")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowContracts : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowContracts = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Annual")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowAnnual : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowAnnual = checked
                    }

                    Controls.CheckBox {
                        text: qsTr("Settings")
                        checked: root.settingsViewModel ? root.settingsViewModel.toolbarShowSettings : true
                        onToggled: if (root.settingsViewModel) root.settingsViewModel.toolbarShowSettings = checked
                    }
                }
            }
        }
    }
}
