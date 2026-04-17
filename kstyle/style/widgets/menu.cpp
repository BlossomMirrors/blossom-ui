// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "private.h"

#include <KColorUtils>
#include <QApplication>
#include <QMenuBar>
#include <QPainter>
#include <QStyleOptionMenuItem>
#include <QToolBar>

namespace BlossomUI {

bool Style::drawMenuBarEmptyAreaControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  if (!widget)
    return true;

  const bool windowActive(widget && widget->isActiveWindow());

  const auto &rect(option->rect);

  // draw background

  // menubar and toolbar background should match for consistency
  QColor opacityBackground(
      _toolsAreaManager->palette().color(QPalette::Window));

  // changes menubar background opacity
  if (StyleConfigData::menuBarOpacity() < 100 && !_app.isOpaque) {
    opacityBackground = _helper->transparentBarBgColor(
        opacityBackground, painter, rect, BarType::MenuBar);
    painter->fillRect(rect, opacityBackground);
  }

  if (widget && _helper->titleBarColor(windowActive).alphaF() * 100.0 < 100 &&
      _translucentWidgets.contains(widget->window())) {
    bool shouldDrawShadow = false;
    if (BlossomUIPrivate::possibleTranslucentToolBars.isEmpty())
      shouldDrawShadow = true;

    if (BlossomUIPrivate::possibleTranslucentToolBars.size() == 1) {
      QSet<const QWidget *>::const_iterator i =
          BlossomUIPrivate::possibleTranslucentToolBars.constBegin();
      const QToolBar *tb = qobject_cast<const QToolBar *>(*i);

      if (tb) {
        if (tb->orientation() == Qt::Vertical)
          shouldDrawShadow = true;
        else if (tb->y() > widget->y() + rect.height())
          shouldDrawShadow = true; // bottom toolbar
      }
    } else if (_helper->titleBarColor(windowActive).alphaF() * 100.0 < 100) {
      shouldDrawShadow = false;
    }

    if (_app.isKonsole && StyleConfigData::unifiedTabBarKonsole())
      shouldDrawShadow = false;

    if (shouldDrawShadow) {
      painter->setBrush(Qt::NoBrush);
      QLinearGradient gradient(rect.bottomLeft(), rect.bottomRight());
      gradient.setColorAt(0, QColor(0, 0, 0, 40));
      gradient.setColorAt(0.95, QColor(0, 0, 0, 40));
      gradient.setColorAt(1, QColor(0, 0, 0, 40 / 2));
      painter->setPen(QPen(gradient, 1));
      painter->drawLine(rect.bottomLeft(), rect.bottomRight());

      gradient.setColorAt(0, QColor(0, 0, 0, 12));
      gradient.setColorAt(0.95, QColor(0, 0, 0, 12));
      gradient.setColorAt(1, QColor(0, 0, 0, 12 / 2));
      painter->setPen(QPen(gradient, 1));
      painter->drawLine(rect.bottomLeft() - QPoint(0, 1),
                        rect.bottomRight() - QPoint(0, 1));

      gradient.setColorAt(0, QColor(0, 0, 0, 3));
      gradient.setColorAt(0.95, QColor(0, 0, 0, 3));
      gradient.setColorAt(1, QColor(0, 0, 0, 3 / 2));
      painter->setPen(QPen(gradient, 1));
      painter->drawLine(rect.bottomLeft() - QPoint(0, 2),
                        rect.bottomRight() - QPoint(0, 2));
    }
  }

  return true;
}

