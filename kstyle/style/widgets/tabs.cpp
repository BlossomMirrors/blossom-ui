// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "private.h"

#include <KColorUtils>
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

bool Style::drawTabBarTabLabelControl(const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const {
  // code is copied from QCommonStyle
  // cast option and check
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));

  if (tabOption) {
    // call parent style method
    if (tabOption->documentMode)
      ParentStyleClass::drawControl(CE_TabBarTabLabel, option, painter, widget);

    // overwrite parent style behaviour and add horizontal padding
    else {
      QRect tr = tabOption->rect;
      bool verticalTabs = tabOption->shape == QTabBar::RoundedEast ||
                          tabOption->shape == QTabBar::RoundedWest ||
                          tabOption->shape == QTabBar::TriangularEast ||
                          tabOption->shape == QTabBar::TriangularWest;

      int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
      if (styleHint(SH_UnderlineShortcut, option, widget))
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
      tabLayout(tabOption, widget, &tr, &iconRect);
      tr = subElementRect(SE_TabBarTabText, option,
                          widget); // we compute tr twice because the style may
                                   // override subElementRect

      if (!tabOption->icon.isNull()) {
        QPixmap tabIcon = tabOption->icon.pixmap(
            tabOption->iconSize,
            (tabOption->state & State_Enabled) ? QIcon::Normal
                                               : QIcon::Disabled,
            (tabOption->state & State_Selected) ? QIcon::On : QIcon::Off);
        painter->drawPixmap(iconRect.x(), iconRect.y(), tabIcon);
      }

      QFont font = painter->font();
      if (!(tabOption->state & State_Enabled)) {
        if (tabOption->state & State_Selected) {
          // painter->setPen(Colors::mix(option->palette.brush(QPalette::Text).color(),
          // option->palette.brush(QPalette::Window).color(), 0.3));
          painter->setPen(option->palette.brush(QPalette::WindowText).color());
          // font.setBold(true);
        } else {
          painter->setPen(_helper->alphaColor(
              option->palette.brush(QPalette::WindowText).color(), 0.5));
        }
      } else {
        if (tabOption->state & State_Selected) {
          painter->setPen(option->palette.brush(QPalette::WindowText).color());
          // painter->setPen( KColorUtils::mix(
          // option->palette.brush(QPalette::WindowText).color(),       // test
          // more combinations later
          // option->palette.brush(QPalette::Highlight).color(), 0.5 ) );
          // font.setWeight( QFont::Medium );
        } else if (tabOption->state & State_Active &&
                   tabOption->state & State_MouseOver) {
          painter->setPen(option->palette.brush(QPalette::WindowText).color());
        } else {
          painter->setPen(_helper->alphaColor(
              option->palette.brush(QPalette::WindowText).color(), 0.75));
        }
      }
      painter->setFont(font);

      proxy()->drawItemText(painter, tr, alignment, tabOption->palette,
                            tabOption->state & State_Enabled, tabOption->text,
                            QPalette::NoRole);
      if (verticalTabs)
        painter->restore();
    }
  }

  // store rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // check focus
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool selected(state & State_Selected);
  const bool hasFocus(enabled && selected && (state & State_HasFocus));

  // update mouse over animation state
  _animations->tabBarEngine().updateState(widget, rect.topLeft(),
                                          AnimationFocus, hasFocus);
  const bool animated(enabled && selected &&
                      _animations->tabBarEngine().isAnimated(
                          widget, rect.topLeft(), AnimationFocus));
  const qreal opacity(_animations->tabBarEngine().opacity(
      widget, rect.topLeft(), AnimationFocus));

  if (!(hasFocus || animated))
    return true;
  if (!tabOption || tabOption->text.isEmpty())
    return true;

  // tab option rect
  const bool verticalTabs(isVerticalTab(tabOption));
  const int textFlags(Qt::AlignCenter | _mnemonics->textFlags());

  // text rect
  auto textRect(subElementRect(SE_TabBarTabText, option, widget));

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
    focusColor = _helper->alphaColor(_helper->focusColor(palette), opacity);
  else if (hasFocus)
    focusColor = _helper->focusColor(palette);

  // render focus line
  // _helper->renderFocusLine( painter, textRect, focusColor );

  if (verticalTabs)
    painter->restore();

  return true;
}

