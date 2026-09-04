/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  BlossomUI notice surface. The stock control paints a full-saturation 1px
 *  ring around a 20% tint, with the inner radius at 0.6x the outer one, which
 *  reads as a hard outline with mismatched corners. This is one tinted
 *  surface instead, with a border in the same hue that you sense rather than
 *  see, matching the card treatment.
 */
import QtQuick
import org.kde.kirigami.controls as KC
import org.kde.kirigami.platform as Platform
import org.kde.kirigami.templates as KT

KT.InlineMessage {
    id: root

    //* the message type's hue, used for both the fill tint and the border
    readonly property color __blossomAccent: {
        switch (root.type) {
        case KC.MessageType.Positive:
            return Platform.Theme.positiveTextColor;
        case KC.MessageType.Warning:
            return Platform.Theme.neutralTextColor;
        case KC.MessageType.Error:
            return Platform.Theme.negativeTextColor;
        default:
            return Platform.Theme.activeTextColor;
        }
    }

    // stock lands on 5px (1px border margin + smallSpacing), which is cramped
    // for a full-width notice
    padding: Platform.Units.largeSpacing

    background: Rectangle {
        // header and footer messages span the full width, so they stay square
        radius: root.position === KT.InlineMessage.Position.Inline
            ? Platform.Units.cornerRadius : 0

        color: Platform.ColorUtils.tintWithAlpha(Platform.Theme.backgroundColor,
                                                 root.__blossomAccent, 0.12)

        border {
            width: 1
            color: Platform.ColorUtils.tintWithAlpha(Platform.Theme.backgroundColor,
                                                     root.__blossomAccent, 0.35)
        }
    }
}
