// SPDX-License-Identifier: GPL-2.0-or-later
// Blossom override: kstyle-matching translucent selection background.

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.desktop as Desktop
import "private" as P
import "private/contrast.js" as Contrast

Desktop.ItemDelegate {
    id: control

    font.weight: control.highlighted || control.down
        ? Font.DemiBold : Font.Normal
    Kirigami.Theme.highlightedTextColor: Contrast.accentOnSelection(
        Kirigami.Theme.highlightColor, Kirigami.Theme.backgroundColor,
        control.hovered ? 0.28 : 0.18)
    background: P.BlossomListItemBackground {
        control: control
    }
}
