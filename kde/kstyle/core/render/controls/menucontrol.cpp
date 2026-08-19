// SPDX-License-Identifier: GPL-2.0-or-later
#include "menucontrol.h"
#include "blossomuianimations.h"
#include "blossomuiblurhelper.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "button.h"
#include "checkbox.h"
#include "frame.h"
#include "private.h"
#include "widgetrenderer.h"

#include <KColorUtils>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QSet>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QStyleOptionMenuItem>
#include <QToolBar>

namespace BlossomUI {

bool Render::MenuControl::drawPanelMenuPrimitive(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  /*
   * Skip drawing only for embedded QMenu (transparent background).
   * Draw for toplevel menus and for non-menu widgets (e.g. Slint Qt backend
   * NativeComboBoxPopup uses QStyle::PE_PanelMenu with a plain QWidget, not a window).
   */
  if (widget && qobject_cast<const QMenu *>(widget) && !widget->isWindow())
    return true;

  const bool isSlintPopup(widget && !widget->isWindow() &&
                          !qobject_cast<const QMenu *>(widget));
  if (isSlintPopup && widget->window()) {
    const QRect rectInWindow(
        widget->mapTo(widget->window(), option->rect.topLeft()),
        widget->mapTo(widget->window(), option->rect.bottomRight()));
    _style->_blurHelper->setSlintPopupRegion(widget->window(),
                                     rectInWindow.normalized());
  }

  const auto &palette(option->palette);
  const auto outline(_style->_helper->isDarkTheme(palette) ? QColor(255, 255, 255, 20)
                                                   : QColor(0, 0, 0, 40));
  bool hasAlpha(_style->_helper->hasAlphaChannel(widget));
  if (isSlintPopup)
    hasAlpha = true;
  auto background(_style->_helper->frameBackgroundColor(palette));

  painter->save();

  if (isSlintPopup && widget->window()) {
    const QRect rectInWindow(
        widget->mapTo(widget->window(), option->rect.topLeft()),
        widget->mapTo(widget->window(), option->rect.bottomRight()));
    if (_style->_helper->renderBlurredBackground(painter, widget->window(),
                                         rectInWindow.normalized(),
                                         option->rect, 12)) {
      background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
      painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter->setBrush(background);
      painter->setPen(Qt::NoPen);
      painter->drawRoundedRect(option->rect, StyleConfigData::menuItemRadius(),
                               StyleConfigData::menuItemRadius());
      if (outline.isValid()) {
        painter->setPen(outline);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(
            QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5),
            StyleConfigData::menuItemRadius() + 0.5,
            StyleConfigData::menuItemRadius() + 0.5);
      }
    } else {
      if (hasAlpha) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
      }
      _style->_helper->renderMenuFrame(painter, option->rect, background, outline,
                               hasAlpha);
    }
  } else {
    if (hasAlpha) {
      painter->setCompositionMode(QPainter::CompositionMode_Source);
      background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
    }
    _style->_helper->renderMenuFrame(painter, option->rect, background, outline,
                             hasAlpha);
  }

  painter->restore();

  return true;
}

bool Render::MenuControl::drawMenuBarEmptyAreaControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  if (!widget)
    return true;

  const bool windowActive(widget && widget->isActiveWindow());

  const auto &rect(option->rect);

  // draw background

  // menubar and toolbar background should match for consistency
  QColor opacityBackground(
      _style->_toolsAreaManager->palette().color(QPalette::Window));

  // changes menubar background opacity
  if (StyleConfigData::menuBarOpacity() < 100 && !_style->_app.isOpaque) {
    opacityBackground = _style->_helper->transparentBarBgColor(
        opacityBackground, painter, rect, BarType::MenuBar);
    painter->fillRect(rect, opacityBackground);
  }

  if (widget && _style->_helper->titleBarColor(windowActive).alphaF() * 100.0 < 100 &&
      _style->_translucentWidgets.contains(widget->window())) {
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
    } else if (_style->_helper->titleBarColor(windowActive).alphaF() * 100.0 < 100) {
      shouldDrawShadow = false;
    }

    if (_style->_app.isKonsole && StyleConfigData::unifiedTabBarKonsole())
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