bool Style::drawMenuBarItemControl(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  // cast option and check
  const auto menuItemOption =
      qstyleoption_cast<const QStyleOptionMenuItem *>(option);
  if (!menuItemOption)
    return true;

  const bool windowActive(widget && widget->isActiveWindow());

  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // menubar and toolbar background should match for consistency
  QColor opacityBackground(
      _toolsAreaManager->palette().color(QPalette::Window));

  // changes menubar background opacity
  if (StyleConfigData::menuBarOpacity() < 100 && !_app.isOpaque) {
    opacityBackground = _helper->transparentBarBgColor(
        opacityBackground, painter, rect, BarType::MenuBar);
    painter->fillRect(rect, opacityBackground);
  }

  if (widget && _helper->titleBarColor(windowActive).alphaF() * 100.0 < 100 &&
      _translucentWidgets.contains(widget->window())) {
    bool shouldDrawShadow = false;
    int shadow_xoffset = 0;
    if (BlossomUIPrivate::possibleTranslucentToolBars.isEmpty())
      shouldDrawShadow = true;

    if (BlossomUIPrivate::possibleTranslucentToolBars.size() == 1) {
      QSet<const QWidget *>::const_iterator i =
          BlossomUIPrivate::possibleTranslucentToolBars.constBegin();
      const QToolBar *tb = qobject_cast<const QToolBar *>(*i);

      if (tb) {
        if (tb->orientation() == Qt::Vertical) {
          shouldDrawShadow = true;
          shadow_xoffset =
              tb->rect().width() - 1; // assumes the toolbar is at the left side
        } else if (tb->y() > widget->y() + rect.height())
          shouldDrawShadow = true; // bottom toolbar
      }
    } else if (_helper->titleBarColor(windowActive).alphaF() * 100.0 < 100) {
      shouldDrawShadow =
          false; // don't draw shadow if using transparent color schemes
    }

    if (_app.isKonsole && StyleConfigData::unifiedTabBarKonsole())
      shouldDrawShadow = false;

    if (shouldDrawShadow) {
      // QRect shadowRect = rect.adjusted(shadow_xoffset, 0, 0, 0);
      const auto &widgetRect(widget->rect().adjusted(shadow_xoffset, 0, 0, 0));

      painter->setBrush(Qt::NoBrush);
      QLinearGradient gradient(widgetRect.bottomLeft(),
                               widgetRect.bottomRight());
      gradient.setColorAt(0, QColor(0, 0, 0, shadow_xoffset > 0 ? 0 : 40 / 2));
      gradient.setColorAt(0.05, QColor(0, 0, 0, 40));
      gradient.setColorAt(1, QColor(0, 0, 0, 40));
      painter->setPen(QPen(gradient, 1));
      // painter->setPen( QColor(0,0,0,40) );
      painter->drawLine(widgetRect.bottomLeft(), widgetRect.bottomRight());

      gradient.setColorAt(0, QColor(0, 0, 0, shadow_xoffset > 0 ? 0 : 12 / 2));
      gradient.setColorAt(0.05, QColor(0, 0, 0, 12));
      gradient.setColorAt(1, QColor(0, 0, 0, 12));
      painter->setPen(QPen(gradient, 1));
      // painter->setPen( QColor(0,0,0,12) );
      painter->drawLine(widgetRect.bottomLeft() - QPoint(0, 1),
                        widgetRect.bottomRight() - QPoint(0, 1));

      gradient.setColorAt(0, QColor(0, 0, 0, shadow_xoffset > 0 ? 0 : 3 / 2));
      gradient.setColorAt(0.05, QColor(0, 0, 0, 3));
      gradient.setColorAt(1, QColor(0, 0, 0, 3));
      painter->setPen(QPen(gradient, 1));
      // painter->setPen(QColor(0,0,0,3) );
      painter->drawLine(widgetRect.bottomLeft() - QPoint(0, 2),
                        widgetRect.bottomRight() - QPoint(0, 2));

      /*painter->setBrush( Qt::NoBrush );
      QLinearGradient gradient( shadowRect.bottomLeft(),
      shadowRect.bottomRight() ); gradient.setColorAt( 0, QColor(0,0,0,
      shadow_xoffset > 0 ? 0 : 40/2) ); gradient.setColorAt( 0.05,
      QColor(0,0,0,40) ); gradient.setColorAt( 1, QColor(0,0,0,40) );
      painter->setPen( QPen(gradient, 1) );
      painter->drawLine( shadowRect.bottomLeft(), shadowRect.bottomRight() );

      gradient.setColorAt( 0, QColor(0,0,0,shadow_xoffset > 0 ? 0 : 12/2) );
      gradient.setColorAt( 0.05, QColor(0,0,0,12) );
      gradient.setColorAt( 1, QColor(0,0,0,12) );
      painter->setPen( QPen(gradient, 1) );
      painter->drawLine( shadowRect.bottomLeft() - QPoint(0, 1),
      shadowRect.bottomRight() - QPoint(0, 1) );

      gradient.setColorAt( 0, QColor(0,0,0,shadow_xoffset > 0 ? 0 : 3/2) );
      gradient.setColorAt( 0.05, QColor(0,0,0,3) );
      gradient.setColorAt( 1, QColor(0,0,0,3/2) );
      painter->setPen( QPen(gradient, 1) );
      painter->drawLine( shadowRect.bottomLeft() - QPoint(0, 2),
      shadowRect.bottomRight() - QPoint(0, 2) );*/
    }
  }

  // store state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool selected(enabled && (state & State_Selected));
  const bool sunken(enabled && (state & State_Sunken));
  const bool useStrongFocus(StyleConfigData::menuItemDrawStrongFocus());

  painter->save();

  // render hover and focus
  if (useStrongFocus && (selected || sunken)) {
    QColor backgroundColor;
    if (sunken)
      backgroundColor = _helper->focusColor(palette);
    else if (selected)
      backgroundColor = _helper->hoverColor(palette);

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setBrush(backgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1),
                             StyleConfigData::cornerRadius(),
                             StyleConfigData::cornerRadius());
  }

  /*
  check if item as an icon, in which case only the icon should be rendered
  consistently with comment in QMenuBarPrivate::calcActionRects
  */
  if (!menuItemOption->icon.isNull()) {
    // icon size is forced to SmallIconSize
    const auto iconSize =
        pixelMetric(QStyle::PM_SmallIconSize, nullptr, widget);
    const auto iconRect = centerRect(rect, iconSize, iconSize);

    // decide icon mode and state
    QIcon::Mode iconMode;
    QIcon::State iconState;
    if (!enabled) {
      iconMode = QIcon::Disabled;
      iconState = QIcon::Off;

    } else {
      if (useStrongFocus && sunken)
        iconMode = QIcon::Selected;
      else if (useStrongFocus && selected)
        iconMode = QIcon::Active;
      else
        iconMode = QIcon::Normal;

      iconState = sunken ? QIcon::On : QIcon::Off;
    }

    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const auto pixmap =
        _helper->coloredIcon(menuItemOption->icon, menuItemOption->palette,
                             iconRect.size(), dpr, iconMode, iconState);
    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

    // render outline
    if (!useStrongFocus && (selected || sunken)) {
      QColor outlineColor;
      if (sunken)
        outlineColor = _helper->focusColor(palette);
      else if (selected)
        outlineColor = _helper->hoverColor(palette);

      _helper->renderFocusLine(painter, iconRect, outlineColor);
    }

  } else {
    // get text rect
    const int textFlags(Qt::AlignCenter | _mnemonics->textFlags());
    const auto textRect =
        option->fontMetrics.boundingRect(rect, textFlags, menuItemOption->text);

    // render text
    const QPalette::ColorRole role = (useStrongFocus && sunken)
                                         ? QPalette::HighlightedText
                                         : QPalette::WindowText;
    drawItemText(painter, textRect, textFlags, palette, enabled,
                 menuItemOption->text, role);

    // render outline
    if (!useStrongFocus && (selected || sunken)) {
      QColor outlineColor;
      if (sunken)
        outlineColor = _helper->focusColor(palette);
      else if (selected)
        outlineColor = _helper->hoverColor(palette);

      _helper->renderFocusLine(painter, textRect, outlineColor);
    }
  }

  painter->restore();
  return true;
}

