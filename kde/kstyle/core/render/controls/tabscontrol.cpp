// SPDX-License-Identifier: GPL-2.0-or-later
#include "tabscontrol.h"
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "frame.h"
#include "private.h"
#include "tabbar.h"

#include <KColorUtils>
#include <QAbstractButton>
#include <QApplication>
#include <QDockWidget>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionDockWidget>
#include <QStyleOptionTab>
#include <QStyleOptionTabBarBase>
#include <QStyleOptionToolBox>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBox>

namespace BlossomUI {

bool Render::TabsControl::drawTabBarTabLabelControl(const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const {
  // code is copied from QCommonStyle
  // cast option and check
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));

  if (tabOption) {
    QRect tr = tabOption->rect;
    bool verticalTabs = tabOption->shape == QTabBar::RoundedEast ||
                        tabOption->shape == QTabBar::RoundedWest ||
                        tabOption->shape == QTabBar::TriangularEast ||
                        tabOption->shape == QTabBar::TriangularWest;

    int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
    if (_style->styleHint(QStyle::SH_UnderlineShortcut, option, widget))
      alignment |= Qt::TextHideMnemonic;

    if (verticalTabs) {
      painter->save();
      int newX, newY, newRot;
      if (tabOption->shape == QTabBar::RoundedEast ||
          tabOption->shape == QTabBar::TriangularEast) {
        newX = tr.width() + tr.x();
        newY = tr.y();
        newRot = 90;
      } else {
        newX = tr.x();
        newY = tr.y() + tr.height();
        newRot = -90;
      }
      QTransform m = QTransform::fromTranslate(newX, newY);
      m.rotate(newRot);
      painter->setTransform(m, true);
    }
    QRect iconRect;
    _style->tabLayout(tabOption, widget, &tr, &iconRect);
    tr = _style->subElementRect(QStyle::SE_TabBarTabText, option, widget);

    const bool isSelected = tabOption->state & QStyle::State_Selected;
    const bool isEnabled = tabOption->state & QStyle::State_Enabled;

    if (!tabOption->icon.isNull()) {
      QSize iconSize = tabOption->iconSize;
      if (!iconSize.isValid()) {
        const int e = _style->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
        iconSize = QSize(e, e);
      }
      const QIcon::Mode iconMode =
          isEnabled ? QIcon::Normal : QIcon::Disabled;
      const QIcon::State iconState = isSelected ? QIcon::On : QIcon::Off;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      const qreal dpr = painter->device()
                            ? painter->device()->devicePixelRatioF()
                            : qApp->devicePixelRatio();
      QPixmap tabIcon =
          tabOption->icon.pixmap(iconSize, dpr, iconMode, iconState);
#else
      QPixmap tabIcon = tabOption->icon.pixmap(iconSize, iconMode, iconState);
#endif
      if (isSelected && isEnabled) {
        tabIcon = tabIcon.copy();
        QPainter tp(&tabIcon);
        tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tp.fillRect(tabIcon.rect(),
                    option->palette.brush(QPalette::Highlight).color());
      }

      const int leftPad = 10;
      const QRect tabR = tabOption->rect;
      QRect drawRect(tabR.left() + leftPad,
                     tabR.center().y() - iconSize.height() / 2 + 1,
                     iconSize.width(), iconSize.height());
      drawRect = QStyle::visualRect(tabOption->direction, tabR, drawRect);
      _style->proxy()->drawItemPixmap(painter, drawRect, Qt::AlignCenter, tabIcon);

      const int newTextLeft = drawRect.right() + 6;
      if (tabOption->direction == Qt::LeftToRight) {
        if (newTextLeft > tr.left())
          tr.setLeft(newTextLeft);
      } else {
        const int newTextRight = drawRect.left() - 6;
        if (newTextRight < tr.right())
          tr.setRight(newTextRight);
      }
    }

    QFont font = painter->font();
    if (isSelected)
      font.setBold(true);
    painter->setFont(font);

    if (!isEnabled) {
      if (isSelected)
        painter->setPen(option->palette.brush(QPalette::Highlight).color());
      else
        painter->setPen(_style->_helper->alphaColor(
            option->palette.brush(QPalette::WindowText).color(), 0.5));
    } else {
      if (isSelected)
        painter->setPen(option->palette.brush(QPalette::Highlight).color());
      else if (tabOption->state & QStyle::State_Active &&
               tabOption->state & QStyle::State_MouseOver)
        painter->setPen(option->palette.brush(QPalette::WindowText).color());
      else
        painter->setPen(_style->_helper->alphaColor(
            option->palette.brush(QPalette::WindowText).color(), 0.75));
    }

    _style->proxy()->drawItemText(painter, tr, alignment, tabOption->palette, isEnabled,
                          tabOption->text, QPalette::NoRole);
    if (verticalTabs)
      painter->restore();
  }

