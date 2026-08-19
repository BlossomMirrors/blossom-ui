// SPDX-License-Identifier: GPL-2.0-or-later
#include "itemviewcontrol.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "itemview.h"

#include <QAbstractItemView>
#include <QFrame>
#include <QPainterPath>
#include <QListView>
#include <QListWidget>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <QTreeView>

namespace BlossomUI {

bool Render::ItemViewControl::drawPanelItemViewItemPrimitive(const QStyleOption *option,
                                           QPainter *painter,
                                           const QWidget *widget) const {
  // cast option and check
  const auto viewItemOption =
      qstyleoption_cast<const QStyleOptionViewItem *>(option);
  if (!viewItemOption)
    return false;

  // try cast widget
  const auto abstractItemView = qobject_cast<const QAbstractItemView *>(widget);

  // store palette and rect
  const auto &palette(option->palette);
  auto rect(option->rect);

  // For table/tree views with a viewport: keep drawing inside the visible area
  // so we don't overflow right/bottom and so right/bottom rounded corners have
  // room to show. Only apply to QTableView/QTreeView (main content); skip
  // QListView (e.g. Dolphin Places sidebar). Don't clip for Dolphin - clipping
  // cuts off the right corner and causes asymmetric rounding.
  if (abstractItemView &&
      (qobject_cast<const QTableView *>(widget) ||
       qobject_cast<const QTreeView *>(widget)) &&
      !(_style->_app.isDolphin && qobject_cast<const QTableView *>(widget))) {
    if (QWidget *vp = abstractItemView->viewport()) {
      const int radius = StyleConfigData::itemViewRadius();
      const QRect vpRect(0, 0, vp->width(), vp->height());
      const QRect safeRect =
          vpRect.adjusted(0, 0, -qMax(1, radius), -qMax(1, radius));
      rect = rect.intersected(safeRect);
      if (!rect.isValid())
        return true;
    }
  }

  // Clip to item rect so we never draw outside the row/card
  painter->save();
  painter->setClipRect(rect, Qt::IntersectClip);

  // store flags
  const QStyle::State &state(option->state);
  const bool mouseOver(
      (state & QStyle::State_MouseOver) &&
      (!abstractItemView ||
       abstractItemView->selectionMode() != QAbstractItemView::NoSelection));
  const bool selected(state & QStyle::State_Selected);
  const bool enabled(state & QStyle::State_Enabled);

  const bool hasCustomBackground =
      viewItemOption->backgroundBrush.style() != Qt::NoBrush &&
      !(state & QStyle::State_Selected);
  const bool hasSolidBackground =
      !hasCustomBackground ||
      viewItemOption->backgroundBrush.style() == Qt::SolidPattern;
  const bool hasAlternateBackground(viewItemOption->features &
                                    QStyleOptionViewItem::Alternate);

  // do nothing if no background is to be rendered
  if (!(mouseOver || selected || hasCustomBackground ||
        hasAlternateBackground)) {
    painter->restore();
    return true;
  }

  // define color group
  QPalette::ColorGroup colorGroup;
  if (enabled)
    colorGroup = QPalette::Active;
  else
    colorGroup = QPalette::Disabled;

  // render alternate background
  if (hasAlternateBackground) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette.brush(colorGroup, QPalette::AlternateBase));
    painter->drawRect(rect);
  }

  // stop here if no highlight is needed
  if (!(mouseOver || selected || hasCustomBackground)) {
    painter->restore();
    return true;
  }

  // render custom background
  if (hasCustomBackground && !hasSolidBackground) {
    painter->setBrushOrigin(viewItemOption->rect.topLeft());
    painter->setBrush(viewItemOption->backgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(viewItemOption->rect,
                             StyleConfigData::itemViewRadius(),
                             StyleConfigData::itemViewRadius());
    painter->restore();
    return true;
  }

  const QColor color = Render::itemViewSelectionFill(
                            palette, colorGroup, selected, mouseOver,
                            (hasCustomBackground && hasSolidBackground)
                                ? viewItemOption->backgroundBrush.color()
                                : QColor())
                            .brush.color();

  if (widget) {
    if (widget->property(PropertyNames::sidePanelView).toBool()) {
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setPen(Qt::NoPen);
      painter->setBrush(color);
      const qreal r = qMin(4.0, 0.5 * qMin(rect.width(), rect.height()));
      painter->drawRoundedRect(QRectF(rect), r, r);
      painter->restore();
      return true;
    }

    // Dolphin file list (KItemListContainer): bottom corners only, small radius
    // for symmetric corners (avoids "left circle, right none" from
    // viewItemPosition + large radius)
    if (_style->_app.isDolphin && (qobject_cast<const QTableView *>(widget) ||
                           widget->inherits("KItemListWidget"))) {
      const qreal radius = 4;
      _style->_helper->renderSelection(painter, rect, color, CornersBottom, radius);
      painter->restore();
      return true;
    }

    if (!(qobject_cast<const QTableView *>(widget) ||
          qobject_cast<const QListWidget *>(widget) ||
          qobject_cast<const QListView *>(widget))) {
      Corners corners;
      if (!viewItemOption->rect.isNull()) {
        if (viewItemOption->viewItemPosition ==
                QStyleOptionViewItem::Beginning ||
            viewItemOption->viewItemPosition == QStyleOptionViewItem::OnlyOne)
          corners |= CornersLeft;
        if (viewItemOption->viewItemPosition == QStyleOptionViewItem::End ||
            viewItemOption->viewItemPosition == QStyleOptionViewItem::OnlyOne)
          corners |= CornersRight;
      }

      _style->_helper->renderSelection(painter, rect, color, corners);
      painter->restore();
      return true;
    }
  }

  _style->_helper->renderSelection(painter, rect, color, AllCorners);

  painter->restore();
  return true;
}

