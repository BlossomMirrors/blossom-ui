#pragma once
// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuistyleconfigdata.h"

#include <cmath>
#include <QAbstractItemDelegate>
#include <QAbstractItemView>
#include <QApplication>
#include <QDragMoveEvent>
#include <QHash>
#include <QHoverEvent>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QStorageInfo>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QWidget>
#include <QWindow>

namespace BlossomUIPrivate {

class DolphinPlacesDelegate : public QStyledItemDelegate {
  static constexpr int s_lateralMargin =
      4; // mirrors KFilePlacesViewDelegate's hardcoded inset
  static constexpr int s_iconLeftPad = 4;
  static constexpr int s_extraGap = 1;
  static constexpr int s_heightExtra = 2;
  static constexpr int s_itemGap = 1;
  static constexpr int s_capacityBarRole =
      0x1548C5C4; // KFilePlacesModel::CapacityBarRecommendedRole
  static constexpr int s_urlRole = 0x069CD12B; // KFilePlacesModel::UrlRole

  struct FreeSpaceInfo {
    quint64 used = 0;
    quint64 size = 0;
  };

public:
  explicit DolphinPlacesDelegate(QAbstractItemView *view,
                                 QAbstractItemDelegate *original)
      : QStyledItemDelegate(view), m_view(view), m_original(original) {
    view->viewport()->installEventFilter(this);
  }

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override {
    QSize s = m_original ? m_original->sizeHint(option, index)
                         : QStyledItemDelegate::sizeHint(option, index);
    s.rheight() += 2 * s_itemGap + s_heightExtra;
    return s;
  }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    if (!m_original || !m_view) {
      QStyledItemDelegate::paint(painter, option, index);
      return;
    }

    if (m_dragging) {
      m_original->paint(painter, option, index);
      return;
    }

    const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (icon.isNull()) {
      m_original->paint(painter, option, index);
      return;
    }

    const int iconSize = m_view->iconSize().width();
    const bool isLTR = option.direction == Qt::LeftToRight;
    const int iconAreaW = s_lateralMargin + s_iconLeftPad + iconSize;
    const int stdH = qMax(iconSize, option.fontMetrics.height()) +
                     s_lateralMargin + s_heightExtra;
    const int headerH = qMax(0, option.rect.height() - stdH - 2 * s_itemGap);
    const int itemTop = option.rect.top() + headerH + s_itemGap;
    const QRect itemRect(option.rect.left(), itemTop, option.rect.width(),
                         stdH);

    if (headerH > 0) {
      const qreal dpr = painter->device()->devicePixelRatioF();
      QPixmap hdrBuf(qRound(option.rect.width() * dpr), qRound(headerH * dpr));
      hdrBuf.setDevicePixelRatio(dpr);
      hdrBuf.fill(Qt::transparent);
      {
        QPainter hp(&hdrBuf);
        QStyleOptionViewItem hopt = option;
        hopt.rect = QRect(0, 0, option.rect.width(), option.rect.height());
        m_original->paint(&hp, hopt, index);
      }
      painter->drawPixmap(option.rect.topLeft(), hdrBuf);
    }

    painter->save();

    const bool selected = option.state & QStyle::State_Selected;
    const bool mouseOver = (QPersistentModelIndex(index) == m_lastHoveredIndex && m_lastHoverOnItem);
    if (selected || mouseOver) {
      const QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                          ? QPalette::Active
                                          : QPalette::Disabled;
      QColor color;
      if (selected) {
        color = option.palette.color(cg, QPalette::Highlight);
        color.setAlphaF(mouseOver ? 0.28 : 0.18);
      } else {
        color = option.palette.color(cg, QPalette::Button);
      }
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setPen(Qt::NoPen);
      painter->setBrush(color);
      const QRectF bgRect = QRectF(itemRect).adjusted(
          s_lateralMargin + 0.5, 0.5, -s_lateralMargin - 0.5, -0.5);
      const qreal r = qMin(4.0, 0.5 * qMin(bgRect.width(), bgRect.height()));
      painter->drawRoundedRect(bgRect, r, r);
    }

    const QIcon::Mode iconMode = !(option.state & QStyle::State_Enabled)
                                     ? QIcon::Disabled
                                     : QIcon::Normal;
    auto *winHandle =
        m_view->window() ? m_view->window()->windowHandle() : nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const qreal dpr =
        winHandle ? winHandle->devicePixelRatio() : m_view->devicePixelRatioF();
    QPixmap pm = icon.pixmap(QSize(iconSize, iconSize), dpr, iconMode);
#else
    QPixmap pm =
        winHandle ? icon.pixmap(winHandle, QSize(iconSize, iconSize), iconMode)
                  : icon.pixmap(QSize(iconSize, iconSize), iconMode);
#endif
    if (selected && (option.state & QStyle::State_Enabled)) {
      QPainter tp(&pm);
      tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
      tp.fillRect(pm.rect(),
                  option.palette.color(QPalette::Active, QPalette::Highlight));
    }
    const int iconX =
        isLTR
            ? option.rect.left() + s_lateralMargin + s_iconLeftPad
            : option.rect.right() - s_lateralMargin - s_iconLeftPad - iconSize;
    painter->drawPixmap(iconX, itemTop + (stdH - iconSize) / 2, pm);

