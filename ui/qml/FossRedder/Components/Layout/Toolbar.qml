/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Components/Layout/Toolbar.qml
 * @brief Provides the Toolbar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: toolBar
    required property var appContext
    required property var theme
    readonly property var navigation: toolBar.appContext ? toolBar.appContext.navigation : null
    readonly property var session: toolBar.appContext ? toolBar.appContext.session : null
    readonly property var settingsController: toolBar.appContext ? toolBar.appContext.settingsController : null
    readonly property int navActors: 0
    readonly property int navProperties: 1
    readonly property int navContracts: 2
    readonly property int navBooking: 3
    readonly property int navImport: 4
    readonly property int navExport: 5
    readonly property int navSettings: 6
    readonly property int navAnalysis: 7
    readonly property int navAnnual: 8
    readonly property int navStatements: 0
    implicitHeight: toolBar.theme.toolbarHeight
    property int iconRowHeight: Math.round(implicitHeight * 0.55)

    function assetUrl(fileName) {
        return Qt.resolvedUrl("../../Assets/" + fileName)
    }

    function clearDomainSelection() {
        if (!toolBar.session) return
        toolBar.session.selectedActorId = ""
        toolBar.session.selectedPropertyId = ""
        toolBar.session.selectedContractId = ""
        toolBar.session.selectedAnalysisId = ""
        toolBar.session.selectedAnnualId = ""
        toolBar.session.selectedStatementId = ""
        toolBar.session.selectedTransactionId = ""
    }

    function clearBookingStateForSection(section) {
        if (!toolBar.navigation || section === toolBar.navBooking) return
        toolBar.navigation.setBookingViewValue(toolBar.navStatements)
    }

    function navigateTo(section, clearSelection, bookingView) {
        if (!toolBar.navigation) return
        toolBar.clearBookingStateForSection(section)
        if (clearSelection) toolBar.clearDomainSelection()
        if (bookingView !== undefined) toolBar.navigation.setBookingViewValue(bookingView)
        toolBar.navigation.setSectionValue(section)
    }

    function showDividerForGroup(visibleGroup, hasVisibleAfter) {
        return visibleGroup && hasVisibleAfter
    }

    Rectangle {
        id: bg
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        color: toolBar.theme.toolbarBackground
        border.width: toolBar.theme.borderWidthThin
        border.color: toolBar.theme.toolbarBorder
            clip: false

        RowLayout {
            id: rootRow
            anchors.fill: parent
            spacing: toolBar.theme.spacing + toolBar.theme.margins
            Layout.alignment: Qt.AlignVCenter

            ColumnLayout {
                id: fileGroup
                visible: (toolBar.settingsController ? toolBar.settingsController.toolbarShowImport : true)
                         || (toolBar.settingsController ? toolBar.settingsController.toolbarShowExport : true)
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: fileIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowImport : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: fileIcons.height
                        svgSource: toolBar.assetUrl("import.svg")
                        label: qsTr("Import")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navImport
                        onClicked: toolBar.navigateTo(toolBar.navImport, false)
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowExport : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: fileIcons.height
                        svgSource: toolBar.assetUrl("export.svg")
                        label: qsTr("Export")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navExport
                        onClicked: toolBar.navigateTo(toolBar.navExport, false)
                    }
                }

                Text {
                    id: groupLabelFile
                    text: qsTr("File")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                width: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: toolBar.showDividerForGroup(fileGroup.visible,
                    domainGroup.visible || toolsGroup.visible || appGroup.visible)
            }

            ColumnLayout {
                id: domainGroup
                visible: (toolBar.settingsController ? toolBar.settingsController.toolbarShowBooking : true)
                         || (toolBar.settingsController ? toolBar.settingsController.toolbarShowActors : true)
                         || (toolBar.settingsController ? toolBar.settingsController.toolbarShowProperties : true)
                         || (toolBar.settingsController ? toolBar.settingsController.toolbarShowContracts : true)
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: domainIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowBooking : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("booking.svg")
                        label: qsTr("Booking")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navBooking
                        onClicked: {
                            if (toolBar.session) {
                                toolBar.session.selectedStatementId = ""
                                toolBar.session.selectedTransactionId = ""
                            }
                            toolBar.navigateTo(toolBar.navBooking, false, toolBar.navStatements)
                        }
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowActors : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("actor.svg")
                        label: qsTr("Actor")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navActors
                        onClicked: toolBar.navigateTo(toolBar.navActors, true)
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowProperties : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("property.svg")
                        label: qsTr("Property")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navProperties
                        onClicked: toolBar.navigateTo(toolBar.navProperties, true)
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowContracts : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: domainIcons.height
                        svgSource: toolBar.assetUrl("contract.svg")
                        label: qsTr("Contract")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navContracts
                        onClicked: toolBar.navigateTo(toolBar.navContracts, true)
                    }
                }

                Text {
                    id: groupLabelDomain
                    text: qsTr("Domain")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                width: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: toolBar.showDividerForGroup(domainGroup.visible,
                    toolsGroup.visible || appGroup.visible)
            }

            ColumnLayout {
                id: toolsGroup
                visible: (toolBar.settingsController ? toolBar.settingsController.toolbarShowAnalysis : true)
                         || (toolBar.settingsController ? toolBar.settingsController.toolbarShowAnnual : true)
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: toolsIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowAnalysis : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: toolBar.assetUrl("analysis.svg")
                        label: qsTr("Analysis")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navAnalysis
                        onClicked: {
                            if (toolBar.session)
                                toolBar.session.selectedAnalysisId = ""
                            toolBar.navigateTo(toolBar.navAnalysis, true)
                        }
                    }
                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowAnnual : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: toolsIcons.height
                        svgSource: toolBar.assetUrl("annual.svg")
                        label: qsTr("Annual")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navAnnual
                        onClicked: toolBar.navigateTo(toolBar.navAnnual, true)
                    }
                }

                Text {
                    id: groupLabelTools
                    text: qsTr("Tools")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                width: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: toolBar.showDividerForGroup(toolsGroup.visible, appGroup.visible)
            }

            ColumnLayout {
                id: appGroup
                visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowSettings : true
                spacing: toolBar.theme.toolbarSectionSpacing
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: toolBar.implicitHeight
                Layout.maximumHeight: toolBar.implicitHeight

                RowLayout {
                    id: appIcons
                    spacing: toolBar.theme.toolbarGroupSpacing
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    height: toolBar.iconRowHeight

                    Controls.IconButton {
                        visible: toolBar.settingsController ? toolBar.settingsController.toolbarShowSettings : true
                        Layout.preferredWidth: toolBar.theme.toolbarIconButtonWidth
                        Layout.preferredHeight: appIcons.height
                        svgSource: toolBar.assetUrl("settings.svg")
                        label: qsTr("Settings")
                        active: toolBar.navigation && toolBar.navigation.sectionValue === toolBar.navSettings
                        onClicked: toolBar.navigateTo(toolBar.navSettings, false)
    }
                }

                Text {
                    id: groupLabelApp
                    text: qsTr("Application")
                    color: toolBar.theme.textMuted
                    font.pointSize: toolBar.theme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    elide: Text.ElideRight
                    visible: true
                }
            }

            Rectangle {
                width: toolBar.theme.borderWidthThin
                Layout.fillHeight: true
                color: toolBar.theme.divider
                Layout.alignment: Qt.AlignVCenter
                visible: appGroup.visible
            }

            Item { Layout.fillWidth: true }
        }
    }
}
