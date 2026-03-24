import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: Theme.spacingSmall

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingSmall

        Image {
            id: proofImage
            Layout.fillWidth: true
            Layout.preferredHeight: source.toString().length > 0 ? Math.max(180, paintedHeight > 0 ? paintedHeight : width * 0.6) : 0
            Layout.minimumHeight: 0
            visible: source.toString().length > 0
            fillMode: Image.PreserveAspectFit
            source: root.txRoot && root.txRoot.draft && root.txRoot.draft.current
                ? root.txRoot.proofSource(root.txRoot.draft.current.proofImagePath || "")
                : ""
            cache: true
            asynchronous: true
        }
    }
}