  // store rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // check focus
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool selected(state & QStyle::State_Selected);
  const bool hasFocus(enabled && selected && (state & QStyle::State_HasFocus));

  // update mouse over animation state
  _style->_animations->tabBarEngine().updateState(widget, rect.topLeft(),
                                          AnimationFocus, hasFocus);
  const bool animated(enabled && selected &&
                      _style->_animations->tabBarEngine().isAnimated(
                          widget, rect.topLeft(), AnimationFocus));
  const qreal opacity(_style->_animations->tabBarEngine().opacity(
      widget, rect.topLeft(), AnimationFocus));

  if (!(hasFocus || animated))
    return true;
  if (!tabOption || tabOption->text.isEmpty())
    return true;

  // tab option rect
  const bool verticalTabs(_style->isVerticalTab(tabOption));
  const int textFlags(Qt::AlignCenter | _style->_mnemonics->textFlags());

  // text rect
  auto textRect(_style->subElementRect(QStyle::SE_TabBarTabText, option, widget));

  if (verticalTabs) {
    // properly rotate painter
    painter->save();
    int newX, newY, newRot;
    if (tabOption->shape == QTabBar::RoundedEast ||
        tabOption->shape == QTabBar::TriangularEast) {
      newX = rect.width() + rect.x();
      newY = rect.y();
      newRot = 90;

    } else {
      newX = rect.x();
      newY = rect.y() + rect.height();
      newRot = -90;
    }

    QTransform transform;
    transform.translate(newX, newY);
    transform.rotate(newRot);
    painter->setTransform(transform, true);
  }

  // adjust text rect based on font metrics
  textRect =
      option->fontMetrics.boundingRect(textRect, textFlags, tabOption->text);

  // focus color
  QColor focusColor;
  if (animated)
    focusColor = _style->_helper->alphaColor(_style->_helper->focusColor(palette), opacity);
  else if (hasFocus)
    focusColor = _style->_helper->focusColor(palette);

  // render focus line
  // _style->_helper->renderFocusLine( painter, textRect, focusColor );

  if (verticalTabs)
    painter->restore();

  return true;
}

