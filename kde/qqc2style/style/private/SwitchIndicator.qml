/*
    SPDX-FileCopyrightText: 2022 Tanbir Jishan <tantalising007@gmail.com>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later

    Blossom switch — mirrors Helper::renderSwitch in the BlossomUI kstyle
    (kde/kstyle/blossomuihelper.cpp) so QML switches match QtWidgets ones.
    The accent fill is drawn as a trail behind the thumb, so it follows the
    thumb during drags and toggle animations; the thumb overshoots its end
    position instead of a pressed-sink offset.
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

Item {
    id: indicator

    // scales with Kirigami.Units.gridUnit (like the Breeze QQC2 style's
    // switch) so it matches surrounding text/control size at any font size
    // or DPI; keeps the BlossomUI kstyle's 46:26:3 track/thumb proportions
    implicitHeight: Kirigami.Units.gridUnit
    implicitWidth: implicitHeight * 46 / 26
    layer.enabled: control.opacity < 1.0

    property T.AbstractButton control
    property alias handle: handle

    readonly property real thumbMargin: implicitHeight * 3 / 26
    readonly property bool highlighted: control.hovered || control.visualFocus
    // progress of the *animated* thumb (not visualPosition, which snaps on
    // toggle) so the accent trail and border follow drags AND animations
    readonly property real progress: Math.max(0, Math.min(1, (handle.x - thumbMargin) / handle.travel))

    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    Rectangle {
        id: track

        anchors.fill: parent
        radius: height / 2

        readonly property color offColor: indicator.highlighted && indicator.control.enabled
            ? Qt.lighter(Kirigami.Theme.backgroundColor, 1.05)
            : Kirigami.Theme.backgroundColor

        color: offColor
        border.width: 1
        // follow the thumb: gray border fades toward the accent border as the
        // accent fill is pulled across the track
        border.color: Kirigami.ColorUtils.linearInterpolation(
            Kirigami.ColorUtils.linearInterpolation(offColor, Kirigami.Theme.textColor,
                                                    indicator.highlighted ? 0.16 : 0.1),
            Kirigami.ColorUtils.linearInterpolation(fill.color, Kirigami.Theme.textColor,
                                                    indicator.highlighted ? 0.16 : 0.1),
            indicator.progress)

        Rectangle {
            id: fill

            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            radius: track.radius
            // accent trail drawn behind the thumb: grows with the thumb during
            // drags and toggle animations, covers the full track when on
            width: Math.min(track.width, handle.x + handle.width + indicator.thumbMargin)
            opacity: Math.min(1, indicator.progress * 12)

            color: indicator.highlighted && indicator.control.enabled
                ? Qt.lighter(Kirigami.Theme.highlightColor, 1.05)
                : Kirigami.Theme.highlightColor
        }

        Rectangle {
            id: handle

            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false

            readonly property real travel: track.width - width - 2 * indicator.thumbMargin

            x: indicator.thumbMargin
               + Math.max(0, Math.min(travel, indicator.control.visualPosition * travel))
            anchors.verticalCenter: parent.verticalCenter
            width: track.height - 2 * indicator.thumbMargin
            height: width
            radius: width / 2

            // always the theme's light color: Window background in light
            // themes (kstyle parity), text color in dark themes — keeps the
            // thumb visible on the accent track and on highlighted rows
            color: Kirigami.ColorUtils.brightnessForColor(Kirigami.Theme.backgroundColor) === Kirigami.ColorUtils.Light
                ? Kirigami.Theme.backgroundColor
                : Kirigami.Theme.textColor
            border.width: 1
            border.color: Kirigami.ColorUtils.linearInterpolation(color, Kirigami.Theme.textColor, 0.12)

            Behavior on x {
                enabled: !indicator.control.pressed && Kirigami.Units.shortDuration > 0
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 1.5
                    easing.type: Easing.OutBack
                }
            }
        }
    }
}
