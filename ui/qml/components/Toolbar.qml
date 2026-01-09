import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: toolBar
    implicitHeight: 64

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolBar.implicitHeight
        color: "#f6fbfd"

        RowLayout {
            anchors.fill: parent
            spacing: 12
            Layout.alignment: Qt.AlignVCenter

            Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/import.svg";
                    item.label = qsTr("Import");
                    item.clicked.connect(function(){
                        if (uiNav) uiNav.section = UiNavigation.Import;
                    })
                }
            }
            /*Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/export.svg";
                    item.label = qsTr("Export");
                    item.clicked.connect(function(){
                        if (uiActions) uiActions.exportFile();
                    })
                }
            }*/
            Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/booking.svg";
                    item.label = qsTr("Booking");
                    item.clicked.connect(function(){
                        if (uiData) {
                            uiData.selectedStatementId = "";
                            uiData.selectedTransactionId = "";
                        }
                        if (uiNav) {
                            uiNav.bookingView = UiNavigation.Statements;
                            uiNav.section = UiNavigation.Booking;
                        }
                    })
                }
            }
            /*Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/actor.svg";
                    item.label = qsTr("Actor");
                    item.clicked.connect(function(){
                        if (uiData) {
                            uiData.selectedActorId = "";
                            uiData.selectedPropertyId = "";
                            uiData.selectedContractId = "";
                        }
                        if (uiNav) uiNav.section = UiNavigation.Actors;
                    })
                }
            }*/
            Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/property.svg";
                    item.label = qsTr("Property");
                    item.clicked.connect(function(){
                        if (uiData) {
                            uiData.selectedActorId = "";
                            uiData.selectedPropertyId = "";
                            uiData.selectedContractId = "";
                        }
                        if (uiNav) uiNav.section = UiNavigation.Properties;
                    })
                }
            }
            /*Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/contract.svg";
                    item.label = qsTr("Contract");
                    item.clicked.connect(function(){
                        if (uiData) {
                            uiData.selectedActorId = "";
                            uiData.selectedPropertyId = "";
                            uiData.selectedContractId = "";
                        }
                        if (uiNav) uiNav.section = UiNavigation.Contracts;
                    })
                }
            }*/
            /*Loader {
                source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight;
                onLoaded: {
                    item.svgSource = "qrc:/qml/assets/analysis.svg";
                    item.label = qsTr("Analysis");
                }
            }*/

            Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/annual.svg"; item.label = qsTr("Annual"); } }
            /*Loader { source: "IconButton.qml"; asynchronous: false; Layout.preferredWidth: 100; Layout.preferredHeight: toolBar.implicitHeight; onLoaded: { item.svgSource = "qrc:/qml/assets/settings.svg"; item.label = qsTr("Settings"); } }*/

            Item { Layout.fillWidth: true }
        }
    }
}