bool Render::TabsControl::drawTabBarTabShapeControl(const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const {
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));
  if (!tabOption)
    return true;

  // palette and state
  const auto &palette(option->palette);
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool selected(state & QStyle::State_Selected);
  const bool mouseOver(enabled && !selected && (state & QStyle::State_MouseOver));

  const bool windowActive(widget && widget->isActiveWindow());

  // check if tab is being dragged
  const bool isDragged(widget && selected && painter->device() != widget);
  const bool isLocked(widget && _style->_tabBarData->isLocked(widget));

  // store rect
  auto rect(option->rect);

  // update mouse over animation state
  _style->_animations->tabBarEngine().updateState(widget, rect.topLeft(),
                                          AnimationHover, mouseOver);
  const bool animated(enabled && !selected &&
                      _style->_animations->tabBarEngine().isAnimated(
                          widget, rect.topLeft(), AnimationHover));
  const qreal opacity(_style->_animations->tabBarEngine().opacity(
      widget, rect.topLeft(), AnimationHover));

  // lock state
  if (selected && widget && isDragged)
    _style->_tabBarData->lock(widget);
  else if (widget && selected && _style->_tabBarData->isLocked(widget))
    _style->_tabBarData->release();

  // tab position
  const QStyleOptionTab::TabPosition &position = tabOption->position;
  const bool isSingle(position == QStyleOptionTab::OnlyOneTab);
  const bool isQtQuickControl(_style->isQtQuickControl(option, widget));
  bool isFirst(isSingle || position == QStyleOptionTab::Beginning);
  bool isLast(isSingle || position == QStyleOptionTab::End);
  bool isLeftOfSelected(!isLocked && tabOption->selectedPosition ==
                                         QStyleOptionTab::NextIsSelected);
  bool isRightOfSelected(!isLocked && tabOption->selectedPosition ==
                                          QStyleOptionTab::PreviousIsSelected);

  // true if widget is aligned to the frame
  // need to check for 'isRightOfSelected' because for some reason the isFirst
  // flag is set when active tab is being moved
  isFirst &= !isRightOfSelected;
  isLast &= !isLeftOfSelected;

  // swap state based on reverse layout, so that they become layout independent
  const bool reverseLayout(option->direction == Qt::RightToLeft);
  const bool verticalTabs(_style->isVerticalTab(tabOption));
  if ((reverseLayout && !verticalTabs) || _style->_app.isLibreoffice) {
    qSwap(isFirst, isLast);
    qSwap(isLeftOfSelected, isRightOfSelected);
  }

  // overlap
  // for QtQuickControls, ovelap is already accounted of in the option. Unlike
  // in the qwidget case
  const int overlap(isQtQuickControl ? 0 : Render::TabBar_TabOverlap);

  // define if tab is fixed or mutable
  bool documentMode = tabOption->documentMode;
  // flag passed to QStyleOptionTab is unfortunately not reliable enough
  // also need to check on parent widget
  const auto tabWidget =
      (widget && widget->parentWidget())
          ? qobject_cast<const QTabWidget *>(widget->parentWidget())
          : nullptr;
  documentMode |= (tabWidget ? tabWidget->documentMode() : true);

  // define the 'tabbar' background color
  QColor configTabBgColor =
      StyleConfigData::adjustToDarkThemes()
          ? QColor(StyleConfigData::tabBGColor() /*0, 0, 0, 160*/)
          : palette.color(QPalette::Shadow);
  QColor backgroundColor;
  const bool isDark = _style->_helper->isDarkTheme(palette);
  if (StyleConfigData::documentModeTabs())
    backgroundColor = documentMode
                          ? isDark
                                ? palette.color(QPalette::Base)
                                : _style->_helper->alphaColor(configTabBgColor, 0.2)
                          : _style->_helper->alphaColor(configTabBgColor, 0.1);
  else
    backgroundColor = isDark
                          ? palette.color(QPalette::Base)
                          : _style->_helper->alphaColor(configTabBgColor, 0.2);

  if ((_style->_app.isDolphin || _style->_app.isKonsole) &&
      (StyleConfigData::tabBarOpacity() < 100) && !_style->_app.isOpaque) {
    // override the tab background color if adjustToDarkThemes is false
    if (StyleConfigData::adjustToDarkThemes()) {
      backgroundColor = _style->_helper->transparentBarBgColor(backgroundColor, painter,
                                                       rect, BarType::TabBar);
    } else {
      backgroundColor = _style->_helper->transparentBarBgColor(
          _style->_toolsAreaManager->palette().color(QPalette::Window), painter, rect,
          BarType::TabBar);
    }
  }

  // shadow size
  constexpr int shadowSize = 4;
  QRect shadowRect;
  QRect backgroundRect;

  const QColor color =
      Render::tabFill(palette, selected, mouseOver, animated, opacity).brush.color();

  // adjust rect and define corners based on tabbar orientation
  Corners corners;
  Corners backgroundCorners;

  // store tab shape
  Side side = SideTop;

  switch (tabOption->shape) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:

    side = SideTop;
    if (selected) {
      corners = CornerTopLeft | CornerTopRight | CornerBottomLeft |
                CornerBottomRight;
    }

    shadowRect = QRect(rect.bottomLeft() + QPoint(0, 1),
                       QSize(rect.width(), shadowSize));
    if (isFirst)
      backgroundCorners |= CornerTopLeft | CornerBottomLeft;
    else
      shadowRect.adjust(-shadowSize, 0, 0, 0);
    if (isLast)
      backgroundCorners |= CornerTopRight | CornerBottomRight;
    if (isLeftOfSelected)
      shadowRect.adjust(0, 0, shadowSize, 0);

    backgroundRect = rect;
    rect.adjust(4, 4, -4, -4);
    break;

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:

    side = SideBottom;
    if (selected)
      corners = CornerTopLeft | CornerTopRight | CornerBottomLeft |
                CornerBottomRight;

    shadowRect = QRect(rect.topLeft() - QPoint(0, shadowSize + 1),
                       QSize(rect.width(), shadowSize));
    if (isFirst)
      backgroundCorners |= CornerBottomLeft | CornerTopLeft;
    else
      shadowRect.adjust(-shadowSize, 0, 0, 0);
    if (isLast)
      backgroundCorners |= CornerBottomRight | CornerTopRight;

    backgroundRect = rect;
    rect.adjust(4, 4, -4, -4);
    break;

  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:

    side = SideLeft;
    if (selected) {
      corners = CornerTopLeft | CornerTopRight | CornerBottomLeft |
                CornerBottomRight;

    } else {
      rect.adjust(4, 4, -4, -4);
      if (isFirst)
        corners |= CornerTopLeft | CornerTopRight;
      if (isLast)
        corners |= CornerBottomLeft | CornerBottomRight;
      if (isRightOfSelected)
        rect.adjust(0, -StyleConfigData::cornerRadius(), 0, 0);
      if (isLeftOfSelected)
        rect.adjust(0, 0, 0, StyleConfigData::cornerRadius());
      else if (!isLast)
        rect.adjust(0, 0, 0, overlap);
    }
    break;

  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:

    side = SideRight;
    if (selected) {
      corners = CornerTopLeft | CornerTopRight | CornerBottomLeft |
                CornerBottomRight;

    } else {
      rect.adjust(4, 4, -4, -4);
      if (isFirst)
        corners |= CornerTopRight | CornerTopLeft;
      if (isLast)
        corners |= CornerBottomRight | CornerBottomLeft;
      if (isRightOfSelected)
        rect.adjust(0, -StyleConfigData::cornerRadius(),
                    -StyleConfigData::cornerRadius(), 0);
      if (isLeftOfSelected)
        rect.adjust(0, -StyleConfigData::cornerRadius(), 0,
                    StyleConfigData::cornerRadius());
      else if (!isLast)
        rect.adjust(0, 0, 0, overlap);
    }
    break;

  default:
    break;
  }

  const bool unifiedTabAndHeader =
      _style->_app.isKonsole && StyleConfigData::unifiedTabBarKonsole();
  if (documentMode && unifiedTabAndHeader &&
      _style->_helper->titleBarColor(true).alphaF() < 1.0) {
    _style->_helper->renderTransparentArea(painter, backgroundRect);
    painter->fillRect(backgroundRect,
                      QBrush(_style->_helper->titleBarColor(windowActive)));
  }

  // render
  if (selected) {
    QRegion oldRegion(painter->clipRegion());

    // thin light border on selected tab (mockup: distinct border, subtle
    // shadow)
    const QColor tabOutlineColor(_style->_helper->isDarkTheme(palette)
                                     ? _style->_helper->alphaColor(Qt::white, 0.10)
                                     : _style->_helper->alphaColor(Qt::black, 0.12));
    const qreal tabRadius(_style->_helper->tabFrameRadius());
    // sliding pill animation: use animated rect when selected tab is changing
    const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget);
    const bool pillAnimated =
        tabBar && documentMode &&
        _style->_animations->tabBarEngine().isSelectedAnimated(tabBar);
    const QRect pillRect =
        pillAnimated
            ? _style->_animations->tabBarEngine()
                  .selectedPillRect(tabBar, const_cast<QTabBar *>(tabBar))
                  .adjusted(4, 4, -4, -4)
            : rect;

    // when pill is animating, clip to whole bar so pill can slide across
    painter->setClipRect(pillAnimated && tabBar ? tabBar->rect() : option->rect,
                         Qt::IntersectClip);

    if (documentMode) {
      // konsole's tab
      if (unifiedTabAndHeader) {
        // reajust rect to flush tab with window border
        if (isFirst)
          rect.adjust(-3, 0, 0, 0);
        else if (isLast)
          rect.adjust(0, 0, 3, 0);
        if (color.alphaF() < 1)
          rect.adjust(0, side == SideTop ? -3 : 0, 0,
                      side == SideBottom ? 3 : 0);

        // erase alpha
        painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
        _style->_helper->renderTabBarTab(painter, pillRect, Qt::black, AllCorners,
                                 tabRadius);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        // no shadow for translucent tab
        if (!(color.alphaF() < 1))
          _style->_helper->renderBoxShadow(painter, pillRect, 0, 1, 2,
                                   _style->_helper->shadowColor(palette, 80, 28),
                                   qRound(tabRadius), true);
        _style->_helper->renderTabBarTab(painter, pillRect, color, AllCorners,
                                 tabRadius);
        if (color.alphaF() >= 1)
          _style->_helper->renderTabBarTabOutline(painter, pillRect, tabOutlineColor,
                                          AllCorners, tabRadius);
      } else {
        _style->_helper->renderTabBarTab(painter, backgroundRect, backgroundColor,
                                 backgroundCorners);
        _style->_helper->renderBoxShadow(painter, shadowRect, 0, 1, shadowSize,
                                 _style->_helper->shadowColor(palette, 220, 75),
                                 StyleConfigData::cornerRadius(), true);

        if (color.alphaF() >= 1) {
          _style->_helper->renderBoxShadow(painter, pillRect, 0, 1, 2,
                                   _style->_helper->shadowColor(palette, 80, 28),
                                   qRound(tabRadius), true);
        }
        _style->_helper->renderTabBarTab(
            painter, pillRect,
            _style->_app.isLibreoffice ? palette.color(QPalette::Highlight) : color,
            pillAnimated ? AllCorners : corners, tabRadius);
        if (color.alphaF() >= 1) {
          _style->_helper->renderTabBarTabOutline(painter, pillRect, tabOutlineColor,
                                          pillAnimated ? AllCorners : corners,
                                          tabRadius);
        }
      }

      // highlight (slides with pill, uses tab radius)
      if (StyleConfigData::tabDrawHighlight()) {
        constexpr int hh = 2; // highlight height
        const QRect &hlRect = pillRect;

        // render only whats inside the highlight rect
        painter->setClipRect(
            hlRect.adjusted(side == SideRight ? hlRect.width() - hh : 0,
                            side == SideBottom ? hlRect.height() - hh : 0,
                            side == SideLeft ? -hlRect.width() + hh : 0,
                            side == SideTop ? -hlRect.height() + hh : 0),
            Qt::IntersectClip);

        // when the highlight is too thin, it's not rendered properly,
        // so we increase its size by one, the extra part will not be rendered
        // anyway.
        _style->_helper->renderTabBarTab(
            painter,
            hlRect.adjusted(side == SideRight ? hlRect.width() - (hh + 1) : 0,
                            side == SideBottom ? hlRect.height() - (hh + 1) : 0,
                            side == SideLeft ? -hlRect.width() + (hh + 1) : 0,
                            side == SideTop ? -hlRect.height() + (hh + 1) : 0),
            _style->_helper->focusColor(palette), pillAnimated ? AllCorners : corners,
            tabRadius);
      }

    }

    else {
      _style->_helper->renderTabBarTab(painter, backgroundRect, backgroundColor,
                               backgroundCorners);
      painter->setRenderHint(QPainter::Antialiasing, true);
      painter->setPen(Qt::NoPen);
      if (StyleConfigData::documentModeTabs())
        backgroundRect.adjust(5, 6, -5, -6);
      else
        backgroundRect.adjust(4, 4, -4, -4);
      if (color.alphaF() >= 1) {
        _style->_helper->renderBoxShadow(painter, backgroundRect, 0, 1, 2,
                                 _style->_helper->shadowColor(palette, 60, 20),
                                 qRound(tabRadius), true);
      }
      painter->setBrush(color);
      painter->drawRoundedRect(backgroundRect, tabRadius, tabRadius);

      if (!StyleConfigData::tabUseHighlightColor() && color.alphaF() >= 1) {
        if (StyleConfigData::documentModeTabs())
          painter->setBrush(QColor(255, 255, 255, 20));
        painter->drawRoundedRect(backgroundRect, tabRadius, tabRadius);
      }
      if (color.alphaF() >= 1) {
        _style->_helper->renderTabBarTabOutline(painter, backgroundRect,
                                        tabOutlineColor, AllCorners, tabRadius);
      }
    }

    painter->setClipRegion(oldRegion);

  } else {
    _style->_helper->renderTabBarTab(painter, backgroundRect, backgroundColor,
                             backgroundCorners);
    if (mouseOver) {
      if (documentMode) {
        backgroundRect.adjust(4, 4, -4, -4);
      } else {
        if (StyleConfigData::documentModeTabs())
          backgroundRect.adjust(5, 6, -5, -6);
        else
          backgroundRect.adjust(4, 4, -4, -4);
      }
      painter->setBrush(palette.color(QPalette::Button));
      painter->setRenderHint(QPainter::Antialiasing, true);
      painter->setPen(Qt::NoPen);
      const qreal hoverRadius(_style->_helper->tabFrameRadius());
      painter->drawRoundedRect(backgroundRect, hoverRadius, hoverRadius);
    }
  }

  return true;
}

