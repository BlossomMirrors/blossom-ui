/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  BlossomUI card surface: rounder, roomier and with a real drop shadow.
 *  Kirigami resolves this through StyleSelector, which prefers
 *  styles/<style-name>/AbstractCard.qml over controls/AbstractCard.qml.
 */
import QtQuick
import org.kde.kirigami.platform as Platform
import org.kde.kirigami.primitives as Primitives
import org.kde.kirigami.templates as KT

KT.AbstractCard {
    id: root

    // same formula KirigamiAddons FormCardUnits uses, so cards and form cards
    // land at the same density and it stays DPI-aware
    padding: Platform.Units.largeSpacing + Platform.Units.smallSpacing

    background: Primitives.ShadowedRectangle {
        radius: Platform.Units.cornerRadius

        // hover/press feedback carried over from DefaultCardBackground; we own
        // the file, so root can be read directly instead of casting parent
        color: {
            if (root.checked || (root.showClickFeedback && (root.down || root.highlighted))) {
                return Platform.ColorUtils.tintWithAlpha(Platform.Theme.backgroundColor,
                                                         Platform.Theme.highlightColor, 0.3);
            }
            if (root.hoverEnabled && root.hovered) {
                return Platform.ColorUtils.tintWithAlpha(Platform.Theme.backgroundColor,
                                                         Platform.Theme.highlightColor, 0.1);
            }
            return Platform.Theme.backgroundColor;
        }

        border {
            width: 1
            color: Platform.ColorUtils.linearInterpolation(Platform.Theme.backgroundColor,
                                                           Platform.Theme.textColor,
                                                           Platform.Theme.frameContrast)
        }

        // a real shadow, replacing the stock fake solid offset Rectangle at z: -1.
        // Light themes need a much weaker black than dark ones, where the same
        // alpha reads as dirt instead of elevation.
        shadow {
            size: Platform.Units.largeSpacing
            color: Platform.ColorUtils.brightnessForColor(Platform.Theme.backgroundColor) === Platform.ColorUtils.Light
                ? Qt.rgba(0, 0, 0, 0.08)
                : Qt.rgba(0, 0, 0, 0.25)
            yOffset: 2
        }
    }
}
