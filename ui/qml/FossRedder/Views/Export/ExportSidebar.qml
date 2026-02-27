import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: Theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing
            Layout.alignment: Qt.AlignVCenter

            Label {
                text: qsTr("Export-Protokolle")
                font.pointSize: Theme.fontSizeLarge
                Layout.fillWidth: true
                color: Theme.textPrimary
            }

            Controls.Button {
                text: qsTr("Leeren")
                enabled: runsModel.count > 0
                implicitHeight: 32
                implicitWidth: 88
                fillColor: Theme.surface
                textColor: Theme.textPrimary
                onClicked: runsModel.clear()
            }
        }

        ListModel { id: runsModel }

        Components.RunLogList {
            id: exportRunLogList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: runsModel
        }
    }

    function addRun(path, status, message) {
        var t = new Date().toLocaleString()
        runsModel.append({ time: t, path: path ? path : "", status: status ? status : "Unknown", message: message ? message : "" })
    }

    property int _lastRunningIndex: -1

    Component.onCompleted: {
        try {
            if (typeof exportController !== 'undefined' && exportController) {
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
                        var status = success ? "Success" : "Failure"
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
