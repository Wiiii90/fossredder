import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

    property var propertiesList: []
    property var contractTypes: []
    property var matrixRows: []

    function parseNumber(value) {
        const n = parseFloat(value)
        return isNaN(n) ? 0.0 : n
    }

    function propertyLabel(propertyId) {
        try {
            if (root.session && root.session.propertyName)
                return root.session.propertyName(propertyId)
        } catch (e) {
        }
        return propertyId
    }

    function matrixValue(row, contractType) {
        if (!row || !row.amountsByContract)
            return 0.0
        const value = row.amountsByContract[contractType]
        return value === undefined ? 0.0 : parseNumber(value)
    }

    function rebuildMatrix() {
        const result = root.session ? root.session.lastAnalysisResult : null
        const table = result && result.table ? result.table : []
        const transactions = result && result.transactions ? result.transactions : []

        const txContractSet = ({})
        const txPropertySet = ({})
        const txAmounts = ({})

        for (let t = 0; t < transactions.length; ++t) {
            const tx = transactions[t]
            if (!tx)
                continue

            const contractType = tx.contractType && String(tx.contractType).length > 0
                    ? String(tx.contractType)
                    : qsTr("Unassigned")
            const amount = parseNumber(tx.amount)
            const propertyIds = tx.propertyIds && tx.propertyIds.length > 0
                    ? tx.propertyIds
                    : []

            txContractSet[contractType] = true

            for (let p = 0; p < propertyIds.length; ++p) {
                const propertyId = String(propertyIds[p])
                if (propertyId.length === 0)
                    continue
                const propertyName = root.propertyLabel(propertyId)
                txPropertySet[propertyName] = true
                if (!txAmounts[propertyName])
                    txAmounts[propertyName] = ({})
                const previous = txAmounts[propertyName][contractType]
                        ? parseNumber(txAmounts[propertyName][contractType])
                        : 0.0
                txAmounts[propertyName][contractType] = previous + amount
            }
        }

        const txContracts = Object.keys(txContractSet).sort()
        const txProperties = Object.keys(txPropertySet).sort()
        if (txContracts.length > 0 && txProperties.length > 0) {
            const txRows = []
            for (let p = 0; p < txProperties.length; ++p) {
                const propertyName = txProperties[p]
                txRows.push({
                    propertyId: propertyName,
                    propertyName: propertyName,
                    amountsByContract: txAmounts[propertyName] ? txAmounts[propertyName] : ({})
                })
            }

            root.contractTypes = txContracts
            root.propertiesList = txProperties
            root.matrixRows = txRows
            return
        }

        const contractSet = ({})
        const propertySet = ({})
        const amounts = ({})

        for (let i = 0; i < table.length; ++i) {
            const row = table[i]
            if (!row || row.length < 2)
                continue

            let parsed = ({})
            try {
                parsed = JSON.parse(row[1])
            } catch (e) {
                continue
            }

            const byContract = parsed && parsed.byContract ? parsed.byContract : ({})
            const byProperty = parsed && parsed.byProperty ? parsed.byProperty : ({})

            let totalContractAmount = 0.0
            for (const contractType in byContract) {
                contractSet[contractType] = true
                totalContractAmount += parseNumber(byContract[contractType])
            }

            for (const propertyId in byProperty) {
                propertySet[propertyId] = true
                if (!amounts[propertyId])
                    amounts[propertyId] = ({})

                const propertyValue = byProperty[propertyId]
                if (propertyValue && typeof propertyValue === "object") {
                    for (const contractType in propertyValue) {
                        contractSet[contractType] = true
                        const previous = amounts[propertyId][contractType] ? parseNumber(amounts[propertyId][contractType]) : 0.0
                        amounts[propertyId][contractType] = previous + parseNumber(propertyValue[contractType])
                    }
                } else {
                    const propAmount = parseNumber(propertyValue)
                    if (totalContractAmount > 0.0) {
                        for (const contractType in byContract) {
                            const weight = parseNumber(byContract[contractType]) / totalContractAmount
                            const previous = amounts[propertyId][contractType] ? parseNumber(amounts[propertyId][contractType]) : 0.0
                            amounts[propertyId][contractType] = previous + propAmount * weight
                        }
                    }
                }
            }
        }

        const nextContracts = Object.keys(contractSet).sort()
        const nextProperties = Object.keys(propertySet).sort()
        const nextRows = []
        for (let p = 0; p < nextProperties.length; ++p) {
            const propertyId = nextProperties[p]
            nextRows.push({
                propertyId: propertyId,
                propertyName: root.propertyLabel(propertyId),
                amountsByContract: amounts[propertyId] ? amounts[propertyId] : ({})
            })
        }

        root.contractTypes = nextContracts
        root.propertiesList = nextProperties
        root.matrixRows = nextRows
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: matrixViewport
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: matrixContent.width
            contentHeight: matrixContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            ColumnLayout {
                id: matrixContent
                width: Math.max(matrixViewport.width,
                                root.theme.analysis.table.contractColumnWidth
                                + Math.max(1, root.contractTypes.length) * root.theme.analysis.table.amountColumnWidth)
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderStrong

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0
                        Label {
                            text: qsTr("Property")
                            Layout.preferredWidth: root.theme.analysis.table.contractColumnWidth
                            leftPadding: root.theme.spacingSmall
                            elide: Text.ElideRight
                        }
                        Repeater {
                            model: root.contractTypes
                            Label {
                                required property var modelData
                                text: modelData
                                Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Repeater {
                    model: root.matrixRows
                    Rectangle {
                        id: rowContainer
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.theme.analysis.table.rowHeight
                        color: index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt
                        border.width: root.theme.borderWidthThin
                        border.color: root.theme.borderLight

                        RowLayout {
                            anchors.fill: parent
                            spacing: 0
                            Label {
                                text: modelData.propertyName
                                Layout.preferredWidth: root.theme.analysis.table.contractColumnWidth
                                leftPadding: root.theme.spacingSmall
                                elide: Text.ElideRight
                            }
                            Repeater {
                                model: root.contractTypes
                                Label {
                                    required property var modelData
                                    text: root.matrixValue(rowContainer.modelData, modelData).toFixed(2)
                                    Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                    horizontalAlignment: Text.AlignRight
                                    rightPadding: root.theme.spacingSmall
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: root.session
        function onLastAnalysisResultChanged() { root.rebuildMatrix() }
    }

    Component.onCompleted: root.rebuildMatrix()
}
