// SPDX-License-Identifier: GPL-2.0-or-later
#include "contentrenderer.h"

#include "blossomuihelper.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>

namespace BlossomUI {
namespace Render {

static bool wantsIcon(const ContentLayout &spec, const Content &content) {
  if (content.icon.isNull())
    return false;
  return spec.arrangement != ContentArrangement::TextOnly;
}

static bool wantsText(const ContentLayout &spec, const Content &content) {
  if (content.text.isEmpty())
    return false;
  return spec.arrangement != ContentArrangement::IconOnly;
}

ContentRects ContentRenderer::layout(const QRectF &bounds,
                                     const ContentLayout &spec,
                                     const Content &content,
                                     const QFontMetrics &metrics) const {
  ContentRects rects;
  const bool hasIcon = wantsIcon(spec, content);
  const bool hasText = wantsText(spec, content);

  if (!hasIcon && !hasText)
    return rects;

  if (hasText && !hasIcon) {
    rects.text = bounds;
    return rects;
  }
  if (hasIcon && !hasText) {
    rects.icon = bounds;
    return rects;
  }

  const QSizeF iconSize = spec.iconSize;
  const QSizeF textSize = metrics.size(content.textFlags, content.text);

  if (spec.arrangement == ContentArrangement::TextUnderIcon) {
    const qreal height = iconSize.height() + textSize.height() + spec.itemSpacing;
    rects.icon = QRectF(
        QPointF(bounds.left() + (bounds.width() - iconSize.width()) / 2.0,
               bounds.top() + (bounds.height() - height) / 2.0),
        iconSize);
    rects.text = QRectF(
        QPointF(bounds.left() + (bounds.width() - textSize.width()) / 2.0,
               rects.icon.bottom() + spec.itemSpacing),
        textSize);
    return rects;
  }

  if (spec.alignment & Qt::AlignLeft) {
    rects.icon =
        QRectF(QPointF(bounds.left() + spec.leftMargin,
                       bounds.top() + (bounds.height() - iconSize.height()) / 2.0),
              iconSize);
  } else {
    const qreal width = iconSize.width() + textSize.width() + spec.itemSpacing;
    rects.icon = QRectF(
        QPointF(bounds.left() + (bounds.width() - width) / 2.0,
               bounds.top() + (bounds.height() - iconSize.height()) / 2.0),
        iconSize);
  }
  rects.text =
      QRectF(QPointF(rects.icon.right() + spec.itemSpacing,
                     bounds.top() + (bounds.height() - textSize.height()) / 2.0),
            textSize);
  return rects;
}

void ContentRenderer::paint(QPainter *painter, const QRectF &bounds,
                            const ContentLayout &spec, const Content &content,
                            const WidgetInteractionState &state) const {
  ContentRects rects = layout(bounds, spec, content, QFontMetrics(content.font));

  if (rects.icon.isValid() && !content.icon.isNull()) {
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap =
        _helper->coloredIcon(content.icon, state.palette, spec.iconSize, dpr,
                             content.iconMode, content.iconState);
    QRectF target(QPointF(), QSizeF(pixmap.size()) / pixmap.devicePixelRatio());
    target.moveCenter(rects.icon.center());
    painter->drawPixmap(target.topLeft(), pixmap);
  }

  if (rects.text.isValid() && !content.text.isEmpty()) {
    painter->save();
    painter->setFont(content.font);
    painter->setPen(state.palette.color(state.enabled ? QPalette::Active
                                                      : QPalette::Disabled,
                                        content.textRole));
    painter->drawText(rects.text, content.textFlags, content.text);
    painter->restore();
  }
}

} // namespace Render
} // namespace BlossomUI