bool Render::ItemViewControl::drawItemViewItemControl(const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget) const {
  if (!widget || !widget->property(PropertyNames::sidePanelView).toBool())
    return false;

  const auto viewItemOption =
      qstyleoption_cast<const QStyleOptionViewItem *>(option);
  if (!viewItemOption)
    return false;

  if (!(option->state & QStyle::State_Selected))
    return false;

  QStyleOptionViewItem opt = *viewItemOption;
  opt.font.setBold(true);
  opt.fontMetrics = QFontMetrics(opt.font);

  const QColor accent = option->palette.color(QPalette::Highlight);
  opt.palette.setColor(QPalette::Active, QPalette::HighlightedText, accent);
  opt.palette.setColor(QPalette::Inactive, QPalette::HighlightedText, accent);
  opt.palette.setColor(QPalette::Disabled, QPalette::HighlightedText, accent);

  _style->ParentStyleClass::drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
  return true;
}

QMargins Helper::itemViewItemMargins(const QStyleOptionViewItem *option) const {
  QMargins margins(
      Render::ItemView_ItemMarginLeft, Render::ItemView_ItemMarginTop,
      Render::ItemView_ItemMarginRight, Render::ItemView_ItemMarginBottom);
  if (!option) {
    return margins;
  }

  const QFrame *frame = qobject_cast<const QFrame *>(option->widget);
  const QAbstractItemView *abstractItemView =
      qobject_cast<const QAbstractItemView *>(option->widget);

  const bool isFirst = option->index.row() == 0;
  const bool hasFrame = frame && frame->frameShape() == QFrame::StyledPanel;
  const bool reverse = option->direction == Qt::RightToLeft;

  if (isFirst) {
    margins.setTop(Render::ItemView_FirstItemTopMarginHeight);
  }

  // Breeze frame has one extra white pixel
  if (hasFrame) {
    margins -= QMargins(1, isFirst ? 1 : 0, 1, 0);
  }

  if (abstractItemView &&
      abstractItemView->selectionBehavior() != QAbstractItemView::SelectRows) {
    return margins;
  }

  if ((reverse && option->viewItemPosition == QStyleOptionViewItem::End) ||
      (!reverse &&
       option->viewItemPosition == QStyleOptionViewItem::Beginning) ||
      option->viewItemPosition == QStyleOptionViewItem::Middle) {
    margins.setRight(0);
  }
  if ((reverse &&
       option->viewItemPosition == QStyleOptionViewItem::Beginning) ||
      (!reverse && option->viewItemPosition == QStyleOptionViewItem::End) ||
      option->viewItemPosition == QStyleOptionViewItem::Middle) {
    margins.setLeft(0);
  }

  return margins;
}

void Helper::renderSelection(QPainter *painter, const QRect &rect,
                             const QColor &color, Corners corners) const {
  renderSelection(painter, rect, color, corners,
                  StyleConfigData::itemViewRadius());
}

void Helper::renderSelection(QPainter *painter, const QRect &rect,
                             const QColor &color, Corners corners,
                             qreal radius) const {
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);
  painter->setBrush(color);

  QPainterPath path(roundedPath(rect, corners, radius));
  painter->drawPath(path);
}

//* enable mouse over effects in itemviews' viewport
bool Style::polishItemView(QWidget *widget) {
  auto itemView = qobject_cast<QAbstractItemView *>(widget);
  if (!itemView)
    return false;
  itemView->viewport()->setAttribute(Qt::WA_Hover);
  return true;
}
bool Style::drawPanelItemViewItemPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ItemViewControl(this).drawPanelItemViewItemPrimitive(option, painter, widget);
}

bool Style::drawItemViewItemControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ItemViewControl(this).drawItemViewItemControl(option, painter, widget);
}
} // namespace BlossomUI
