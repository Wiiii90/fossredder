.pragma library

function findObject(root, objectName) {
    if (!root)
        return null

    if (root.objectName === objectName)
        return root

    var specialChildren = [root.contentItem, root.background, root.indicator, root.popup]
    for (var i = 0; i < specialChildren.length; ++i) {
        var special = specialChildren[i]
        if (!special)
            continue
        var specialMatch = findObject(special, objectName)
        if (specialMatch)
            return specialMatch
    }

    if (!root.children)
        return null

    for (var childIndex = 0; childIndex < root.children.length; ++childIndex) {
        var match = findObject(root.children[childIndex], objectName)
        if (match)
            return match
    }

    return null
}
