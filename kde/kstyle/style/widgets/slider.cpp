// SPDX-License-Identifier: GPL-2.0-or-later
#include "slider.h"
#include "switch.h"

#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

const WidgetSpec SliderHandleSpec = WidgetBuilder().geometry(
    Geometry().frameInset(Slider_HandleInset).shape(Shape::Ellipse));

WidgetSpec sliderHandle(const QPalette &palette, const Fill &fill, const QColor &outline) {
  const bool isDark = palette.color(QPalette::Window).lightness() < 128;
  Shadow shadow;
  shadow.yOffset = Slider_HandleShadowYOffset;
  shadow.blur = Slider_HandleShadowBlur;
  shadow.color = QColor(0, 0, 0, isDark ? Slider_HandleShadowAlphaDark : Slider_HandleShadowAlphaLight);

  WidgetSpec spec = SliderHandleSpec;
  spec.fill(StateStyle<Fill>(fill).pressed(Fill(fill.brush.color().darker(Slider_HandlePressedDarker))));
  if (outline.isValid())
    spec.border(Border(outline, 1.0));
  spec.shadow(shadow);
  return spec;
}

Fill sliderGrooveFill(const Helper *helper, const QPalette &palette) {
    Q_UNUSED(helper)
    // the empty groove uses the button surface like the switch track
    return Fill(palette.color(QPalette::Button));
}

Fill sliderGrooveHighlight(const QPalette &palette) {
  return Fill(KColorUtils::mix(palette.color(QPalette::Highlight),
                               palette.color(QPalette::Window), 0.25));
}

Fill sliderHandleFill(const QPalette &palette) {
  return switchThumbFill(palette);
}

Fill sliderHandleOutline(const QPalette &palette) {
  return Fill(palette.color(QPalette::Highlight));
}

Fill dialGrooveFill(const QPalette &palette) {
    // the empty groove uses the button surface like the switch track
    return Fill(palette.color(QPalette::Button));
}

Fill dialHighlight(const QPalette &palette) {
  return Fill(KColorUtils::mix(palette.color(QPalette::Highlight),
                               palette.color(QPalette::Window), 0.25));
}

Fill dialHandleOutline(const QPalette &palette) {
  const QColor background = palette.color(QPalette::Button);
  return Fill(KColorUtils::mix(background, palette.color(QPalette::WindowText), 0.2));
}

} // namespace Render
} // namespace BlossomUI