bool Render::MenuControl::drawMenuBarItemControl(const QStyleOption *option,
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
      _style->_toolsAreaManager->palette().color(QPalette::Window));

  // changes menubar background opacity
  if (StyleConfigData::menuBarOpacity() < 100 && !_style->_app.isOpaque) {
    opacityBackground = _style->_helper->transparentBarBgColor(
        opacityBackground, painter, rect, BarType::MenuBar);
    painter->fillRect(rect, opacityBackground);
  }

  if (widget && _style->_helper->titleBarColor(windowActive).alphaF() * 100.0 < 100 &&
      _style->_translucentWidgets.contains(widget->window())) {
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
    } else if (_style->_helper->titleBarColor(windowActive).alphaF() * 100.0 < 100) {
      shouldDrawShadow =
          false; // don't draw shadow if using transparent color schemes
    }

    if (_style->_app.isKonsole && StyleConfigData::unifiedTabBarKonsole())
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
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool selected(enabled && (state & QStyle::State_Selected));
  const bool sunken(enabled && (state & QStyle::State_Sunken));
  const bool useStrongFocus(StyleConfigData::menuItemDrawStrongFocus());

  painter->save();

  // render hover and focus
  if (useStrongFocus && (selected || sunken)) {
    QColor backgroundColor;
    if (sunken)
      backgroundColor = _style->_helper->focusColor(palette);
    else if (selected)
      backgroundColor = _style->_helper->hoverColor(palette);

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
        _style->pixelMetric(QStyle::PM_SmallIconSize, nullptr, widget);
    const auto iconRect = _style->centerRect(rect, iconSize, iconSize);

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
        _style->_helper->coloredIcon(menuItemOption->icon, menuItemOption->palette,
                             iconRect.size(), dpr, iconMode, iconState);
    _style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

    // render outline
    if (!useStrongFocus && (selected || sunken)) {
      QColor outlineColor;
      if (sunken)
        outlineColor = _style->_helper->focusColor(palette);
      else if (selected)
        outlineColor = _style->_helper->hoverColor(palette);

      _style->_helper->renderFocusLine(painter, iconRect, outlineColor);
    }

  } else {
    // get text rect
    const int textFlags(Qt::AlignCenter | _style->_mnemonics->textFlags());
    const auto textRect =
        option->fontMetrics.boundingRect(rect, textFlags, menuItemOption->text);

    // render text
    const QPalette::ColorRole role = (useStrongFocus && sunken)
                                         ? QPalette::HighlightedText
                                         : QPalette::WindowText;
    _style->drawItemText(painter, textRect, textFlags, palette, enabled,
                 menuItemOption->text, role);

    // render outline
    if (!useStrongFocus && (selected || sunken)) {
      QColor outlineColor;
      if (sunken)
        outlineColor = _style->_helper->focusColor(palette);
      else if (selected)
        outlineColor = _style->_helper->hoverColor(palette);

      _style->_helper->renderFocusLine(painter, textRect, outlineColor);
    }
  }

  painter->restore();
  return true;
}

