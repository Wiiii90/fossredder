import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var appContext
    required property var theme
    property var stackView
    readonly property var annualController: root.appContext ? root.appContext.annualController : null
    readonly property var session: root.appContext ? root.appContext.session : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    anchors.fill: parent

    function createAnnual() {
        if (!root.annualController) return
        try {
            const year = parseInt(yearField.text)
            if (isNaN(year)) return
            if (typeof root.annualController.addAnnual === 'function') {
                const id = root.annualController.addAnnual(year)
                if (id && id.length > 0 && root.session) root.session.selectedAnnualId = id
            }
        } catch(e) {
        }
        if (root.stackView) root.stackView.pop()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageMargin
        spacing: root.theme.spacingMedium

        Flickable {
            id: annualFormScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: annualFormContent.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: annualFormContent
                width: annualFormScroll.width
                spacing: root.theme.spacingMedium

                Label { text: qsTr("New Year"); font.pointSize: root.theme.fontSizeTitle + root.theme.margins }

                Controls.TextField { id: yearField; placeholderText: qsTr("Year (e.g. 2025)") }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button { text: qsTr("Cancel"); onClicked: { if (root.stackView) root.stackView.pop() } }
            Item { Layout.fillWidth: true }
            Controls.Button {
                text: qsTr("Create")
                enabled: yearField.text.length > 0
                onClicked: root.createAnnual()
            }
        }
    }
}
