// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomui.h"
#include "blossomuidecoration.h"

#include <KColorUtils>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/ScaleHelpers>

namespace BlossomUI {

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

int Decoration::buttonSize() const {
  const int baseSize = settings()->gridUnit();
  switch (m_internalSettings->buttonSize()) {
  case InternalSettings::ButtonTiny:
    return baseSize * 1.2;
  case InternalSettings::ButtonSmall:
    return baseSize * 1.8;
  default:
  case InternalSettings::ButtonDefault:
    return baseSize * 2.4;
  case InternalSettings::ButtonLarge:
    return baseSize * 3.0;
  case InternalSettings::ButtonVeryLarge:
    return baseSize * 4.0;
  }
}

int Decoration::captionHeight() const {
  return hideTitleBar() ? borderTop()
                        : borderTop() -
                              settings()->smallSpacing() *
                                  (Metrics::TitleBar_BottomMargin +
                                   Metrics::TitleBar_TopMargin) -
                              1;
}

QPair<QRectF, Qt::Alignment> Decoration::captionRect() const {
  if (hideTitleBar())
    return qMakePair(QRect(), Qt::AlignCenter);

  auto c = window();
  const qreal leftOffset = KDecoration3::snapToPixelGrid(
      m_leftButtons->buttons().isEmpty()
          ? Metrics::TitleBar_SideMargin * settings()->smallSpacing()
          : m_leftButtons->geometry().x() + m_leftButtons->geometry().width() +
                Metrics::TitleBar_SideMargin * settings()->smallSpacing(),
      window()->scale());

  const qreal rightOffset = KDecoration3::snapToPixelGrid(
      m_rightButtons->buttons().isEmpty()
          ? Metrics::TitleBar_SideMargin * settings()->smallSpacing()
          : size().width() - m_rightButtons->geometry().x() +
                Metrics::TitleBar_SideMargin * settings()->smallSpacing(),
      window()->scale());

  const qreal yOffset = KDecoration3::snapToPixelGrid(
      settings()->smallSpacing() * Metrics::TitleBar_TopMargin,
      window()->scale());
  const QRectF maxRect(leftOffset, yOffset,
                       size().width() - leftOffset - rightOffset,
                       captionHeight());

  switch (m_internalSettings->titleAlignment()) {
  case InternalSettings::AlignLeft:
    return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);

  case InternalSettings::AlignRight:
    return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);

  case InternalSettings::AlignCenter:
    return qMakePair(maxRect, Qt::AlignCenter);

  default:
  case InternalSettings::AlignCenterFullWidth: {
    const QRectF fullRect = QRect(0, yOffset, size().width(), captionHeight());
    QRectF boundingRect(settings()->fontMetrics().boundingRect(c->caption()));

    boundingRect.setTop(yOffset);
    boundingRect.setHeight(captionHeight());
    boundingRect.moveLeft((size().width() - boundingRect.width()) / 2.0);

    if (boundingRect.left() < leftOffset)
      return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);
    else if (boundingRect.right() > size().width() - rightOffset)
      return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);
    else
      return qMakePair(fullRect, Qt::AlignCenter);
  }
  }
}

qreal Decoration::borderSize(bool bottom, qreal scale) const {
  const qreal pixelSize = KDecoration3::pixelSize(scale);
  const qreal baseSize = std::max<qreal>(
      pixelSize,
      KDecoration3::snapToPixelGrid(settings()->smallSpacing(), scale));

  if (m_internalSettings && (m_internalSettings->mask() & BorderSize)) {
    switch (m_internalSettings->borderSize()) {
    case InternalSettings::BorderNone:
      return 0;
    case InternalSettings::BorderNoSides:
      if (bottom) {
        return KDecoration3::snapToPixelGrid(
            std::max(4.0, baseSize + Metrics::Frame_FrameRadius), scale);
      } else {
        return 0;
      }
    default:
    case InternalSettings::BorderTiny:
      if (bottom) {
        return KDecoration3::snapToPixelGrid(std::max(4.0, baseSize), scale);
      } else {
        return baseSize;
      }
    case InternalSettings::BorderNormal:
      return baseSize * 2;
    case InternalSettings::BorderLarge:
      return baseSize * 3;
    case InternalSettings::BorderVeryLarge:
      return baseSize * 4;
    case InternalSettings::BorderHuge:
      return baseSize * 5;
    case InternalSettings::BorderVeryHuge:
      return baseSize * 6;
    case InternalSettings::BorderOversized:
      return baseSize * 10;
    }
  } else {
    switch (settings()->borderSize()) {
    case KDecoration3::BorderSize::None:
      return 0;
    case KDecoration3::BorderSize::NoSides:
      if (bottom) {
        return KDecoration3::snapToPixelGrid(
            std::max(4.0, baseSize + Metrics::Frame_FrameRadius), scale);
      } else {
        return 0;
      }
    default:
    case KDecoration3::BorderSize::Tiny:
      if (bottom) {
        return KDecoration3::snapToPixelGrid(std::max(4.0, baseSize), scale);
      } else {
        return baseSize;
      }
    case KDecoration3::BorderSize::Normal:
      return baseSize * 2;
    case KDecoration3::BorderSize::Large:
      return baseSize * 3;
    case KDecoration3::BorderSize::VeryLarge:
      return baseSize * 4;
    case KDecoration3::BorderSize::Huge:
      return baseSize * 5;
    case KDecoration3::BorderSize::VeryHuge:
      return baseSize * 6;
    case KDecoration3::BorderSize::Oversized:
      return baseSize * 10;
    }
  }
}

