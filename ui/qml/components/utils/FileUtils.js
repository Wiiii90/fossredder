// Utility functions for file handling in QML
.pragma library

function toFileUrl(p) {
    if (!p || p.length === 0) return "";
    if (p.indexOf("file://") === 0) return p;
    var s = String(p).replace(/\\/g, "/");
    if (/^[A-Za-z]:\//.test(s)) return "file:///" + s;
    if (s.indexOf("//") === 0) return "file:" + s;
    return "file:///" + s;
}
