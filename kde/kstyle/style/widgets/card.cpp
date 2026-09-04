// SPDX-License-Identifier: GPL-2.0-or-later
#include "card.h"

#include "blossomuistyleconfigdata.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

namespace {

//* only cardFrame needs the elevation, so it stays file-local
Shadow cardShadow(const QPalette &palette) {
  const bool isDark = palette.color(QPalette::Window).lightness() < 128;
  Shadow shadow;
  shadow.yOffset = Card_ShadowYOffset;
  shadow.blur = Card_ShadowBlur;
  shadow.color =
      QColor(0, 0, 0, isDark ? Card_ShadowAlphaDark : Card_ShadowAlphaLight);
  return shadow;
}

} // namespace

// RadiusResolver subtracts the inset, so the bias adds Frame_FrameWidth back:
// painted radius stays cornerRadius + Card_RadiusBias, matching cardRadius()
const WidgetSpec CardSpec =
    WidgetBuilder().geometry(
        Geometry()
            .frameInset(Frame_FrameWidth)
            .radius(RadiusRole::Frame)
            .radiusBias(Card_RadiusBias + Frame_FrameWidth));

WidgetSpec cardFrame(const QPalette &palette, const QColor &background) {
  WidgetSpec spec = CardSpec;
  spec.fill(StateStyle<Fill>(Fill(background)));
  spec.border(cardBorder(palette));
  spec.shadow(cardShadow(palette));
  return spec;
}

int cardRadius() {
  return StyleConfigData::cornerRadius() + Card_RadiusBias;
}

Fill cardBackgroundFill(const QColor &windowColor) {
  const bool isDark = windowColor.lightness() < 128;
  return Fill(isDark ? KColorUtils::mix(windowColor, QColor(255, 255, 255), 0.12)
                     : KColorUtils::mix(windowColor, QColor(0, 0, 0), 0.04));
}

Border cardBorder(const QPalette &palette) {
  QColor color = palette.color(QPalette::WindowText);
  color.setAlphaF(color.alphaF() * Card_BorderAlpha);
  return Border(color, Card_BorderWidth, BorderAlign::Inside);
}

} // namespace Render
} // namespace BlossomUI
