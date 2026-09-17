import QtQuick
import org.kde.kirigami.controls as KC
import org.kde.kirigami.platform as Platform
import org.kde.kirigami.templates as KT

KT.InlineMessage {
    id: root

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

    padding: Platform.Units.largeSpacing

    background: Rectangle {
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