    const int textLeft = (isLTR ? iconAreaW : 0) + s_lateralMargin + s_extraGap;
    const int textWidth = option.rect.width() - iconAreaW -
                          2 * s_lateralMargin - s_extraGap - s_iconLeftPad;

    painter->save();
    QFont textFont = option.font;
    if (selected)
      textFont.setBold(true);
    painter->setFont(textFont);
    painter->setPen(selected
                        ? option.palette.color(QPalette::Active,
                                               QPalette::Highlight)
                        : option.palette.text().color());

    const bool hasCapacityBar = index.data(s_capacityBarRole).toBool();
    if (hasCapacityBar) {
      const int barH = static_cast<int>(std::ceil(iconSize / 6.0));
      const int textH = option.fontMetrics.height();
      const int textY = itemTop + (stdH - textH - barH - 2) / 2;
      const QRect textRect(textLeft, textY, textWidth, textH);
      painter->drawText(
          textRect, Qt::AlignLeft | Qt::AlignVCenter,
          option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(),
                                        Qt::ElideRight, textWidth));
      painter->restore();

      const QPersistentModelIndex pIdx(index);
      const QUrl url = index.data(s_urlRole).toUrl();
      if (!m_freeSpaceCache.contains(pIdx) && url.isLocalFile()) {
        const QStorageInfo si(url.toLocalFile());
        if (si.isValid() && si.bytesTotal() > 0)
          m_freeSpaceCache[pIdx] = {
              quint64(si.bytesTotal() - si.bytesAvailable()),
              quint64(si.bytesTotal())};
      }
      const auto it = m_freeSpaceCache.constFind(pIdx);
      if (it != m_freeSpaceCache.constEnd() && it->size > 0)
        drawCapacityBar(painter, option,
                        QRect(textLeft, textY, textWidth, textH),
                        it->used / qreal(it->size), iconSize);
      painter->restore();
      return;
    } else {
      painter->drawText(
          QRect(textLeft, itemTop, textWidth, stdH),
          Qt::AlignLeft | Qt::AlignVCenter,
          option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(),
                                        Qt::ElideRight, textWidth));
      painter->restore();
    }

    painter->restore();
  }

  bool eventFilter(QObject *watched, QEvent *event) override {
    if (!m_view || watched != m_view->viewport() || m_redirectingEvent)
      return QStyledItemDelegate::eventFilter(watched, event);

    const auto type = event->type();

    // handle cursor: show pointer when mouse is over an item or in small gaps between items
    if (type == QEvent::MouseMove || type == QEvent::HoverMove || type == QEvent::HoverEnter) {
      const QPoint pos =
          type == QEvent::MouseMove
              ? static_cast<QMouseEvent *>(event)->pos()
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              : static_cast<QHoverEvent *>(event)->position().toPoint();
#else
              : static_cast<QHoverEvent *>(event)->pos();
#endif
      const QModelIndex idx = m_view->indexAt(pos);
      bool onItem = false;
      if (idx.isValid()) {
        const QRect vr = m_view->visualRect(idx);
        const int iconSize = m_view->iconSize().width();
        const int stdH = qMax(iconSize, m_view->fontMetrics().height()) + s_lateralMargin;
        if (vr.height() > stdH) {
          // match the item rect from paint(): headerH + s_itemGap above, item fills bottom
          const int headerH = qMax(0, vr.height() - stdH - 2 * s_itemGap);
          const QRect itemR(vr.left(), vr.top() + headerH + s_itemGap, vr.width(), stdH);
          onItem = itemR.contains(pos);
          if (!onItem && pos.y() < itemR.top() && pos.y() >= vr.top()) {
            // above item rect: only treat as clickable if it's small padding, not a section header
            if (headerH <= s_heightExtra)
              onItem = true;
          } else if (!onItem && pos.y() > itemR.bottom() && pos.y() <= vr.bottom()) {
            // below item rect: small bottom padding, treat as clickable
            onItem = true;
          }
        } else {
          onItem = vr.contains(pos);
        }
        // section headers have no icon and should not be clickable
        if (onItem && idx.data(Qt::DecorationRole).value<QIcon>().isNull())
          onItem = false;
      } else {
        // in the spacing between items, redirect to the nearest item
        onItem = nearestItemAt(pos).isValid();
      }
      static_cast<QWidget *>(watched)->setCursor(onItem ? Qt::PointingHandCursor : Qt::ArrowCursor);
    } else if (type == QEvent::HoverLeave) {
      static_cast<QWidget *>(watched)->setCursor(Qt::ArrowCursor);
      if (m_lastHoveredIndex.isValid()) {
        const QModelIndex oldHover = m_lastHoveredIndex;
        m_lastHoveredIndex = QPersistentModelIndex();
        m_lastHoverOnItem = false;
        m_view->update(oldHover);
      }
    }

    if (type == QEvent::MouseMove) {
      const auto *me = static_cast<QMouseEvent *>(event);
      const QPoint pos = me->pos();
      if (m_hasRedirectedPress) {
        sendMouseRedirected(watched, me, m_redirectedPressPos);
        updateHoverBoundary(m_redirectedPressPos);
        return true;
      }
      updateHoverBoundary(pos);
      if (!m_view->indexAt(pos).isValid()) {
        const QModelIndex nearest = nearestItemAt(pos);
        if (nearest.isValid()) {
          const QPoint target = snapToItem(pos, nearest);
          sendMouseRedirected(watched, me, target);
          updateHoverBoundary(target);
          return true;
        }
      }
    } else if (type == QEvent::MouseButtonPress) {
      const auto *me = static_cast<QMouseEvent *>(event);
      if (me->button() == Qt::LeftButton) {
        QModelIndex pressedIdx = m_view->indexAt(me->pos());
        if (!pressedIdx.isValid())
          pressedIdx = nearestItemAt(me->pos());
        m_dragSourceIndex = QPersistentModelIndex(pressedIdx);
        if (!m_view->indexAt(me->pos()).isValid()) {
          const QModelIndex nearest = nearestItemAt(me->pos());
          if (nearest.isValid()) {
            m_redirectedPressPos = snapToItem(me->pos(), nearest);
            m_hasRedirectedPress = true;
            sendMouseRedirected(watched, me, m_redirectedPressPos);
            return true;
          }
        }
      }
    } else if (type == QEvent::MouseButtonRelease) {
      m_dragging = false;
      m_dragSourceIndex = QPersistentModelIndex();
      if (m_hasRedirectedPress) {
        const auto *me = static_cast<QMouseEvent *>(event);
        sendMouseRedirected(watched, me, m_redirectedPressPos);
        m_hasRedirectedPress = false;
        return true;
      }
    } else if (type == QEvent::DragEnter || type == QEvent::DragMove) {
      m_dragging = true;
      const QPoint pos =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
          static_cast<QDragMoveEvent *>(event)->position().toPoint();
#else
          static_cast<QDragMoveEvent *>(event)->pos();
#endif
      if (const QModelIndex idx = m_view->indexAt(pos); idx.isValid())
        m_view->update(idx);
      if (m_dragSourceIndex.isValid())
        m_view->update(m_dragSourceIndex);
    } else if (type == QEvent::Drop || type == QEvent::DragLeave) {
      m_dragging = false;
      m_dragSourceIndex = QPersistentModelIndex();
      m_view->viewport()->update();
    }

    return QStyledItemDelegate::eventFilter(watched, event);
  }

