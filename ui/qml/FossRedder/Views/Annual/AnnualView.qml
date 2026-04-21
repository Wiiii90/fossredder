import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    anchors.fill: parent
    StackView {
        id: annualStack
        anchors.fill: parent

        Component { id: annualFormComp; Views.AnnualForm { appContext: root.appContext; theme: root.theme; stackView: annualStack } }

        initialItem: Component {
            Item {
                anchors.fill: parent
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: root.theme.pageMargin
                    spacing: root.theme.spacingMedium

                    Flickable {
                        id: annualScroll
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: width
                        contentHeight: annualContent.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        ColumnLayout {
                            id: annualContent
                            width: annualScroll.width
                            spacing: root.theme.spacingMedium

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: qsTr("Selected Annual:") }
                                Label { text: root.session && root.session.selectedAnnual ? root.session.selectedAnnual.name : qsTr("(none)") }
                            }
                        }
                    }

                    Components.BottomBar {
                        Layout.fillWidth: true
                        theme: root.theme

                        Controls.Button { text: qsTr("Create"); onClicked: { annualStack.push(annualFormComp) } }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }
}