QMarginsF Decoration::bordersFor(qreal scale) const {
  qreal left = isLeftEdge() ? 0 : borderSize(false, scale);
  qreal right = isRightEdge() ? 0 : borderSize(false, scale);
  qreal bottom = isBottomEdge() ? 0 : borderSize(true, scale);

  qreal top = 0;
  if (hideTitleBar()) {
    top = bottom;
  } else {
    QFontMetrics fm(settings()->font());
    top += KDecoration3::snapToPixelGrid(std::max(fm.height(), buttonSize()),
                                         scale);

    const int baseSize = settings()->smallSpacing();
    top += KDecoration3::snapToPixelGrid(
        baseSize * Metrics::TitleBar_BottomMargin, scale);
    top += KDecoration3::snapToPixelGrid(baseSize * Metrics::TitleBar_TopMargin,
                                         scale);
  }
  return QMarginsF(left, top, right, bottom);
}

void Decoration::recalculateBorders() {
  setBorders(bordersFor(window()->nextScale()));

  const qreal extSize = KDecoration3::snapToPixelGrid(
      settings()->largeSpacing(), window()->nextScale());
  qreal extSides = 0;
  qreal extBottom = 0;
  if (hasNoBorders()) {
    if (!isMaximizedHorizontally()) {
      extSides = extSize;
    }
    if (!isMaximizedVertically()) {
      extBottom = extSize;
    }
  } else if (hasNoSideBorders() && !isMaximizedHorizontally()) {
    extSides = extSize;
  }

  setResizeOnlyBorders(QMarginsF(extSides, 0, extSides, extBottom));

  qreal bottomLeftRadius = 0;
  qreal bottomRightRadius = 0;
  if (hasNoBorders() && m_internalSettings->roundedCorners()) {
    if (!isBottomEdge()) {
      if (!isLeftEdge()) {
        bottomLeftRadius = m_scaledCornerRadius;
      }
      if (!isRightEdge()) {
        bottomRightRadius = m_scaledCornerRadius;
      }
    }
  }
  setBorderRadius(
      KDecoration3::BorderRadius(0, 0, bottomRightRadius, bottomLeftRadius));

  if (window()->isMaximized() || !outlinesEnabled()) {
    setBorderOutline(KDecoration3::BorderOutline());
  } else {
    auto c = window();
    const auto frameColor =
        c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive,
                 ColorRole::Frame);
    const auto textColor = c->palette().text().color();

    const bool isLightTheme = textColor.lightness() < 128;
    const bool isOLEDTheme = frameColor.lightness() < 5;
    const qreal mixRatio = isLightTheme ? 0.30 : (isOLEDTheme ? 0.25 : 0.12);
    const auto borderColor = KColorUtils::mix(frameColor, textColor, mixRatio);

    const qreal thickness =
        std::max(KDecoration3::pixelSize(window()->scale()),
                 KDecoration3::snapToPixelGrid(1, window()->scale()));

    qreal bottomLeftRadius = 0;
    qreal bottomRightRadius = 0;
    if (!hasNoBorders() || m_internalSettings->roundedCorners()) {
      bottomLeftRadius = m_scaledCornerRadius;
      bottomRightRadius = m_scaledCornerRadius;
    }

    const auto radius =
        KDecoration3::BorderRadius(m_scaledCornerRadius, m_scaledCornerRadius,
                                   bottomRightRadius, bottomLeftRadius);
    setBorderOutline(
        KDecoration3::BorderOutline(thickness, borderColor, radius));
  }
}

void Decoration::setScaledCornerRadius() {
  m_scaledCornerRadius =
      m_internalSettings->cornerRadius() * window()->nextScale();
}

void Decoration::updateScale() {
  setScaledCornerRadius();
  recalculateBorders();
}

} // namespace BlossomUI