bool Render::TabsControl::drawToolBoxTabLabelControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // rendering is similar to drawPushButtonLabelControl
  // cast option and check
  const auto toolBoxOption(
      qstyleoption_cast<const QStyleOptionToolBox *>(option));
  if (!toolBoxOption)
    return true;

  // copy palette
  const auto &palette(option->palette);

  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);

  // text alignment
  const int textFlags(_style->_mnemonics->textFlags() | Qt::AlignCenter);

  // contents rect
  const auto rect(_style->subElementRect(QStyle::SE_ToolBoxTabContents, option, widget));

  // store icon size
  const int iconSize(_style->pixelMetric(QStyle::PM_SmallIconSize, option, widget));

  // find contents size and rect
  auto contentsRect(rect);
  QSize contentsSize;
  if (!toolBoxOption->text.isEmpty()) {
    contentsSize =
        option->fontMetrics.size(_style->_mnemonics->textFlags(), toolBoxOption->text);
    if (!toolBoxOption->icon.isNull())
      contentsSize.rwidth() += Render::ToolBox_TabItemSpacing;
  }

  // icon size
  if (!toolBoxOption->icon.isNull()) {
    contentsSize.setHeight(qMax(contentsSize.height(), iconSize));
    contentsSize.rwidth() += iconSize;
  }

  // adjust contents rect
  contentsRect = _style->centerRect(contentsRect, contentsSize);

  // render icon
  if (!toolBoxOption->icon.isNull()) {
    // icon rect
    QRect iconRect;
    if (toolBoxOption->text.isEmpty())
      iconRect = _style->centerRect(contentsRect, iconSize, iconSize);
    else {
      iconRect = contentsRect;
      iconRect.setWidth(iconSize);
      iconRect = _style->centerRect(iconRect, iconSize, iconSize);
      contentsRect.setLeft(iconRect.right() + Render::ToolBox_TabItemSpacing +
                           1);
    }

    iconRect = _style->visualRect(option, iconRect);
    const QIcon::Mode mode(enabled ? QIcon::Normal : QIcon::Disabled);
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap(_style->_helper->coloredIcon(toolBoxOption->icon,
                                              toolBoxOption->palette,
                                              iconRect.size(), dpr, mode));
    _style->drawItemPixmap(painter, iconRect, textFlags, pixmap);
  }

  // render text
  if (!toolBoxOption->text.isEmpty()) {
    contentsRect = _style->visualRect(option, contentsRect);
    _style->drawItemText(painter, contentsRect, textFlags, palette, enabled,
                 toolBoxOption->text, QPalette::WindowText);
  }

  return true;
}

