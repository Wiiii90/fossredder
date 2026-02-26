import QtQuick 2.15
pragma Singleton

QtObject {
    property color primary: "#1976D2"
    property color accent: "#03A9F4"
    property color background: "#fff8e1"
    property color surface: "#FFFFFF"
    property color surfaceAlt: "#F6F7F9"
    property color border: "#D6DADF"
    property color textPrimary: "#213547"
    property color textMuted: "#6f7d89"

    property color danger: "#C62828"
    property color success: "#2E7D32"

    property int margins: 2
    property int spacing: 12
    property int spacingSmall: 6
    property int spacingLarge: 20
    property int radius: 4

    property string fontFamily: "Segoe UI"

    property int fontSizeSmall: 11
    property int fontSize: 12
    property int fontSizeLarge: 14
    property int fontSizeTitle: 16
}
