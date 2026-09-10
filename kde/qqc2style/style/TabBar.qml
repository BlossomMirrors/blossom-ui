

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import "private/contrast.js" as Contrast

T.TabBar {
    id: control

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    readonly property bool blossomSlidingSelection: true

    readonly property real pillRadius: Kirigami.Units.smallSpacing

    readonly property real uniformTabWidth: {
        let widest = 0;
        for (let i = 0; i < contentModel.count; ++i) {
            const tab = contentModel.get(i);
            if (tab) {
                widest = Math.max(widest, tab.implicitWidth);
            }
        }
        return widest;
    }

    readonly property real containerAlpha: 0.08
    readonly property real pillAlpha: 0.18

    readonly property color containerColor: Qt.alpha(Kirigami.Theme.textColor, containerAlpha)
    readonly property color pillColor: Qt.alpha(Kirigami.Theme.highlightColor, pillAlpha)

    readonly property color selectionSurface: Contrast.blend(
        Contrast.blend(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, containerAlpha),
        Kirigami.Theme.highlightColor, pillAlpha)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            Kirigami.Units.gridUnit * 6)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: 0
    padding: Kirigami.Units.smallSpacing

    contentItem: ListView {
        implicitWidth: contentWidth
        implicitHeight: control.contentModel.get(control.contentModel.count * 0)?.height ?? 0

        model: control.contentModel
        currentIndex: control.currentIndex

        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 40
        preferredHighlightEnd: width - 40

        highlight: Rectangle {
            radius: control.pillRadius
            color: control.pillColor
        }
        highlightMoveDuration: Kirigami.Units.longDuration
        highlightResizeDuration: Kirigami.Units.longDuration
    }

    background: Rectangle {
        radius: control.pillRadius + control.padding
        color: control.containerColor

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: wheel => {
                const delta = wheel.pixelDelta.y < 0 || wheel.angleDelta.y < 0 ? 1 : -1;
                for (let i = control.currentIndex + delta; i >= 0 && i < control.contentModel.count; i += delta) {
                    if (control.contentModel.get(i).enabled) {
                        control.currentIndex = i;
                        break;
                    }
                }
            }
        }
    }
}