bool Render::TabsControl::drawToolBoxTabShapeControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto toolBoxOption(
      qstyleoption_cast<const QStyleOptionToolBox *>(option));
  if (!toolBoxOption)
    return true;

  // copy rect and palette
  const auto &rect(option->rect);
  const auto tabRect(_style->toolBoxTabContentsRect(option, widget));

  /*
   * important: option returns the wrong palette.
   * we use the widget palette instead, when set
   */
  const auto &palette(widget ? widget->palette() : option->palette);

  // store flags
  const QStyle::State &flags(option->state);
  const bool enabled(flags & QStyle::State_Enabled);
  const bool selected(flags & QStyle::State_Selected);
  const bool mouseOver(enabled && !selected && (flags & QStyle::State_MouseOver));

  // update animation state
  /*
   * the proper widget ( the toolbox tab ) is not passed as argument by Qt.
   * What is passed is the toolbox directly. To implement animations properly,
   *the painter->device() is used instead
   */
  bool isAnimated(false);
  qreal opacity(AnimationData::OpacityInvalid);
  QPaintDevice *device = painter->device();
  if (enabled && device) {
    _style->_animations->toolBoxEngine().updateState(device, mouseOver);
    isAnimated = _style->_animations->toolBoxEngine().isAnimated(device);
    opacity = _style->_animations->toolBoxEngine().opacity(device);
  }

  // color
  QColor outline;
  if (selected)
    outline = _style->_helper->focusColor(palette);
  else
    outline =
        _style->_helper->frameOutlineColor(palette, mouseOver, false, opacity,
                                   isAnimated ? AnimationHover : AnimationNone);

  // render
  _style->_helper->renderToolBoxFrame(painter, rect, tabRect.width(), outline);

  return true;
}