bool Render::MenuControl::drawMenuItemControl(const QStyleOption *option, QPainter *painter,
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
      const auto color(_style->_helper->separatorColor(palette));
      _style->_helper->renderSeparator(
          painter,
          rect.adjusted(Render::MenuItem_MarginWidth * 2, 0,
                        -Render::MenuItem_MarginWidth * 2, 0),
          color);
      return true;

    } else {
      /*
       * separator can have a title and an icon
       * in that case they are rendered as menu title buttons
       */
      QStyleOptionToolButton copy(
          _style->separatorMenuItemOption(menuItemOption, widget));
      _style->renderMenuTitle(&copy, painter, widget);

      return true;
    }
  }

  // store state
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool selected(enabled && (state & QStyle::State_Selected));
  const bool sunken(enabled && (state & (QStyle::State_On | QStyle::State_Sunken)));
  const bool reverseLayout(option->direction == Qt::RightToLeft);
  const bool useStrongFocus(StyleConfigData::menuItemDrawStrongFocus());

  _style->_animations->inputWidgetEngine().updateState(widget, AnimationHover,
                                               selected);
  _style->_animations->inputWidgetEngine().buttonAnimationMode(widget);
  _style->_animations->inputWidgetEngine().buttonOpacity(widget);

  // render hover and focus
  if (selected || sunken) {
    const auto color =
        useStrongFocus ? _style->_helper->focusColor(palette).darker(sunken ? 120 : 0)
                       : _style->_helper->separatorColor(palette);
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
  auto contentsRect(_style->insideMargin(
      rect, Render::MenuItem_MarginWidth,
      (Render::MenuItem_MarginHeight + StyleConfigData::menuItemHeight())));

  // define relevant rectangles
  // checkbox
  QRect checkBoxRect;
  if (menuItemOption->menuHasCheckableItems) {
    checkBoxRect =
        QRect(contentsRect.left(),
              contentsRect.top() +
                  (contentsRect.height() - Render::CheckBox_Size) / 2,
              Render::CheckBox_Size, Render::CheckBox_Size);
    contentsRect.setLeft(checkBoxRect.right() + Render::MenuItem_ItemSpacing +
                         1);
  } else {
    // always have some left margin
    contentsRect.setLeft(rect.left() + 3 * Render::MenuItem_ItemSpacing);
  }

  // render checkbox indicator
  if (menuItemOption->checkType == QStyleOptionMenuItem::NonExclusive) {
    checkBoxRect = _style->visualRect(option, checkBoxRect);

    // checkbox state

    CheckBoxState state(menuItemOption->checked ? CheckOn : CheckOff);
    // const auto color( _style->_helper->checkBoxIndicatorColor( palette, false,
    // enabled && active ) ); const auto background(state == CheckOn ?
    // palette.color(QPalette::Highlight) : palette.color(QPalette::Button));
    //_style->_helper->renderCheckBoxBackground( painter, checkBoxRect, palette.color(
    // QPalette::Window ), sunken );    //not needed
    _style->_helper->renderCheckBox(painter, checkBoxRect, palette, true, sunken,
                            (selected || sunken), state, windowActive);

  } else if (menuItemOption->checkType == QStyleOptionMenuItem::Exclusive) {
    checkBoxRect = _style->visualRect(option, checkBoxRect);

    const bool active(menuItemOption->checked);
    // const auto shadow( _style->_helper->shadowColor( palette ) );
    // const auto color( _style->_helper->checkBoxIndicatorColor( palette, false,
    // enabled && active ) );
    //_style->_helper->renderRadioButtonBackground( painter, checkBoxRect,
    // palette.color( QPalette::Window ), sunken ); //not needed
    _style->_helper->renderRadioButton(painter, checkBoxRect, palette,
                               (selected || sunken), sunken,
                               active ? RadioOn : RadioOff, true);
  }

  // icon
  int iconWidth = 0;
  const bool showIcon(_style->showIconsInMenuItems());
  if (showIcon)
    iconWidth = _style->isQtQuickControl(option, widget)
                    ? qMax(_style->pixelMetric(QStyle::PM_SmallIconSize, option, widget),
                           menuItemOption->maxIconWidth)
                    : menuItemOption->maxIconWidth;

  QRect iconRect;
  if (showIcon && iconWidth > 0) {
    iconRect =
        QRect(contentsRect.left(),
              contentsRect.top() + (contentsRect.height() - iconWidth) / 2,
              iconWidth, iconWidth);
    contentsRect.setLeft(iconRect.right() + Render::MenuItem_ItemSpacing * 2);
    const QSize iconSize(_style->pixelMetric(QStyle::PM_SmallIconSize, option, widget),
                         _style->pixelMetric(QStyle::PM_SmallIconSize, option, widget));
    iconRect = _style->centerRect(iconRect, iconSize);
  }

  if (showIcon && !menuItemOption->icon.isNull()) {
    iconRect = _style->visualRect(option, iconRect);

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
        _style->_helper->coloredIcon(menuItemOption->icon, menuItemOption->palette,
                             iconRect.size(), dpr, mode, iconState);
    _style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
  }

  // arrow
  QRect arrowRect(
      contentsRect.right() - Render::MenuButton_IndicatorWidth + 1,
      contentsRect.top() +
          (contentsRect.height() - Render::MenuButton_IndicatorWidth) / 2,
      Render::MenuButton_IndicatorWidth, Render::MenuButton_IndicatorWidth);
  contentsRect.setRight(arrowRect.left() - Render::MenuItem_ItemSpacing - 1);

  if (menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu) {
    // apply right-to-left layout
    arrowRect = _style->visualRect(option, arrowRect);

    // arrow orientation
    const ArrowOrientation orientation(reverseLayout ? ArrowLeft : ArrowRight);

    // color
    QColor arrowColor;
    if (useStrongFocus && (selected || sunken))
      arrowColor = palette.color(QPalette::HighlightedText);
    else if (sunken)
      arrowColor = _style->_helper->focusColor(palette);
    else if (selected)
      arrowColor = _style->_helper->hoverColor(palette);
    else
      arrowColor = _style->_helper->arrowColor(palette, QPalette::Text);

    // render
    _style->_helper->renderArrow(painter, arrowRect, arrowColor, orientation);
  }

  // text
  auto textRect = contentsRect;
  if (!menuItemOption->text.isEmpty()) {
    // adjust textRect
    QString text = menuItemOption->text;
    textRect = _style->centerRect(
        textRect, textRect.width(),
        option->fontMetrics.size(_style->_mnemonics->textFlags(), text).height());
    textRect = _style->visualRect(option, textRect);

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
      _style->drawItemText(painter, textRect, textFlags, p, enabled, accelerator, role);
    }

    // render text
    const int textFlags(Qt::AlignVCenter |
                        (reverseLayout ? Qt::AlignRight : Qt::AlignLeft) |
                        _style->_mnemonics->textFlags());
    textRect = option->fontMetrics.boundingRect(textRect, textFlags, text);
    _style->drawItemText(painter, textRect, textFlags, palette, enabled, text, role);

    // render hover and focus
    if (!useStrongFocus && (selected || sunken)) {
      QColor outlineColor;
      if (sunken)
        outlineColor = _style->_helper->focusColor(palette);
      else if (selected)
        outlineColor = _style->_helper->hoverColor(palette);

      _style->_helper->renderFocusLine(painter, textRect, outlineColor);
    }
  }

  return true;
}
void Helper::renderMenuFrame(QPainter *painter, const QRect &rect,
                             const QColor &color, const QColor &outline,
                             bool roundCorners) const {
  // set brush
  if (color.isValid())
    painter->setBrush(color);
  else
    painter->setBrush(Qt::NoBrush);

  if (roundCorners) {
    Render::WidgetSpec spec = Render::MenuFrameSpec;
    spec.geom.fixedRadius(StyleConfigData::menuItemRadius());
    spec.fill(Render::StateStyle<Render::Fill>(Render::Fill(color)));
    if (outline.isValid())
      spec.border(Render::Border(outline, 1.0, Render::BorderAlign::Inside));

    Render::WidgetInteractionState wstate;
    wstate.enabled = true;

    Render::WidgetRenderer(this).render(painter, rect, spec, wstate);

  } else {
    painter->setRenderHint(QPainter::Antialiasing, false);
    QRect frameRect(rect);
    if (outline.isValid()) {
      painter->setPen(outline);
      frameRect.adjust(0, 0, -1, -1);

    } else
      painter->setPen(Qt::NoPen);

    painter->drawRect(frameRect);
  }
}

