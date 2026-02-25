import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

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
                font.pointSize: 14
                Layout.fillWidth: true
                color: Theme.textPrimary
            }

            AppButton {
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

        Loader {
            id: exportRunLogLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "qrc:/qml/components/common/RunLogList.qml"
            onLoaded: { try { if (exportRunLogLoader.item) exportRunLogLoader.item.model = runsModel } catch(e) {} }
        }
    }

    function addRun(path, status, message) {
        var t = new Date().toLocaleString()
        runsModel.append({ time: t, path: path ? path : "", status: status ? status : "Unknown", message: message ? message : "" })
    }

    // Keep last running entry index so we can update it on finish
    property int _lastRunningIndex: -1

    Component.onCompleted: {
        // when export starts, uiExport.stateChanged will be emitted; detect running state
        try {
            if (typeof uiExport !== 'undefined' && uiExport) {
                // watch state changes
                uiExport.stateChanged.connect(function() {
                    try {
                        if (uiExport.isRunning) {
                            addRun("", "Running", "")
                            _lastRunningIndex = runsModel.count - 1
                        }
                    } catch(e) {}
                })

                // on finish, update last entry
                uiExport.exportFinished.connect(function(success) {
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
