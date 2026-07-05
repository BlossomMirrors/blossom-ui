// SPDX-License-Identifier: GPL-2.0-or-later
// Blossom override: Blossom switch indicator + kstyle-matching translucent
// selection background.

import QtQuick.Templates as T
import org.kde.desktop as Desktop
import "private" as P

Desktop.SwitchDelegate {
    id: control

    indicator: P.SwitchIndicator {
        x: !control.mirrored ? control.horizontalPadding : control.width - width - control.horizontalPadding
        y: control.topPadding + (control.display === T.AbstractButton.TextUnderIcon ? 0 : ((control.availableHeight - height) / 2))

        control: control
    }

    background: P.BlossomListItemBackground {
        control: control
    }
}