bool Render::TabsControl::drawDockWidgetTitleControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto dockWidgetOption =
      qstyleoption_cast<const QStyleOptionDockWidget *>(option);
  if (!dockWidgetOption)
    return true;

  const auto &palette(option->palette);
  const auto &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // cast to v2 to check vertical bar
  const bool verticalTitleBar(dockWidgetOption->verticalTitleBar);

  const auto buttonRect(_style->subElementRect(dockWidgetOption->floatable
                                           ? QStyle::SE_DockWidgetFloatButton
                                           : QStyle::SE_DockWidgetCloseButton,
                                       option, widget));

  // get rectangle and adjust to properly accounts for buttons
  auto rect(_style->insideMargin(dockWidgetOption->rect, Render::Frame_FrameWidth));
  if (verticalTitleBar) {
    if (buttonRect.isValid())
      rect.setTop(buttonRect.bottom() + 1);

  } else if (reverseLayout) {
    if (buttonRect.isValid())
      rect.setLeft(buttonRect.right() + 1);
    rect.adjust(0, 0, -4, 0);

  } else {
    if (buttonRect.isValid())
      rect.setRight(buttonRect.left() - 1);
    rect.adjust(4, 0, 0, 0);
  }

  QString title(dockWidgetOption->title);
  int titleWidth =
      dockWidgetOption->fontMetrics.size(_style->_mnemonics->textFlags(), title)
          .width();
  int width = verticalTitleBar ? rect.height() : rect.width();
  if (width < titleWidth)
    title = dockWidgetOption->fontMetrics.elidedText(
        title, Qt::ElideRight, width, Qt::TextShowMnemonic);

  if (verticalTitleBar) {
    QSize size = rect.size();
    size.transpose();
    rect.setSize(size);

    painter->save();
    painter->translate(rect.left(), rect.top() + rect.width());
    painter->rotate(-90);
    painter->translate(-rect.left(), -rect.top());
    _style->drawItemText(painter, rect,
                 Qt::AlignLeft | Qt::AlignVCenter | _style->_mnemonics->textFlags(),
                 palette, enabled, title, QPalette::WindowText);
    painter->restore();

  } else {
    _style->drawItemText(painter, rect,
                 Qt::AlignLeft | Qt::AlignVCenter | _style->_mnemonics->textFlags(),
                 palette, enabled, title, QPalette::WindowText);
  }

  return true;
}

