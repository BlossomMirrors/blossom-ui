// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomui.h"
#include "blossomuidecoration.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/ScaleHelpers>
#include <QPainter>

namespace BlossomUI {

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

void Decoration::updateBlur() {
  auto c = window();
  const QColor titleBarColor =
      c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive,
               ColorRole::TitleBar);

  if (titleBarColor.alpha() == 255) {
    this->setOpaque(c->isMaximized());
  } else {
    this->setOpaque(false);
  }

  calculateWindowAndTitleBarShapes(true);
  this->setBlurRegion(QRegion(m_windowPath->toFillPolygon().toPolygon()));
}

void Decoration::calculateWindowAndTitleBarShapes(const bool windowShapeOnly) {
  auto s = settings();

  if (!windowShapeOnly) {
    m_titleRect = QRect(QPoint(0, 0), QSize(size().width(), borderTop()));
    m_titleBarPath->clear();
    if (isMaximized() || !s->isAlphaChannelSupported()) {
      m_titleBarPath->addRect(m_titleRect);
    } else {
      QPainterPath path;
      const qreal radius = m_scaledCornerRadius;

      const qreal W = m_titleRect.width();
      const qreal bottom = m_titleRect.bottom();
      path.moveTo(-1.0, bottom);
      path.lineTo(-1.0, radius - 1.0);
      path.arcTo(QRectF(-1.0, -1.0, radius * 2, radius * 2), 180, -90);
      path.lineTo(W + 1.0 - radius, -1.0);
      path.arcTo(QRectF(W + 1.0 - radius * 2, -1.0, radius * 2, radius * 2), 90,
                 -90);
      path.lineTo(W + 1.0, bottom);
      path.closeSubpath();

      *m_titleBarPath = path;
    }
  }

  m_windowPath->clear();
  if (s->isAlphaChannelSupported() && !isMaximized()) {
    QRectF adjustedRect = rect().adjusted(-1.0, -1.0, 1.0, 1.0);
    m_windowPath->addRoundedRect(adjustedRect, m_scaledCornerRadius,
                                 m_scaledCornerRadius);
  } else {
    m_windowPath->addRect(rect());
  }
}

void Decoration::updateTitleBar() {
  auto s = settings();
  const bool maximized = isMaximized();
  const qreal width =
      maximized ? window()->width()
                : window()->width() -
                      2 * s->smallSpacing() * Metrics::TitleBar_SideMargin;
  const qreal height =
      (maximized || isTopEdge())
          ? borderTop()
          : borderTop() - s->smallSpacing() * Metrics::TitleBar_TopMargin;
  const qreal x =
      maximized ? 0 : s->smallSpacing() * Metrics::TitleBar_SideMargin;
  const qreal y = (maximized || isTopEdge())
                      ? 0
                      : s->smallSpacing() * Metrics::TitleBar_TopMargin;
  setTitleBar(QRectF(x, y, width, height));
}

void Decoration::paint(QPainter *painter, const QRectF &repaintRegion) {
  auto c = window();
  auto s = settings();

  calculateWindowAndTitleBarShapes();

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);
  painter->setPen(Qt::NoPen);

  if (s->isAlphaChannelSupported() && !isMaximized()) {
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(rect(), Qt::transparent);
    QPainterPath roundedClip;
    roundedClip.addRoundedRect(QRectF(rect()), m_scaledCornerRadius,
                               m_scaledCornerRadius);
    painter->setClipPath(roundedClip);
    painter->setBrush(
        c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive,
                 ColorRole::Frame));
    painter->drawRect(QRectF(rect()));
    painter->setClipping(false);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setClipRect(rect());
  } else {
    painter->fillRect(rect(), Qt::transparent);
    painter->setBrush(
        c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive,
                 ColorRole::Frame));
    painter->drawRect(rect());
  }

  if (!hideTitleBar())
    paintTitleBar(painter, repaintRegion);

  painter->restore();
}

void Decoration::paintTitleBar(QPainter *painter, const QRectF &repaintRegion) {
  const auto c = window();

  const QRectF fullTitleRect(QPointF(0, 0),
                             QSizeF(size().width(), borderTop()));
  if (!fullTitleRect.intersects(repaintRegion))
    return;

  if (!m_internalSettings)
    return;

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);
  painter->setPen(Qt::NoPen);

  if (c->isActive() && m_internalSettings->drawBackgroundGradient()) {
    const QColor titleBarColor(this->titleBarColor());
    QLinearGradient gradient(0, 0, 0, m_titleRect.height());
    gradient.setColorAt(0.0, titleBarColor.lighter(120));
    gradient.setColorAt(0.8, titleBarColor);
    painter->setBrush(gradient);
  } else {
    painter->setBrush(titleBarColor());
  }

  painter->drawPath(*m_titleBarPath);
  painter->restore();

  if (!m_leftButtons || !m_rightButtons)
    return;

  auto s = settings();
  painter->setFont(s->font());
  painter->setPen(fontColor());
  const auto cR = captionRect();
  const QString caption = painter->fontMetrics().elidedText(
      c->caption(), Qt::ElideMiddle, cR.first.width());
  painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

  m_leftButtons->paint(painter, repaintRegion);
  m_rightButtons->paint(painter, repaintRegion);
}

} // namespace BlossomUI
