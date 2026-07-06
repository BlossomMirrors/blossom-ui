// SPDX-License-Identifier: GPL-2.0-or-later
// Blossom override: kstyle-matching translucent selection background.

import org.kde.desktop as Desktop
import "private" as P

Desktop.CheckDelegate {
    id: control
    background: P.BlossomListItemBackground {
        control: control
    }
}
