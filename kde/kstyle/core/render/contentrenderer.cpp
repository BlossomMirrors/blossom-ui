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

ContentRects ContentRenderer::layout(const QRect &bounds,
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

  const QSize iconSize = spec.iconSize;
  const QSize textSize = metrics.size(content.textFlags, content.text);

  if (spec.arrangement == ContentArrangement::TextUnderIcon) {
    const int height = iconSize.height() + textSize.height() + spec.itemSpacing;
    rects.icon = QRect(
        QPoint(bounds.left() + qRound((bounds.width() - iconSize.width()) / 2.0),
               bounds.top() + qRound((bounds.height() - height) / 2.0)),
        iconSize);
    rects.text = QRect(
        QPoint(bounds.left() + qRound((bounds.width() - textSize.width()) / 2.0),
               rects.icon.bottom() + spec.itemSpacing + 1),
        textSize);
    return rects;
  }

  if (spec.alignment & Qt::AlignLeft) {
    rects.icon =
        QRect(QPoint(bounds.left() + spec.leftMargin,
                     bounds.top() + (bounds.height() - iconSize.height()) / 2),
              iconSize);
  } else {
    const int width = iconSize.width() + textSize.width() + spec.itemSpacing;
    rects.icon = QRect(
        QPoint(bounds.left() + qRound((bounds.width() - width) / 2.0),
               bounds.top() + qRound((bounds.height() - iconSize.height()) / 2.0)),
        iconSize);
  }
  rects.text =
      QRect(QPoint(rects.icon.right() + spec.itemSpacing + 1,
                   bounds.top() + (bounds.height() - textSize.height()) / 2),
            textSize);
  return rects;
}

void ContentRenderer::paint(QPainter *painter, const QRect &bounds,
                            const ContentLayout &spec, const Content &content,
                            const WidgetInteractionState &state) const {
  ContentRects rects = layout(bounds, spec, content, QFontMetrics(content.font));

  if (rects.icon.isValid() && !content.icon.isNull()) {
    QRect iconRect = rects.icon;
    iconRect = QRect(iconRect.center() - QPoint(spec.iconSize.width() / 2,
                                                spec.iconSize.height() / 2),
                     spec.iconSize);
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap =
        _helper->coloredIcon(content.icon, state.palette, spec.iconSize, dpr,
                             content.iconMode, content.iconState);
    QRect target(QPoint(), pixmap.size() / pixmap.devicePixelRatio());
    target.moveCenter(iconRect.center());
    painter->drawPixmap(target, pixmap);
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