void Helper::renderToolBoxFrame(QPainter *painter, const QRect &rect,
                                int tabWidth, const QColor &outline) const {
  if (!outline.isValid())
    return;

  // round radius
  const qreal radius(frameRadius(PenWidth::Frame));
  const QSizeF cornerSize(2 * radius, 2 * radius);

  // if rect - tabwidth is even, need to increase tabWidth by 1 unit
  // for anti aliasing
  if (!((rect.width() - tabWidth) % 2))
    ++tabWidth;

  // adjust rect for antialiasing
  QRectF baseRect(strokedRect(rect));

  // create path
  QPainterPath path;
  path.moveTo(0, baseRect.height() - 1);
  path.lineTo((baseRect.width() - tabWidth) / 2.0 - radius,
              baseRect.height() - 1);
  path.arcTo(QRectF(QPointF((baseRect.width() - tabWidth) / 2.0 - 2 * radius,
                            baseRect.height() - 1 - 2 * radius),
                    cornerSize),
             270, 90);
  path.lineTo((baseRect.width() - tabWidth) / 2.0, radius);
  path.arcTo(
      QRectF(QPointF((baseRect.width() - tabWidth) / 2.0, 0), cornerSize), 180,
      -90);
  path.lineTo((baseRect.width() + tabWidth) / 2.0 - 1 - radius, 0);
  path.arcTo(
      QRectF(QPointF((baseRect.width() + tabWidth) / 2.0 - 1 - 2 * radius, 0),
             cornerSize),
      90, -90);
  path.lineTo((baseRect.width() + tabWidth) / 2.0 - 1,
              baseRect.height() - 1 - radius);
  path.arcTo(QRectF(QPointF((baseRect.width() + tabWidth) / 2.0 - 1,
                            baseRect.height() - 1 - 2 * radius),
                    cornerSize),
             180, 90);
  path.lineTo(baseRect.width() - 1, baseRect.height() - 1);

  // render
  painter->setRenderHints(QPainter::Antialiasing);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(outline);
  painter->translate(baseRect.topLeft());
  painter->drawPath(path);
}

