// SPDX-License-Identifier: GPL-2.0-or-later
#include "slider.h"

#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill sliderGrooveFill(const Helper *helper, const QPalette &palette) {
  return Fill(helper->alphaColor(palette.color(QPalette::WindowText), 0.16));
}

Fill sliderGrooveHighlight(const QPalette &palette) {
  return Fill(KColorUtils::mix(palette.color(QPalette::Highlight),
                               palette.color(QPalette::Window), 0.25));
}

Fill sliderHandleFill(const QPalette &palette) {
  return Fill(palette.color(QPalette::Text));
}

Fill sliderHandleOutline(const QPalette &palette) {
  return Fill(palette.color(QPalette::Highlight));
}

Fill dialGrooveFill(const QPalette &palette) {
  return Fill(KColorUtils::mix(palette.color(QPalette::Window),
                               palette.color(QPalette::WindowText), 0.16));
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
