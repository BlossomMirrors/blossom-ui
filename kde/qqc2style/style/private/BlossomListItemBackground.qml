/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later

    Blossom list item background — mirrors the kstyle's
    drawPanelItemViewItemPrimitive (kde/kstyle/style/primitives/panels.cpp):
    selection is the highlight color at 18% alpha (28% while hovered) on a
    rounded rect, so Kirigami lists match Dolphin and other QtWidgets views.
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

Item {
    id: background

    property T.ItemDelegate control

    readonly property bool highlight: control.highlighted || control.down
    readonly property bool useAlternatingColors: {
        if (control.TableView.view?.alternatingRows && row % 2) {
            return true
        } else if (control.Kirigami.StyleHints.useAlternateBackgroundColor && index % 2) {
            return true
        }
        return false
    }

    readonly property color normalColor: useAlternatingColors ? Kirigami.Theme.alternateBackgroundColor : "transparent"
    // Workaround for QTBUG-113304
    readonly property bool reallyFocus: control.visualFocus || (control.activeFocus && control.focusReason === Qt.OtherFocusReason)

    property alias color: alternatingBackgroundRect.color
    property alias radius: selectionRect.radius

    Rectangle {
        id: alternatingBackgroundRect
        anchors {
            fill: parent
            leftMargin: Math.min(0, -background.control.leftInset)
            rightMargin: -background.control.rightInset
            bottomMargin: -background.control.bottomInset
        }
        color: background.normalColor
    }

    Rectangle {
        id: selectionRect
        anchors.fill: parent

        visible: background.control.enabled

        radius: 4

        color: {
            if (background.highlight) {
                return Qt.alpha(Kirigami.Theme.highlightColor,
                                background.control.hovered ? 0.28 : 0.18)
            }
            if (background.control.hovered || background.reallyFocus) {
                return Qt.alpha(Kirigami.Theme.textColor, 0.07)
            }
            return "transparent"
        }
    }
}