bool Helper::renderBlurredBackground(QPainter *painter, QWidget *window,
                                     const QRect &sourceRectInWindow,
                                     const QRect &targetRect,
                                     int blurRadius) const {
  if (!window || !window->isWindow() || sourceRectInWindow.isEmpty() ||
      targetRect.isEmpty())
    return false;

  // Guard against re-entrancy: window->grab() triggers paint events on all
  // child widgets, which can call back here before the grab completes. This
  // fixes a crash in Konsole where blurred backgrounds are being rendered
  static thread_local QSet<QWidget *> activeGrabs;
  if (activeGrabs.contains(window))
    return false;
  activeGrabs.insert(window);

  QPixmap grab = window->grab(sourceRectInWindow);
  activeGrabs.remove(window);
  if (grab.isNull() || grab.size().isEmpty())
    return false;

  QGraphicsScene scene;
  QGraphicsPixmapItem *item = scene.addPixmap(grab);
  if (!item)
    return false;

  auto *effect = new QGraphicsBlurEffect;
  effect->setBlurRadius(blurRadius);
  effect->setBlurHints(QGraphicsBlurEffect::QualityHint);
  item->setGraphicsEffect(effect);

  scene.setSceneRect(0, 0, grab.width(), grab.height());
  scene.render(painter, targetRect, scene.sceneRect());
  return true;
}

//* QMenu: translucent background, pointing-hand cursor, blur registration
bool Style::polishMenu(QWidget *widget) {
  if (!qobject_cast<QMenu *>(widget))
    return false;

  setTranslucentBackground(widget);
  widget->setCursor(Qt::PointingHandCursor);

  if (widget->testAttribute(Qt::WA_TranslucentBackground) &&
      StyleConfigData::menuOpacity() < 100) {
    _blurHelper->registerWidget(widget->window(), _app.isDolphin);
  }
  return true;
}

//* reset cursor set in polishMenu()
void Style::unpolishMenu(QWidget *widget) {
  if (qobject_cast<QMenu *>(widget)) {
    widget->unsetCursor();
  }
}
bool Style::drawPanelMenuPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::MenuControl(this).drawPanelMenuPrimitive(option, painter, widget);
}

bool Style::drawMenuBarEmptyAreaControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::MenuControl(this).drawMenuBarEmptyAreaControl(option, painter, widget);
}

bool Style::drawMenuBarItemControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::MenuControl(this).drawMenuBarItemControl(option, painter, widget);
}

bool Style::drawMenuItemControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::MenuControl(this).drawMenuItemControl(option, painter, widget);
}
} // namespace BlossomUI