private:
  void drawCapacityBar(QPainter *painter, const QStyleOptionViewItem &option,
                       const QRect &textRect, qreal usedSpace,
                       int iconSize) const {
    const qreal barHeight = std::ceil(iconSize / 6.0);
    const qreal radius = barHeight / 2.0;
    const QRectF bgRect(textRect.x(), textRect.bottom() + 2.0, textRect.width(),
                        barHeight);
    QRectF fillRect = bgRect;
    fillRect.setWidth(bgRect.width() * usedSpace);

    const bool selected = option.state & QStyle::State_Selected;
    QColor bgColor(
        option.palette.color(QPalette::Active, QPalette::WindowText));
    bgColor.setAlphaF(0.2);

    QColor fillColor;
    const int pct = qRound(usedSpace * 100);
    if (pct >= BlossomUI::StyleConfigData::capacityBarCritThreshold())
      fillColor = BlossomUI::StyleConfigData::capacityBarCritColor();
    else if (pct >= BlossomUI::StyleConfigData::capacityBarWarnThreshold())
      fillColor = BlossomUI::StyleConfigData::capacityBarWarnColor();
    else
      fillColor = selected ? option.palette.color(QPalette::Active,
                                                  QPalette::HighlightedText)
                           : option.palette.color(QPalette::Active,
                                                  QPalette::Highlight);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->drawRoundedRect(bgRect, radius, radius);
    painter->setBrush(fillColor);
    painter->drawRoundedRect(fillRect, radius, radius);
    painter->restore();
  }

  QPoint snapToItem(const QPoint &pos, const QModelIndex &idx) const {
    return QPoint(pos.x(), m_view->visualRect(idx).center().y());
  }

  void sendMouseRedirected(QObject *watched, const QMouseEvent *original,
                           const QPoint &target) const {
    const QPointF globalTarget = m_view->viewport()->mapToGlobal(target);
    m_redirectingEvent = true;
    QMouseEvent redirected(original->type(), QPointF(target), globalTarget,
                           original->button(), original->buttons(),
                           original->modifiers());
    QApplication::sendEvent(watched, &redirected);
    m_redirectingEvent = false;
  }

  QModelIndex nearestItemAt(const QPoint &pos) const {
    const QAbstractItemModel *model = m_view ? m_view->model() : nullptr;
    if (!model)
      return {};
    QModelIndex best;
    int bestDist = INT_MAX;
    for (int row = 0, rows = model->rowCount(); row < rows; ++row) {
      const QModelIndex idx = model->index(row, 0);
      if (idx.data(Qt::DecorationRole).value<QIcon>().isNull())
        continue;
      const QRect vr = m_view->visualRect(idx);
      if (!vr.isValid())
        continue;
      const int dist = pos.y() < vr.top()      ? vr.top() - pos.y()
                       : pos.y() > vr.bottom() ? pos.y() - vr.bottom()
                                               : 0;
      if (dist < bestDist) {
        bestDist = dist;
        best = idx;
      }
    }
    return best;
  }

  void updateHoverBoundary(const QPoint &pos) {
    if (!m_view)
      return;
    const QModelIndex idx = m_view->indexAt(pos);
    if (!idx.isValid()) {
      const QModelIndex nearest = nearestItemAt(pos);
      if (nearest.isValid()) {
        const QPersistentModelIndex pidx(nearest);
        if (pidx != m_lastHoveredIndex || m_lastHoverOnItem != true) {
          const QModelIndex oldHover = m_lastHoveredIndex;
          m_lastHoveredIndex = pidx;
          m_lastHoverOnItem = true;
          if (oldHover.isValid())
            m_view->update(oldHover);
          m_view->update(nearest);
        }
      } else {
        if (m_lastHoveredIndex.isValid()) {
          const QModelIndex oldHover = m_lastHoveredIndex;
          m_lastHoveredIndex = QPersistentModelIndex();
          m_lastHoverOnItem = false;
          m_view->update(oldHover);
        }
      }
      return;
    }
    const bool idxHasIcon = !idx.data(Qt::DecorationRole).value<QIcon>().isNull();
    const QRect vr = m_view->visualRect(idx);
    const int iconSize = m_view->iconSize().width();
    const int stdH =
        qMax(iconSize, m_view->fontMetrics().height()) + s_lateralMargin;
    bool onItem;
    if (vr.height() > stdH) {
      const int headerH = qMax(0, vr.height() - stdH - 2 * s_itemGap);
      const QRect itemR(vr.left(), vr.top() + headerH + s_itemGap, vr.width(), stdH);
      onItem = itemR.contains(pos);
      if (!onItem && pos.y() < itemR.top() && pos.y() >= vr.top()) {
        if (headerH <= s_heightExtra)
          onItem = true;
      } else if (!onItem && pos.y() > itemR.bottom() && pos.y() <= vr.bottom()) {
        onItem = true;
      }
    } else {
      onItem = vr.contains(pos);
    }
    // section headers have no icon, clear hover when over them
    if (onItem && !idxHasIcon)
      onItem = false;
    // when in the header/padding area of an icon item, or on a section header, clear hover
    if (!onItem) {
      if (m_lastHoveredIndex.isValid()) {
        const QModelIndex oldHover = m_lastHoveredIndex;
        m_lastHoveredIndex = QPersistentModelIndex();
        m_lastHoverOnItem = false;
        m_view->update(oldHover);
      }
      return;
    }
    const QPersistentModelIndex pidx(idx);
    if (pidx != m_lastHoveredIndex || onItem != m_lastHoverOnItem) {
      const QModelIndex oldHover = m_lastHoveredIndex;
      m_lastHoveredIndex = pidx;
      m_lastHoverOnItem = onItem;
      if (oldHover.isValid())
        m_view->update(oldHover);
      m_view->update(idx);
    }
  }

  QPointer<QAbstractItemView> m_view;
  QPointer<QAbstractItemDelegate> m_original;
  mutable QHash<QPersistentModelIndex, FreeSpaceInfo> m_freeSpaceCache;
  mutable bool m_dragging = false;
  mutable QPersistentModelIndex m_dragSourceIndex;
  mutable bool m_hasRedirectedPress = false;
  mutable QPoint m_redirectedPressPos;
  mutable bool m_redirectingEvent = false;
  mutable QPersistentModelIndex m_lastHoveredIndex;
  mutable bool m_lastHoverOnItem = false;
};

} // namespace BlossomUIPrivate