bool Style::drawTabBarTabShapeControl(const QStyleOption *option,
                                      QPainter *painter,
                                      const QWidget *widget) const {
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));
  if (!tabOption)
    return true;

  // palette and state
  const auto &palette(option->palette);
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool selected(state & State_Selected);
  const bool mouseOver(enabled && !selected && (state & State_MouseOver));

  const bool windowActive(widget && widget->isActiveWindow());

  // check if tab is being dragged
  const bool isDragged(widget && selected && painter->device() != widget);
  const bool isLocked(widget && _tabBarData->isLocked(widget));

  // store rect
  auto rect(option->rect);

  // update mouse over animation state
  _animations->tabBarEngine().updateState(widget, rect.topLeft(),
                                          AnimationHover, mouseOver);
  const bool animated(enabled && !selected &&
                      _animations->tabBarEngine().isAnimated(
                          widget, rect.topLeft(), AnimationHover));
  const qreal opacity(_animations->tabBarEngine().opacity(
      widget, rect.topLeft(), AnimationHover));

  // lock state
  if (selected && widget && isDragged)
    _tabBarData->lock(widget);
  else if (widget && selected && _tabBarData->isLocked(widget))
    _tabBarData->release();

  // tab position
  const QStyleOptionTab::TabPosition &position = tabOption->position;
  const bool isSingle(position == QStyleOptionTab::OnlyOneTab);
  const bool isQtQuickControl(this->isQtQuickControl(option, widget));
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
  const bool verticalTabs(isVerticalTab(tabOption));
  if ((reverseLayout && !verticalTabs) || _app.isLibreoffice) {
    qSwap(isFirst, isLast);
    qSwap(isLeftOfSelected, isRightOfSelected);
  }

  // overlap
  // for QtQuickControls, ovelap is already accounted of in the option. Unlike
  // in the qwidget case
  const int overlap(isQtQuickControl ? 0 : Metrics::TabBar_TabOverlap);

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
  if (StyleConfigData::documentModeTabs())
    backgroundColor = documentMode
                          ? _helper->isDarkTheme(palette)
                                ? _helper->alphaColor(configTabBgColor, 0.4)
                                : _helper->alphaColor(configTabBgColor, 0.2)
                          : _helper->alphaColor(configTabBgColor, 0.1);
  else
    backgroundColor = _helper->isDarkTheme(palette)
                          ? _helper->alphaColor(configTabBgColor, 0.4)
                          : _helper->alphaColor(configTabBgColor, 0.2);

  if ((_app.isDolphin || _app.isKonsole) &&
      (StyleConfigData::tabBarOpacity() < 100) && !_app.isOpaque) {
    // override the tab background color if adjustToDarkThemes is false
    if (StyleConfigData::adjustToDarkThemes()) {
      backgroundColor = _helper->transparentBarBgColor(backgroundColor, painter,
                                                       rect, BarType::TabBar);
    } else {
      backgroundColor = _helper->transparentBarBgColor(
          _toolsAreaManager->palette().color(QPalette::Window), painter, rect,
          BarType::TabBar);
    }
  }

  // shadow size
  constexpr int shadowSize = 4;
  QRect shadowRect;
  QRect backgroundRect;

  // tab color
  QColor color;
  if (selected)
    if (StyleConfigData::tabUseHighlightColor()) {
      color = palette.color(QPalette::Highlight);
    } else {
      color =
          (documentMode && !isQtQuickControl && !hasAlteredBackground(widget))
              ? palette.color(QPalette::Window)
              : _helper->frameBackgroundColor(palette);
    }
  else {
    const auto normal(
        _helper->alphaColor(palette.color(QPalette::Shadow), 0.1));
    const auto hover(_helper->alphaColor(_helper->hoverColor(palette), 0.2));
    if (animated)
      color = KColorUtils::mix(normal, hover, opacity);
    else if (mouseOver)
      color = hover;
    else
      color = normal;
  }

  // adjust rect and define corners based on tabbar orientation
  Corners corners;
  Corners backgroundCorners;

  // store tab shape
  Side side = SideTop;

  if (StyleConfigData::oldTabbar()) {
    switch (tabOption->shape) {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:

      side = SideTop;
      if (selected) {
        corners = CornerTopLeft | CornerTopRight;
      }

      shadowRect = QRect(rect.bottomLeft() + QPoint(0, 1),
                         QSize(rect.width(), shadowSize));
      if (isFirst)
        backgroundCorners |= CornerTopLeft;
      else
        shadowRect.adjust(-shadowSize, 0, 0, 0);
      if (isLast)
        backgroundCorners |= CornerTopRight;
      if (isLeftOfSelected)
        shadowRect.adjust(0, 0, shadowSize, 0);

      backgroundRect = rect;
      rect.adjust(3, 3, -3, 0);
      break;

    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:

      side = SideBottom;
      if (selected)
        corners = CornerBottomLeft | CornerBottomRight;

      shadowRect = QRect(rect.topLeft() - QPoint(0, shadowSize + 1),
                         QSize(rect.width(), shadowSize));
      if (isFirst)
        backgroundCorners |= CornerBottomLeft;
      else
        shadowRect.adjust(-shadowSize, 0, 0, 0);
      if (isLast)
        backgroundCorners |= CornerBottomRight;

      backgroundRect = rect;
      rect.adjust(3, 0, -3, -3);
      break;

    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:

      side = SideLeft;
      if (selected) {
        corners = CornerTopLeft | CornerBottomLeft;
        rect.adjust(0, 0, 1, 0);

      } else {
        rect.adjust(0, 0, -1, 0);
        if (isFirst)
          corners |= CornerTopLeft;
        if (isLast)
          corners |= CornerBottomLeft;
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
        corners = CornerTopRight | CornerBottomRight;
        rect.adjust(-1, 0, 0, 0);

      } else {
        rect.adjust(1, 0, 0, 0);
        if (isFirst)
          corners |= CornerTopRight;
        if (isLast)
          corners |= CornerBottomRight;
        if (isRightOfSelected)
          rect.adjust(0, -StyleConfigData::cornerRadius(), 0, 0);
        if (isLeftOfSelected)
          rect.adjust(0, 0, 0, StyleConfigData::cornerRadius());
        else if (!isLast)
          rect.adjust(0, 0, 0, overlap);
      }
      break;

    default:
      break;
    }
  } else {
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
  }

  const bool unifiedTabAndHeader =
      _app.isKonsole && StyleConfigData::unifiedTabBarKonsole();
  if (documentMode && unifiedTabAndHeader &&
      _helper->titleBarColor(true).alphaF() < 1.0) {
    _helper->renderTransparentArea(painter, backgroundRect);
    painter->fillRect(backgroundRect,
                      QBrush(_helper->titleBarColor(windowActive)));
  }

  // render
  if (StyleConfigData::tabBarAltStyle()) {
    const int hightlightHeight = selected ? 4 : 2;
    const QColor hightlightColor(selected ? _helper->focusColor(palette)
                                 : _helper->isDarkTheme(palette)
                                     ? QColor(255, 255, 255, 30)
                                     : QColor(0, 0, 0, 30));

    const int offset = selected ? 0 : 1;
    const int radius = isFirst || isLast || selected ? hightlightHeight / 2 : 0;

    painter->setPen(Qt::NoPen);
    painter->setBrush(hightlightColor);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawRoundedRect(
        backgroundRect.adjusted(
            side == SideLeft
                ? backgroundRect.width() - (hightlightHeight + offset)
            : (side == SideRight && !selected) ? 1
                                               : 0,
            side == SideTop
                ? backgroundRect.height() - (hightlightHeight + offset)
            : (side == SideBottom && !selected) ? 1
                                                : 0,
            side == SideRight
                ? -backgroundRect.width() + (hightlightHeight + offset)
            : (side == SideLeft && !selected) ? -1
                                              : 0,
            side == SideBottom
                ? -backgroundRect.height() + (hightlightHeight + offset)
            : (side == SideTop && !selected) ? -1
                                             : 0),
        radius, radius);

  } else if (selected) {
    QRegion oldRegion(painter->clipRegion());

    // thin light border on selected tab (mockup: distinct border, subtle
    // shadow)
    const QColor tabOutlineColor(_helper->isDarkTheme(palette)
                                     ? _helper->alphaColor(Qt::white, 0.15)
                                     : _helper->alphaColor(Qt::black, 0.12));
    const qreal tabRadius(_helper->tabFrameRadius());
    // sliding pill animation: use animated rect when selected tab is changing
    const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget);
    const bool pillAnimated =
        tabBar && documentMode &&
        _animations->tabBarEngine().isSelectedAnimated(tabBar);
    const QRect pillRect =
        pillAnimated
            ? _animations->tabBarEngine()
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
        _helper->renderTabBarTab(painter, pillRect, Qt::black, AllCorners,
                                 tabRadius);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        // no shadow for translucent tab
        if (!(color.alphaF() < 1))
          _helper->renderBoxShadow(painter, pillRect, 0, 1, 2,
                                   QColor(0, 0, 0, 80), qRound(tabRadius),
                                   true);
        _helper->renderTabBarTab(painter, pillRect, color, AllCorners,
                                 tabRadius);
        if (color.alphaF() >= 1)
          _helper->renderTabBarTabOutline(painter, pillRect, tabOutlineColor,
                                          AllCorners, tabRadius);
      } else {
        // render dark background and shadow
        _helper->renderTabBarTab(painter, backgroundRect, backgroundColor,
                                 backgroundCorners);
        _helper->renderBoxShadow(painter, shadowRect, 0, 1, shadowSize,
                                 QColor(0, 0, 0, 220),
                                 StyleConfigData::cornerRadius(), true);

        // subtle shadow on selected tab (less pronounced), thin light border
        _helper->renderBoxShadow(painter, pillRect, 0, 1, 2,
                                 QColor(0, 0, 0, 80), qRound(tabRadius), true);
        _helper->renderTabBarTab(
            painter, pillRect,
            _app.isLibreoffice ? palette.color(QPalette::Highlight) : color,
            pillAnimated ? AllCorners : corners, tabRadius);
        _helper->renderTabBarTabOutline(painter, pillRect, tabOutlineColor,
                                        pillAnimated ? AllCorners : corners,
                                        tabRadius);
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
        _helper->renderTabBarTab(
            painter,
            hlRect.adjusted(side == SideRight ? hlRect.width() - (hh + 1) : 0,
                            side == SideBottom ? hlRect.height() - (hh + 1) : 0,
                            side == SideLeft ? -hlRect.width() + (hh + 1) : 0,
                            side == SideTop ? -hlRect.height() + (hh + 1) : 0),
            _helper->focusColor(palette), pillAnimated ? AllCorners : corners,
            tabRadius);
      }

    }

    // non movable tabs
    else {
      _helper->renderTabBarTab(painter, backgroundRect, backgroundColor,
                               backgroundCorners);
      painter->setRenderHint(QPainter::Antialiasing, true);
      painter->setPen(Qt::NoPen);
      if (StyleConfigData::documentModeTabs())
        backgroundRect.adjust(5, 6, -5, -6);
      else
        backgroundRect.adjust(4, 4, -4, -4);
      _helper->renderBoxShadow(painter, backgroundRect, 0, 1, 2,
                               QColor(0, 0, 0, 60), qRound(tabRadius), true);
      painter->setBrush(color);
      painter->drawRoundedRect(backgroundRect, tabRadius, tabRadius);

      // Don't lighten the highlight color
      if (!StyleConfigData::tabUseHighlightColor()) {
        if (StyleConfigData::documentModeTabs())
          painter->setBrush(QColor(255, 255, 255, 20));
        painter->drawRoundedRect(backgroundRect, tabRadius, tabRadius);
      }
      _helper->renderTabBarTabOutline(painter, backgroundRect, tabOutlineColor,
                                      AllCorners, tabRadius);
    }

    painter->setClipRegion(oldRegion);

  } else {
    _helper->renderTabBarTab(painter, backgroundRect, backgroundColor,
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
      painter->setBrush(_helper->alphaColor(_helper->hoverColor(palette), 0.2));
      painter->setRenderHint(QPainter::Antialiasing, true);
      painter->setPen(Qt::NoPen);
      const qreal hoverRadius(_helper->tabFrameRadius());
      painter->drawRoundedRect(backgroundRect, hoverRadius, hoverRadius);
    }
  }

  return true;
}

