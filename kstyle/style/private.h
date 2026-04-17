#pragma once

#include "blossomui.h"
#include "blossomuistyleconfigdata.h"

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QCursor>
#include <QDragMoveEvent>
#include <QHash>
#include <QIcon>
#include <QItemDelegate>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QSet>
#include <QStorageInfo>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QWidget>
#include <QWindow>

namespace BlossomUIPrivate {

//* declared in blossomuistyle.cpp – shared across all split TUs
extern QSet<const QWidget *> possibleTranslucentToolBars;

inline bool isProgressBarHorizontal(const QStyleOptionProgressBar *option) {
  return option && (option->state & QStyle::State_Horizontal);
}

// tracks the tabbar currently being dragged so hover/animation state stays
// consistent
class TabBarData : public QObject {
public:
  explicit TabBarData(QObject *parent) : QObject(parent) {}

  void lock(const QWidget *widget) { _tabBar = widget; }
  bool isLocked(const QWidget *widget) const {
    return _tabBar && _tabBar.data() == widget;
  }
  void release() { _tabBar.clear(); }

private:
  BlossomUI::WeakPointer<const QWidget> _tabBar;
};

// adds vertical padding to combobox items and defers to any custom delegate the
// app installed
class ComboBoxItemDelegate : public QItemDelegate {
public:
  explicit ComboBoxItemDelegate(QAbstractItemView *parent)
      : QItemDelegate(parent), _proxy(parent->itemDelegate()),
        _itemMargin(BlossomUI::Metrics::ItemView_ItemMarginWidth) {}

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    painter->setRenderHint(QPainter::Antialiasing);
    if (_proxy &&
        _proxy->metaObject()->className() != QByteArray("QComboBoxDelegate") &&
        _proxy->metaObject()->className() !=
            QByteArray("QStyledItemDelegate")) {
      _proxy.data()->paint(painter, option, index);
      return;
    }

    if (option.showDecorationSelected &&
        (option.state & QStyle::State_Selected)) {
      QPalette::ColorGroup group = (option.state & QStyle::State_Enabled)
                                       ? QPalette::Active
                                       : QPalette::Disabled;
      QColor c = option.palette.brush(group, QPalette::Highlight).color();
      painter->setPen(c);
      c.setAlphaF(c.alphaF() * 0.3);
      painter->setBrush(c);
      const qreal radius = BlossomUI::StyleConfigData::itemViewRadius();
      painter->drawRoundedRect(
          QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
    }

    QStyleOptionViewItem opt = option;
    opt.showDecorationSelected = false;
    opt.state &= ~QStyle::State_Selected;
    QItemDelegate::paint(painter, opt, index);
  }

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override {
    QSize size(_proxy ? _proxy.data()->sizeHint(option, index)
                      : QItemDelegate::sizeHint(option, index));
    if (size.isValid())
      size.rheight() += _itemMargin * 2;
    return size;
  }

private:
  BlossomUI::WeakPointer<QAbstractItemDelegate> _proxy;
  int _itemMargin;
};

} // namespace BlossomUIPrivate
