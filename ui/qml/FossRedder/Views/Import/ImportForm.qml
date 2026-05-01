import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme

    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        Label { text: qsTr("Source"); Layout.preferredWidth: root.theme.formLabelWidth }
        Controls.ComboBox {
            id: sourceKind
            model: Constants.FileFormats.supportedImportSourceLabels()
            currentIndex: 0
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Label { text: qsTr("Strategy"); Layout.preferredWidth: root.theme.formLabelWidth }
        Controls.ComboBox {
            id: strategy
            model: Constants.FileFormats.supportedStatementStrategyLabels()
            currentIndex: 0
        }
    }
}
