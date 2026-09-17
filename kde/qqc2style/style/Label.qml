// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import org.kde.desktop as Desktop
import org.kde.kirigami as Kirigami
import org.kde.kirigami.templates as KT
import "private/contrast.js" as Contrast

Desktop.Label {
    id: control

    Component.onCompleted: {
        const row = control.parent;
        const badge = row ? row.parent : null;
        if (!badge || badge.contentItem !== row || !badge.background
                || !("customColor" in badge) || !("customTextColor" in badge) || !("type" in badge)) {
            return;
        }

        const isCustom = () => badge.customColor !== Qt.color("transparent");

        const outlineColor = () => {
            switch (badge.type) {
            case KT.Badge.Type.Positive:
                return Kirigami.Theme.positiveTextColor;
            case KT.Badge.Type.Warning:
                return Kirigami.Theme.neutralTextColor;
            case KT.Badge.Type.Error:
                return Kirigami.Theme.negativeTextColor;
            default:
                return /^\s*[-+]?(\d[\d.,\s]*|[IVXLCDM]+)\+?\s*$/.test(badge.text)
                    ? Kirigami.Theme.negativeTextColor
                    : Kirigami.Theme.activeTextColor;
            }
        };

        const fillColor = () => isCustom() ? badge.customColor : outlineColor();

        const textColor = () => {
            if (badge.customTextColor !== Qt.color("transparent")) {
                return badge.customTextColor;
            }
            const fill = fillColor();
            const base = Contrast.relativeLuminance(fill) < 0.4 ? "white" : "black";
            return Kirigami.ColorUtils.linearInterpolation(base, fill, 0.1);
        };

        if (!isCustom()) {
            badge.background.color = Qt.binding(outlineColor);
            badge.background.border.color = Qt.binding(outlineColor);
        }
        control.color = Qt.binding(textColor);

        for (const child of row.children) {
            if (child !== control && "isMask" in child && "color" in child) {
                child.color = Qt.binding(() => badge.icon.color !== Qt.color("transparent") ? badge.icon.color : textColor());
            }
        }
    }
}
