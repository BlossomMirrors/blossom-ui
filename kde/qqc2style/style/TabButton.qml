

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import "private/contrast.js" as Contrast

T.TabButton {
    id: control

    readonly property var bar: control.TabBar.tabBar
    readonly property bool barDrawsSelection: bar?.blossomSlidingSelection ?? false

    readonly property color selectedTextColor: Contrast.accentOn(
        Kirigami.Theme.highlightColor,
        bar?.selectionSurface ?? Contrast.blend(
            Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.18),
        4.5)

    readonly property color currentTextColor: control.checked
        ? control.selectedTextColor : Kirigami.Theme.textColor

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    width: bar?.uniformTabWidth || implicitWidth

    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing
    leftPadding: Kirigami.Units.largeSpacing * 2
    rightPadding: Kirigami.Units.largeSpacing * 2
    spacing: Kirigami.Units.smallSpacing

    hoverEnabled: true

    Kirigami.MnemonicData.enabled: enabled && visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: text
    Shortcut {
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.Kirigami.MnemonicData.sequence
        onActivated: {
            if (typeof control.animateClick === "function") {
                control.animateClick();
            } else {
                control.checked = true;
            }
        }
    }

    contentItem: Item {
        implicitWidth: layout.implicitWidth
        implicitHeight: layout.implicitHeight

        opacity: control.enabled ? 1 : 0.6

        RowLayout {
            id: layout

            anchors.centerIn: parent
            spacing: control.spacing

            Kirigami.Icon {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: control.icon.width || Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: control.icon.height || Kirigami.Units.iconSizes.smallMedium

                visible: control.display !== T.AbstractButton.TextOnly
                    && (control.icon.name.length > 0 || control.icon.source.toString().length > 0)
                source: control.icon.name.length > 0 ? control.icon.name : control.icon.source
                color: control.currentTextColor
            }

            Label {
                Layout.alignment: Qt.AlignVCenter

                visible: control.display !== T.AbstractButton.IconOnly && control.text.length > 0
                text: control.Kirigami.MnemonicData.richTextLabel
                font: control.font
                color: control.currentTextColor
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    background: Rectangle {
        radius: control.bar?.pillRadius ?? Kirigami.Units.smallSpacing

        color: {
            if (control.checked) {
                return control.barDrawsSelection
                    ? "transparent"
                    : Qt.alpha(Kirigami.Theme.highlightColor, control.bar?.pillAlpha ?? 0.18);
            }
            if (control.down) {
                return Qt.alpha(Kirigami.Theme.textColor, 0.12);
            }
            if (control.hovered) {
                return Qt.alpha(Kirigami.Theme.textColor, 0.07);
            }
            return "transparent";
        }

        border.width: control.visualFocus ? 1 : 0
        border.color: Kirigami.Theme.highlightColor

        Behavior on color {
            enabled: Kirigami.Units.shortDuration > 0
            ColorAnimation {
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }
}