bool Style::drawToolBoxTabLabelControl(const QStyleOption *option,
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

  const State &state(option->state);
  const bool enabled(state & State_Enabled);

  // text alignment
  const int textFlags(_mnemonics->textFlags() | Qt::AlignCenter);

  // contents rect
  const auto rect(subElementRect(SE_ToolBoxTabContents, option, widget));

  // store icon size
  const int iconSize(pixelMetric(QStyle::PM_SmallIconSize, option, widget));

  // find contents size and rect
  auto contentsRect(rect);
  QSize contentsSize;
  if (!toolBoxOption->text.isEmpty()) {
    contentsSize =
        option->fontMetrics.size(_mnemonics->textFlags(), toolBoxOption->text);
    if (!toolBoxOption->icon.isNull())
      contentsSize.rwidth() += Metrics::ToolBox_TabItemSpacing;
  }

  // icon size
  if (!toolBoxOption->icon.isNull()) {
    contentsSize.setHeight(qMax(contentsSize.height(), iconSize));
    contentsSize.rwidth() += iconSize;
  }

  // adjust contents rect
  contentsRect = centerRect(contentsRect, contentsSize);

  // render icon
  if (!toolBoxOption->icon.isNull()) {
    // icon rect
    QRect iconRect;
    if (toolBoxOption->text.isEmpty())
      iconRect = centerRect(contentsRect, iconSize, iconSize);
    else {
      iconRect = contentsRect;
      iconRect.setWidth(iconSize);
      iconRect = centerRect(iconRect, iconSize, iconSize);
      contentsRect.setLeft(iconRect.right() + Metrics::ToolBox_TabItemSpacing +
                           1);
    }

    iconRect = visualRect(option, iconRect);
    const QIcon::Mode mode(enabled ? QIcon::Normal : QIcon::Disabled);
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap(_helper->coloredIcon(toolBoxOption->icon,
                                              toolBoxOption->palette,
                                              iconRect.size(), dpr, mode));
    drawItemPixmap(painter, iconRect, textFlags, pixmap);
  }

  // render text
  if (!toolBoxOption->text.isEmpty()) {
    contentsRect = visualRect(option, contentsRect);
    drawItemText(painter, contentsRect, textFlags, palette, enabled,
                 toolBoxOption->text, QPalette::WindowText);
  }

  return true;
}

bool Style::drawToolBoxTabShapeControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto toolBoxOption(
      qstyleoption_cast<const QStyleOptionToolBox *>(option));
  if (!toolBoxOption)
    return true;

  // copy rect and palette
  const auto &rect(option->rect);
  const auto tabRect(toolBoxTabContentsRect(option, widget));

  /*
   * important: option returns the wrong palette.
   * we use the widget palette instead, when set
   */
  const auto &palette(widget ? widget->palette() : option->palette);

  // store flags
  const State &flags(option->state);
  const bool enabled(flags & State_Enabled);
  const bool selected(flags & State_Selected);
  const bool mouseOver(enabled && !selected && (flags & State_MouseOver));

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
    _animations->toolBoxEngine().updateState(device, mouseOver);
    isAnimated = _animations->toolBoxEngine().isAnimated(device);
    opacity = _animations->toolBoxEngine().opacity(device);
  }

  // color
  QColor outline;
  if (selected)
    outline = _helper->focusColor(palette);
  else
    outline =
        _helper->frameOutlineColor(palette, mouseOver, false, opacity,
                                   isAnimated ? AnimationHover : AnimationNone);

  // render
  _helper->renderToolBoxFrame(painter, rect, tabRect.width(), outline);

  return true;
}

bool Style::drawDockWidgetTitleControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto dockWidgetOption =
      qstyleoption_cast<const QStyleOptionDockWidget *>(option);
  if (!dockWidgetOption)
    return true;

  const auto &palette(option->palette);
  const auto &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // cast to v2 to check vertical bar
  const bool verticalTitleBar(dockWidgetOption->verticalTitleBar);

  const auto buttonRect(subElementRect(dockWidgetOption->floatable
                                           ? SE_DockWidgetFloatButton
                                           : SE_DockWidgetCloseButton,
                                       option, widget));

  // get rectangle and adjust to properly accounts for buttons
  auto rect(insideMargin(dockWidgetOption->rect, Metrics::Frame_FrameWidth));
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
      dockWidgetOption->fontMetrics.size(_mnemonics->textFlags(), title)
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
    drawItemText(painter, rect,
                 Qt::AlignLeft | Qt::AlignVCenter | _mnemonics->textFlags(),
                 palette, enabled, title, QPalette::WindowText);
    painter->restore();

  } else {
    drawItemText(painter, rect,
                 Qt::AlignLeft | Qt::AlignVCenter | _mnemonics->textFlags(),
                 palette, enabled, title, QPalette::WindowText);
  }

  return true;
}

} // namespace BlossomUI
