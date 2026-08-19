// SPDX-License-Identifier: GPL-2.0-or-later
#include "button.h"

#include "blossomui.h"
#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

const WidgetSpec ButtonSpec =
    WidgetBuilder()
        .geometry(Geometry()
                      .frameInset(Render::Frame_FrameWidth)
                      .radius(RadiusRole::Button))
        .border(Border::none())
        .motion(Motion().pressedScale(Render::Button_PressedShrink))
        .ripple(Ripple().style(RippleStyle::FromClickPosition).maxOpacity(0.4))
        .cursor(
            Cursor().enabled(Qt::PointingHandCursor).disabled(Qt::ArrowCursor));

const WidgetSpec ToolButtonSpec =
    WidgetBuilder()
        .geometry(Geometry().frameInset(1).radius(RadiusRole::Frame))
        .border(Border::none())
        .motion(Motion().pressedScale(Render::Button_PressedShrink))
        .ripple(Ripple().style(RippleStyle::FromClickPosition).maxOpacity(0.4))
        .cursor(
            Cursor().enabled(Qt::PointingHandCursor).disabled(Qt::ArrowCursor));

WidgetSpec buttonFrame(const Helper *helper,
                       const WidgetInteractionState &state,
                       bool defaultButton) {
  QPalette palette = state.palette;
  if (state.enabled && defaultButton) {
    palette.setColor(QPalette::Button,
                     KColorUtils::mix(palette.color(QPalette::Button),
                                      palette.color(QPalette::Base), 0.7));
  }

  const QColor base = palette.color(QPalette::Button);
  const QColor focus = helper->focusColor(palette);
  const QColor tinted = KColorUtils::mix(base, focus, 0.28);
  const QColor tintedHover = KColorUtils::mix(base, focus, 0.38);

  // accent tint only for a resting focused button, never while pressed or
  // mid-press-animation
  const bool tintForFocus =
      state.focused && !state.pressed && state.pressOpacity < 0.0;

  QColor resting = base;
  if (tintForFocus) {
    resting = (state.mode == AnimationFocus)
                  ? KColorUtils::mix(base, tinted, state.opacity)
                  : tinted;
  }

  WidgetSpec spec = ButtonSpec;
  spec.fill(StateStyle<Fill>(Fill(resting))
                .hovered(Fill(tintForFocus
                                  ? tintedHover
                                  : helper->buttonAlternateBackground(palette)))
                .pressed(Fill(resting.darker(105))));
  return spec;
}

WidgetSpec toolButtonFrame(const Helper *helper,
                           const WidgetInteractionState &state) {
  const QColor text = state.palette.color(QPalette::WindowText);
  const QColor hover = helper->alphaColor(text, 0.1);
  const QColor sunken = helper->alphaColor(text, 0.2);
  const QColor focus = helper->focusColor(state.palette);

  // resting is a fully transparent hover so the ghost fill fades in rather
  // than snapping when the hover animation runs
  QColor resting = helper->alphaColor(text, 0.0);
  if (state.focused && !state.pressed)
    resting = focus;

  WidgetSpec spec = ToolButtonSpec;
  spec.fill(
      StateStyle<Fill>(Fill(resting))
          .hovered(
              Fill(state.focused ? KColorUtils::mix(focus, hover, 0.5) : hover))
          .pressed(Fill(sunken))
          .animated());
  return spec;
}

ContentLayout buttonContent(int itemSpacing) {
  ContentLayout layout;
  layout.arrangement = ContentArrangement::TextBesideIcon;
  layout.itemSpacing = itemSpacing;
  return layout;
}

ContentLayout toolButtonContent(Qt::ToolButtonStyle style,
                                const QSize &iconSize, bool leftAligned) {
  ContentLayout layout;
  layout.iconSize = iconSize;
  layout.itemSpacing = Render::ToolButton_ItemSpacing;
  layout.alignment = leftAligned ? Qt::AlignLeft : Qt::AlignCenter;
  if (leftAligned)
    layout.leftMargin =
        Render::Button_MarginWidth + Render::Frame_FrameWidth + 1;

  switch (style) {
  case Qt::ToolButtonTextOnly:
    layout.arrangement = ContentArrangement::TextOnly;
    break;
  case Qt::ToolButtonIconOnly:
    layout.arrangement = ContentArrangement::IconOnly;
    break;
  case Qt::ToolButtonTextUnderIcon:
    layout.arrangement = ContentArrangement::TextUnderIcon;
    break;
  default:
    layout.arrangement = ContentArrangement::TextBesideIcon;
    break;
  }
  return layout;
}

QPalette::ColorRole buttonTextRole(bool flat, bool focused, bool pressed) {
  if (!flat)
    return QPalette::ButtonText;
  return (focused && pressed) ? QPalette::HighlightedText
                              : QPalette::WindowText;
}

QPalette::ColorRole toolButtonTextRole(bool flat, bool focused, bool pressed,
                                       bool hovered) {
  if (flat) {
    return (((focused && pressed) || pressed) && !hovered)
               ? QPalette::HighlightedText
               : QPalette::WindowText;
  }
  return (focused || pressed) ? QPalette::HighlightedText
                              : QPalette::ButtonText;
}

QIcon::Mode buttonIconMode(bool enabled, bool flat, bool focused, bool pressed,
                           bool hovered) {
  if (!enabled)
    return QIcon::Disabled;
  if (!flat && focused && !pressed)
    return QIcon::Selected;
  if (hovered && flat)
    return QIcon::Active;
  return QIcon::Normal;
}

QIcon::Mode toolButtonIconMode(bool enabled, bool flat, bool focused,
                               bool pressed, bool hovered) {
  if (!enabled)
    return QIcon::Disabled;
  if ((!flat && (focused || pressed)) || (flat && pressed && !hovered))
    return QIcon::Selected;
  return QIcon::Normal;
}

} // namespace Render
} // namespace BlossomUI