bool Style::drawMenuItemControl(const QStyleOption *option, QPainter *painter,
                                const QWidget *widget) const {
  // cast option and check
  const auto menuItemOption =
      qstyleoption_cast<const QStyleOptionMenuItem *>(option);
  if (!menuItemOption)
    return true;
  if (menuItemOption->menuItemType == QStyleOptionMenuItem::EmptyArea)
    return true;

  // store window state
  const bool windowActive(widget && widget->isActiveWindow());

  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // deal with separators
  if (menuItemOption->menuItemType == QStyleOptionMenuItem::Separator) {
    // normal separator
    if (menuItemOption->text.isEmpty() && menuItemOption->icon.isNull()) {
      const auto color(_helper->separatorColor(palette));
      _helper->renderSeparator(
          painter,
          rect.adjusted(Metrics::MenuItem_MarginWidth * 2, 0,
                        -Metrics::MenuItem_MarginWidth * 2, 0),
          color);
      return true;

    } else {
      /*
       * separator can have a title and an icon
       * in that case they are rendered as menu title buttons
       */
      QStyleOptionToolButton copy(
          separatorMenuItemOption(menuItemOption, widget));
      renderMenuTitle(&copy, painter, widget);

      return true;
    }
  }

  // store state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool selected(enabled && (state & State_Selected));
  const bool sunken(enabled && (state & (State_On | State_Sunken)));
  const bool reverseLayout(option->direction == Qt::RightToLeft);
  const bool useStrongFocus(StyleConfigData::menuItemDrawStrongFocus());

  _animations->inputWidgetEngine().updateState(widget, AnimationHover,
                                               selected);
  _animations->inputWidgetEngine().buttonAnimationMode(widget);
  _animations->inputWidgetEngine().buttonOpacity(widget);

  // render hover and focus
  if (selected || sunken) {
    const auto color =
        useStrongFocus ? _helper->focusColor(palette).darker(sunken ? 120 : 0)
                       : _helper->separatorColor(palette);
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    const qreal radius = StyleConfigData::menuItemRadius();
    if (StyleConfigData::menuItemRadius() > 1)
      painter->drawRoundedRect(sunken ? rect.adjusted(2, 2, -2, -2)
                                      : rect.adjusted(1, 1, -1, -1),
                               radius, radius);
    else
      painter->drawRoundedRect(sunken ? rect.adjusted(1, 1, -1, -1) : rect,
                               radius, radius);
  }

  // get rect available for contents
  auto contentsRect(insideMargin(
      rect, Metrics::MenuItem_MarginWidth,
      (Metrics::MenuItem_MarginHeight + StyleConfigData::menuItemHeight())));

  // define relevant rectangles
  // checkbox
  QRect checkBoxRect;
  if (menuItemOption->menuHasCheckableItems) {
    checkBoxRect =
        QRect(contentsRect.left(),
              contentsRect.top() +
                  (contentsRect.height() - Metrics::CheckBox_Size) / 2,
              Metrics::CheckBox_Size, Metrics::CheckBox_Size);
    contentsRect.setLeft(checkBoxRect.right() + Metrics::MenuItem_ItemSpacing +
                         1);
  } else {
    // always have some left margin
    contentsRect.setLeft(rect.left() + 3 * Metrics::MenuItem_ItemSpacing);
  }

  // render checkbox indicator
  if (menuItemOption->checkType == QStyleOptionMenuItem::NonExclusive) {
    checkBoxRect = visualRect(option, checkBoxRect);

    // checkbox state

    CheckBoxState state(menuItemOption->checked ? CheckOn : CheckOff);
    // const auto color( _helper->checkBoxIndicatorColor( palette, false,
    // enabled && active ) ); const auto background(state == CheckOn ?
    // palette.color(QPalette::Highlight) : palette.color(QPalette::Button));
    //_helper->renderCheckBoxBackground( painter, checkBoxRect, palette.color(
    //QPalette::Window ), sunken );    //not needed
    _helper->renderCheckBox(painter, checkBoxRect, palette, true, sunken,
                            (selected || sunken), state, windowActive);

  } else if (menuItemOption->checkType == QStyleOptionMenuItem::Exclusive) {
    checkBoxRect = visualRect(option, checkBoxRect);

    const bool active(menuItemOption->checked);
    // const auto shadow( _helper->shadowColor( palette ) );
    // const auto color( _helper->checkBoxIndicatorColor( palette, false,
    // enabled && active ) );
    //_helper->renderRadioButtonBackground( painter, checkBoxRect,
    //palette.color( QPalette::Window ), sunken ); //not needed
    _helper->renderRadioButton(painter, checkBoxRect, palette,
                               (selected || sunken), sunken,
                               active ? RadioOn : RadioOff, true);
  }

  // icon
  int iconWidth = 0;
  const bool showIcon(showIconsInMenuItems());
  if (showIcon)
    iconWidth = isQtQuickControl(option, widget)
                    ? qMax(pixelMetric(PM_SmallIconSize, option, widget),
                           menuItemOption->maxIconWidth)
                    : menuItemOption->maxIconWidth;

  QRect iconRect;
  if (showIcon && iconWidth > 0) {
    iconRect =
        QRect(contentsRect.left(),
              contentsRect.top() + (contentsRect.height() - iconWidth) / 2,
              iconWidth, iconWidth);
    contentsRect.setLeft(iconRect.right() + Metrics::MenuItem_ItemSpacing * 2);
    const QSize iconSize(pixelMetric(PM_SmallIconSize, option, widget),
                         pixelMetric(PM_SmallIconSize, option, widget));
    iconRect = centerRect(iconRect, iconSize);
  }

  if (showIcon && !menuItemOption->icon.isNull()) {
    iconRect = visualRect(option, iconRect);

    // icon mode
    QIcon::Mode mode;
    if (selected && !useStrongFocus)
      mode = QIcon::Active;
    else if (selected)
      mode = QIcon::Selected;
    else if (enabled)
      mode = QIcon::Normal;
    else
      mode = QIcon::Disabled;

    // icon state
    const QIcon::State iconState(sunken ? QIcon::On : QIcon::Off);
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap =
        _helper->coloredIcon(menuItemOption->icon, menuItemOption->palette,
                             iconRect.size(), dpr, mode, iconState);
    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
  }

  // arrow
  QRect arrowRect(
      contentsRect.right() - Metrics::MenuButton_IndicatorWidth + 1,
      contentsRect.top() +
          (contentsRect.height() - Metrics::MenuButton_IndicatorWidth) / 2,
      Metrics::MenuButton_IndicatorWidth, Metrics::MenuButton_IndicatorWidth);
  contentsRect.setRight(arrowRect.left() - Metrics::MenuItem_ItemSpacing - 1);

  if (menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu) {
    // apply right-to-left layout
    arrowRect = visualRect(option, arrowRect);

    // arrow orientation
    const ArrowOrientation orientation(reverseLayout ? ArrowLeft : ArrowRight);

    // color
    QColor arrowColor;
    if (useStrongFocus && (selected || sunken))
      arrowColor = palette.color(QPalette::HighlightedText);
    else if (sunken)
      arrowColor = _helper->focusColor(palette);
    else if (selected)
      arrowColor = _helper->hoverColor(palette);
    else
      arrowColor = _helper->arrowColor(palette, QPalette::Text);

    // render
    _helper->renderArrow(painter, arrowRect, arrowColor, orientation);
  }

  // text
  auto textRect = contentsRect;
  if (!menuItemOption->text.isEmpty()) {
    // adjust textRect
    QString text = menuItemOption->text;
    textRect = centerRect(
        textRect, textRect.width(),
        option->fontMetrics.size(_mnemonics->textFlags(), text).height());
    textRect = visualRect(option, textRect);

    // set font
    painter->setFont(menuItemOption->font);

    // color role
    const QPalette::ColorRole role = (useStrongFocus && (selected || sunken))
                                         ? QPalette::HighlightedText
                                         : QPalette::Text;

    // locate accelerator and render
    const int tabPosition(text.indexOf(QLatin1Char('\t')));
    if (tabPosition >= 0) {
      // add a bit of transparency
      QColor c(palette.color(role));
      c.setAlpha(100);
      QPalette p;
      p.setColor(role, c);

      const int textFlags(Qt::AlignVCenter | Qt::AlignRight);
      QString accelerator(text.mid(tabPosition + 1));
      text = text.left(tabPosition);
      drawItemText(painter, textRect, textFlags, p, enabled, accelerator, role);
    }

    // render text
    const int textFlags(Qt::AlignVCenter |
                        (reverseLayout ? Qt::AlignRight : Qt::AlignLeft) |
                        _mnemonics->textFlags());
    textRect = option->fontMetrics.boundingRect(textRect, textFlags, text);
    drawItemText(painter, textRect, textFlags, palette, enabled, text, role);

    // render hover and focus
    if (!useStrongFocus && (selected || sunken)) {
      QColor outlineColor;
      if (sunken)
        outlineColor = _helper->focusColor(palette);
      else if (selected)
        outlineColor = _helper->hoverColor(palette);

      _helper->renderFocusLine(painter, textRect, outlineColor);
    }
  }

  return true;
}
} // namespace BlossomUI
