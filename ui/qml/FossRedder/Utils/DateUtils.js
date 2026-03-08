.pragma library

var monthNames = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"]

function formatMonthLabel(monthValue) {
    if (!monthValue)
        return monthValue

    var parts = monthValue.split("-")
    if (parts.length >= 2 && parts[0].length === 4) {
        var year = parts[0]
        var month = parts[1]
        var monthIndex = parseInt(month, 10) - 1
        if (!isNaN(monthIndex) && monthIndex >= 0 && monthIndex < monthNames.length)
            return monthNames[monthIndex] + " " + year
    }

    return monthValue
}
