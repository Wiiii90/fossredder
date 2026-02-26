import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Frame {
    id: root

    property int contentSpacing: Theme.spacingSmall

    padding: Theme.spacing

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.width: 1
        border.color: Theme.border
    }

    contentItem: ColumnLayout {
        id: body
        spacing: root.contentSpacing
    }

    default property alias content: body.data
}
