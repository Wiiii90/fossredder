import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
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

                    Label { text: qsTr("Annual"); font.pointSize: root.theme.fontSizeTitle + root.theme.margins }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Selected Annual:") }
                        Label { text: root.session && root.session.selectedAnnual ? root.session.selectedAnnual.name : qsTr("(none)") }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Controls.Button { text: qsTr("New year"); onClicked: { annualStack.push(annualFormComp) } }
                        Item { Layout.fillWidth: true }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "transparent" }
                }
            }
        }
    }
}
