// SPDX-License-Identifier: GPL-2.0-or-later
#include "button.h"

#include "blossomui.h"
#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

namespace
{

Shadow buttonShadow(const QPalette &palette)
{
    const bool isDark = palette.color(QPalette::Window).lightness() < 128;
    Shadow shadow;
    shadow.yOffset = Button_ShadowYOffset;
    shadow.blur = Button_ShadowBlur;
    shadow.color = QColor(0, 0, 0, isDark ? Button_ShadowAlphaDark : Button_ShadowAlphaLight);
    return shadow;
}

} // namespace

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
    Q_UNUSED(defaultButton)
    const QPalette &palette = state.palette;
    const QColor hover = helper->hoverSurface(palette);
    WidgetSpec spec = ButtonSpec;
    spec.fill(StateStyle<Fill>(Fill(palette.color(QPalette::Button))).hovered(Fill(hover)).pressed(Fill(hover)));
    spec.shadow(buttonShadow(palette));
    return spec;
}

WidgetSpec toolButtonFrame(const Helper *helper,
                           const WidgetInteractionState &state) {
  const QColor text = state.palette.color(QPalette::WindowText);
  const QColor hover = helper->hoverSurface(state.palette);
  const QColor sunken = helper->alphaColor(text, 0.2);

  // resting is a fully transparent hover so the ghost fill fades in rather
  // than snapping when the hover animation runs
  const QColor resting = helper->alphaColor(hover, 0.0);

  WidgetSpec spec = ToolButtonSpec;
  spec.fill(StateStyle<Fill>(Fill(resting)).hovered(Fill(hover)).pressed(Fill(sunken)).animated());
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
    Q_UNUSED(focused)
    Q_UNUSED(pressed)
    return flat ? QPalette::WindowText : QPalette::ButtonText;
}

QPalette::ColorRole toolButtonTextRole(bool flat, bool focused, bool pressed,
                                       bool hovered) {
    Q_UNUSED(focused)
    Q_UNUSED(pressed)
    Q_UNUSED(hovered)
    return flat ? QPalette::WindowText : QPalette::ButtonText;
}

QIcon::Mode buttonIconMode(bool enabled, bool flat, bool focused, bool pressed,
                           bool hovered) {
    Q_UNUSED(focused)
    Q_UNUSED(pressed)
    if (!enabled)
        return QIcon::Disabled;
    if (hovered && flat)
        return QIcon::Active;
    return QIcon::Normal;
}

QIcon::Mode toolButtonIconMode(bool enabled, bool flat, bool focused,
                               bool pressed, bool hovered) {
    Q_UNUSED(flat)
    Q_UNUSED(focused)
    Q_UNUSED(pressed)
    Q_UNUSED(hovered)
    if (!enabled)
        return QIcon::Disabled;
    return QIcon::Normal;
}

} // namespace Render
} // namespace BlossomUI