void Helper::renderTabWidgetFrame(QPainter *painter, const QRect &rect,
                                  const QColor &color, Corners corners,
                                  const bool windowActive) const {
  painter->setRenderHint(QPainter::Antialiasing);

  // QRectF frameRect( rect.adjusted( 1, 1, -1, -1 ) );
  QRectF frameRect(
      rect.adjusted(Render::Frame_FrameWidth, Render::Frame_FrameWidth,
                    -Render::Frame_FrameWidth, -Render::Frame_FrameWidth));
  qreal radius(frameRadius(PenWidth::NoPen, -1));

  // shadow
  renderBoxShadow(painter, frameRect, 0, 1, 5,
                  shadowColor(QApplication::palette(), 115, 40), radius,
                  windowActive);

  painter->setPen(Qt::NoPen);

  // set brush
  if (color.isValid())
    painter->setBrush(color);
  else
    painter->setBrush(Qt::NoBrush);

  // render
  QPainterPath path(roundedPath(frameRect, corners, radius));
  painter->drawPath(path);
}

void Helper::renderTabBarTab(QPainter *painter, const QRect &rect,
                             const QColor &color, Corners corners,
                             qreal radius) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  QRectF frameRect(rect);
  if (radius < 0)
    radius = frameRadius(PenWidth::NoPen, -1);
  else
    radius = qMin(radius, 0.5 * qMin(frameRect.width(), frameRect.height()));

  painter->setPen(Qt::NoPen);

  // brush
  if (color.isValid())
    painter->setBrush(color);
  else
    painter->setBrush(Qt::NoBrush);

  // render
  QPainterPath path(roundedPath(frameRect, corners, radius));
  painter->drawPath(path);
}

void Helper::renderTabBarTabOutline(QPainter *painter, const QRect &rect,
                                    const QColor &outlineColor, Corners corners,
                                    qreal radius) const {
  if (!outlineColor.isValid() || outlineColor.alphaF() <= 0)
    return;

  painter->setRenderHint(QPainter::Antialiasing, true);

  QRectF frameRect(rect);
  if (radius < 0)
    radius = frameRadius(PenWidth::NoPen, -1);
  else
    radius = qMin(radius, 0.5 * qMin(frameRect.width(), frameRect.height()));

  QPainterPath path(roundedPath(frameRect, corners, radius));

  painter->setBrush(Qt::NoBrush);
  painter->setPen(QPen(outlineColor, 1));
  painter->drawPath(path);
}

//* QAbstractButton child of a QToolBox: enable hover
bool Style::polishToolBoxButton(QWidget *widget) {
  if (!(qobject_cast<QAbstractButton *>(widget) &&
        qobject_cast<QToolBox *>(widget->parent())))
    return false;
  widget->setAttribute(Qt::WA_Hover);
  return true;
}

//* QToolBox
bool Style::polishToolBox(QWidget *widget) {
  if (!qobject_cast<QToolBox *>(widget))
    return false;
  widget->setBackgroundRole(QPalette::NoRole);
  widget->setAutoFillBackground(false);
  return true;
}

//* grandchild widget of a QToolBox
bool Style::polishToolBoxChild(QWidget *widget) {
  if (!(widget->parentWidget() && widget->parentWidget()->parentWidget() &&
        qobject_cast<QToolBox *>(
            widget->parentWidget()->parentWidget()->parentWidget())))
    return false;
  widget->setBackgroundRole(QPalette::NoRole);
  widget->setAutoFillBackground(false);
  widget->parentWidget()->setAutoFillBackground(false);
  return true;
}
bool Style::drawTabBarTabLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::TabsControl(this).drawTabBarTabLabelControl(option, painter, widget);
}

bool Style::drawTabBarTabShapeControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::TabsControl(this).drawTabBarTabShapeControl(option, painter, widget);
}

bool Style::drawToolBoxTabLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::TabsControl(this).drawToolBoxTabLabelControl(option, painter, widget);
}

bool Style::drawToolBoxTabShapeControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::TabsControl(this).drawToolBoxTabShapeControl(option, painter, widget);
}

bool Style::drawDockWidgetTitleControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::TabsControl(this).drawDockWidgetTitleControl(option, painter, widget);
}
} // namespace BlossomUI
