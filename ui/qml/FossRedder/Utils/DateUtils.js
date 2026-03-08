.pragma library

function monthNames() {
    return [
        qsTr("January"),
        qsTr("February"),
        qsTr("March"),
        qsTr("April"),
        qsTr("May"),
        qsTr("June"),
        qsTr("July"),
        qsTr("August"),
        qsTr("September"),
        qsTr("October"),
        qsTr("November"),
        qsTr("December")
    ]
}

function formatMonthLabel(monthValue) {
    if (!monthValue)
        return monthValue

    var parts = monthValue.split("-")
    if (parts.length >= 2 && parts[0].length === 4) {
        var year = parts[0]
        var month = parts[1]
        var monthIndex = parseInt(month, 10) - 1
        var names = monthNames()
        if (!isNaN(monthIndex) && monthIndex >= 0 && monthIndex < names.length)
            return names[monthIndex] + " " + year
    }

    return monthValue
}
