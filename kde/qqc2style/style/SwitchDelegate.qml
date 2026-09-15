// SPDX-License-Identifier: GPL-2.0-or-later
// Blossom override: Blossom switch indicator + kstyle-matching translucent
// selection background.

import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.desktop as Desktop
import "private" as P
import "private/contrast.js" as Contrast

Desktop.SwitchDelegate {
    id: control

    font.weight: control.highlighted || control.down
        ? Font.DemiBold : Font.Normal
    Kirigami.Theme.highlightedTextColor: Contrast.accentOnSelection(
        Kirigami.Theme.highlightColor, Kirigami.Theme.backgroundColor,
        control.hovered ? 0.28 : 0.18)

    indicator: P.SwitchIndicator {
        x: !control.mirrored ? control.horizontalPadding : control.width - width - control.horizontalPadding
        y: control.topPadding + (control.display === T.AbstractButton.TextUnderIcon ? 0 : ((control.availableHeight - height) / 2))

        control: control
    }

    background: P.BlossomListItemBackground {
        control: control
    }
}
