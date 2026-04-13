import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var exportController: root.appContext ? root.appContext.exportController : null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacing
            Layout.alignment: Qt.AlignVCenter

            Label {
                text: qsTr("Export-Protokolle")
                font.pointSize: root.theme.fontSizeLarge
                Layout.fillWidth: true
                color: root.theme.textPrimary
            }

            Controls.Button {
                text: qsTr("Leeren")
                enabled: runsModel.count > 0
                implicitHeight: 32
                implicitWidth: 88
                fillColor: root.theme.surface
                textColor: root.theme.textPrimary
                onClicked: runsModel.clear()
            }
        }

        ListModel { id: runsModel }

        Components.RunLogList {
            id: exportRunLogList
            theme: root.theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: runsModel
        }
    }

    function addRun(path, status, message) {
        const t = new Date().toLocaleString()
        runsModel.append({ time: t, path: path ? path : "", status: status ? status : "Unknown", message: message ? message : "" })
    }

    property int _lastRunningIndex: -1

    Component.onCompleted: {
        try {
            if (exportController) {
                exportController.stateChanged.connect(function() {
                    try {
                        if (exportController.isRunning) {
                            addRun("", "Running", "")
                            _lastRunningIndex = runsModel.count - 1
                        }
                    } catch(e) {}
                })

                exportController.exportFinished.connect(function(success) {
                    try {
                        const status = success ? "Success" : "Failure"
                        if (_lastRunningIndex >= 0 && _lastRunningIndex < runsModel.count) {
                            runsModel.set(_lastRunningIndex, { time: runsModel.get(_lastRunningIndex).time, path: runsModel.get(_lastRunningIndex).path, status: status, message: "" })
                        } else {
                            addRun("", status, "")
                        }
                        _lastRunningIndex = -1
                    } catch(e) {}
                })
            }
        } catch(e) {}
    }
}
