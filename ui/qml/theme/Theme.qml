import QtQuick 2.15
pragma Singleton

QtObject {
    // Colors
    property color primary: "#1976D2"
    property color accent: "#03A9F4"
    property color background: "#fff8e1"
    property color surface: "#FFFFFF"
    property color textPrimary: "#213547"
    property color textMuted: "#6f7d89"

    // Spacing / radii / sizes
    property int spacingSmall: 6
    property int spacing: 12
    property int spacingLarge: 20
    property int radius: 2

    // Typography
    property string fontFamily: "Segoe UI"
    property int fontSize: 14
}
