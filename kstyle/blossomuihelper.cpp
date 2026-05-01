/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "blossomuihelper.h"

#if __has_include("config-blossomui.h")
#include "config-blossomui.h"
#else
#define BLOSSOMUI_HAVE_QTQUICK 0
#define BLOSSOMUI_HAVE_KSTYLE 0
#define BLOSSOMUI_HAVE_X11 0
#define BLOSSOMUI_HAVE_KWAYLAND 0
#endif

#include "blossomui.h"

#include <KColorUtils>
#include <KIconLoader>
#include <KWindowSystem>

#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QtMath>

#if __has_include(<KX11Extras>)
#include <KX11Extras>
#endif

#if BLOSSOMUI_HAVE_X11
#include <QX11Info>
#endif

#include <QDialog>
#include <QRegion>
#include <algorithm>

#include <QEvent>
#include <QSet>

// #include <QDebug>

namespace BlossomUI {

//* contrast for arrow and treeline rendering
static const qreal arrowShade = 0.15;

//____________________________________________________________________
PaletteChangedEventFilter::PaletteChangedEventFilter(Helper *helper)
    : QObject(helper), _helper(helper) {}

//____________________________________________________________________
bool PaletteChangedEventFilter::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() != QEvent::ApplicationPaletteChange || watched != qApp) {
    return QObject::eventFilter(watched, event);
  }
  if (!qApp->property("KDE_COLOR_SCHEME_PATH").isValid()) {
    return QObject::eventFilter(watched, event);
  }
  const auto path = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
  if (!path.isEmpty()) {
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group(config.group(QStringLiteral("WM")));
    const QPalette palette(QApplication::palette());
    _helper->_activeTitleBarColor =
        group.readEntry("activeBackground",
                        palette.color(QPalette::Active, QPalette::Highlight));
    _helper->_activeTitleBarTextColor = group.readEntry(
        "activeForeground",
        palette.color(QPalette::Active, QPalette::HighlightedText));
    _helper->_inactiveTitleBarColor =
        group.readEntry("inactiveBackground",
                        palette.color(QPalette::Disabled, QPalette::Highlight));
    _helper->_inactiveTitleBarTextColor = group.readEntry(
        "inactiveForeground",
        palette.color(QPalette::Disabled, QPalette::HighlightedText));
  }
  return QObject::eventFilter(watched, event);
}

//____________________________________________________________________
Helper::Helper(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent), _config(std::move(config)),
      _eventFilter(new PaletteChangedEventFilter(this)) {}

//____________________________________________________________________
KSharedConfig::Ptr Helper::config() const { return _config; }

//____________________________________________________________________
void Helper::loadConfig() {
  _viewFocusBrush =
      KStatefulBrush(KColorScheme::View, KColorScheme::FocusColor);
  _viewHoverBrush =
      KStatefulBrush(KColorScheme::View, KColorScheme::HoverColor);
  _viewNegativeTextBrush =
      KStatefulBrush(KColorScheme::View, KColorScheme::NegativeText);
  _windowAlternateBackgroundBrush =
      KStatefulBrush(KColorScheme::Window, KColorScheme::AlternateBackground);

  const QPalette palette(QApplication::palette());

  KConfig config(qApp->property("KDE_COLOR_SCHEME_PATH").toString(),
                 KConfig::SimpleConfig);
  KConfigGroup appGroup(config.group("WM"));
  KConfigGroup globalGroup(_config->group("WM"));
  _activeTitleBarColor = appGroup.readEntry(
      "activeBackground",
      globalGroup.readEntry(
          "activeBackground",
          palette.color(QPalette::Active, QPalette::Highlight)));
  _activeTitleBarTextColor = appGroup.readEntry(
      "activeForeground",
      globalGroup.readEntry(
          "activeForeground",
          palette.color(QPalette::Active, QPalette::HighlightedText)));
  _inactiveTitleBarColor = appGroup.readEntry(
      "inactiveBackground",
      globalGroup.readEntry(
          "inactiveBackground",
          palette.color(QPalette::Disabled, QPalette::Highlight)));
  _inactiveTitleBarTextColor = appGroup.readEntry(
      "inactiveForeground",
      globalGroup.readEntry(
          "inactiveForeground",
          palette.color(QPalette::Disabled, QPalette::HighlightedText)));
}

//____________________________________________________________________
QColor Helper::frameOutlineColor(const QPalette &palette, bool mouseOver,
                                 bool hasFocus, qreal opacity,
                                 AnimationMode mode) const {
  QColor outline(KColorUtils::mix(palette.color(QPalette::Window),
                                  palette.color(QPalette::WindowText), 0.12));
  // QColor outline( palette.color( QPalette::QPalette::AlternateBase ) );

  // focus takes precedence over hover
  if (mode == AnimationFocus) {
    const QColor focus(focusColor(palette));
    const QColor hover(hoverColor(palette));

    if (mouseOver)
      outline = KColorUtils::mix(hover, focus, opacity);
    else
      outline = KColorUtils::mix(outline, focus, opacity);

  } else if (hasFocus) {
    outline = focusColor(palette);

  } else if (mode == AnimationHover) {
    const QColor hover(hoverColor(palette));
    outline = KColorUtils::mix(outline, hover, opacity);

  } else if (mouseOver) {
    outline = hoverColor(palette);
  }

  return outline;
}

//____________________________________________________________________
QColor Helper::hoverOutlineColor(const QPalette &palette) const {
  return KColorUtils::mix(hoverColor(palette),
                          palette.color(QPalette::WindowText), 0.15);
}

//____________________________________________________________________
QColor Helper::buttonFocusOutlineColor(const QPalette &palette) const {
  return KColorUtils::mix(focusColor(palette),
                          palette.color(QPalette::ButtonText), 0.15);
}

//____________________________________________________________________
QColor Helper::buttonHoverOutlineColor(const QPalette &palette) const {
  return KColorUtils::mix(hoverColor(palette),
                          palette.color(QPalette::ButtonText), 0.15);
}

//____________________________________________________________________
QColor Helper::sidePanelOutlineColor(const QPalette &palette) const {
  QColor outline(qGray(palette.color(QPalette::Window).rgb()) > 150
                     ? QColor(0, 0, 0, 20)
                     : QColor(0, 0, 0, 50));
  return outline;
}

//____________________________________________________________________
QColor Helper::frameBackgroundColor(const QPalette &palette,
                                    QPalette::ColorGroup group) const {
  return KColorUtils::mix(palette.color(group, QPalette::Window),
                          palette.color(group, QPalette::Base), 0.3);
}

//____________________________________________________________________
QColor Helper::arrowColor(const QPalette &palette, QPalette::ColorGroup group,
                          QPalette::ColorRole role) const {
  switch (role) {
  case QPalette::Text:
    return KColorUtils::mix(palette.color(group, QPalette::Text),
                            palette.color(group, QPalette::Base), arrowShade);
  case QPalette::WindowText:
    return KColorUtils::mix(palette.color(group, QPalette::WindowText),
                            palette.color(group, QPalette::Window), arrowShade);
  case QPalette::ButtonText:
    return KColorUtils::mix(palette.color(group, QPalette::ButtonText),
                            palette.color(group, QPalette::Button), arrowShade);
  default:
    return palette.color(group, role);
  }
}

//____________________________________________________________________
QColor Helper::arrowColor(const QPalette &palette, bool mouseOver,
                          bool hasFocus, qreal opacity,
                          AnimationMode mode) const {
  QColor outline(arrowColor(palette, QPalette::WindowText));
  if (mode == AnimationHover) {
    const QColor focus(focusColor(palette));
    const QColor hover(hoverColor(palette));
    if (hasFocus)
      outline = KColorUtils::mix(focus, hover, opacity);
    else
      outline = KColorUtils::mix(outline, hover, opacity);

  } else if (mouseOver) {
    // fix skanlite arrow color bug (mouseOver shows dark color (focusColor),
    // not light color (hoverColor))
    outline = focusColor(palette);

  } else if (mode == AnimationFocus) {
    const QColor focus(focusColor(palette));
    outline = KColorUtils::mix(outline, focus, opacity);

  } else if (hasFocus) {
    outline = focusColor(palette);
  }

  return outline;
}

//____________________________________________________________________
QColor Helper::buttonOutlineColor(const QPalette &palette, bool mouseOver,
                                  bool hasFocus, qreal opacity,
                                  AnimationMode mode) const {
  QColor outline(KColorUtils::mix(palette.color(QPalette::Button),
                                  palette.color(QPalette::ButtonText), 0.3));
  if (mode == AnimationHover) {
    if (hasFocus) {
      const QColor focus(buttonFocusOutlineColor(palette));
      const QColor hover(buttonHoverOutlineColor(palette));
      outline = KColorUtils::mix(focus, hover, opacity);

    } else {
      const QColor hover(hoverColor(palette));
      outline = KColorUtils::mix(outline, hover, opacity);
    }

  } else if (mouseOver) {
    if (hasFocus)
      outline = buttonHoverOutlineColor(palette);
    else
      outline = hoverColor(palette);

  } else if (mode == AnimationFocus) {
    const QColor focus(buttonFocusOutlineColor(palette));
    outline = KColorUtils::mix(outline, focus, opacity);

  } else if (hasFocus) {
    outline = buttonFocusOutlineColor(palette);
  }

  return outline;
}

//____________________________________________________________________
QColor Helper::buttonBackgroundColor(const QPalette &palette, bool mouseOver,
                                     bool hasFocus, bool sunken, qreal opacity,
                                     AnimationMode mode) const {
  const QColor base(palette.color(QPalette::Button));
  QColor background(
      sunken ? KColorUtils::mix(base, palette.color(QPalette::ButtonText), 0.2)
             : base);
  const QColor tinted(KColorUtils::mix(base, focusColor(palette), 0.28));
  const QColor tintedHover(KColorUtils::mix(base, focusColor(palette), 0.38));

  if (mode == AnimationHover) {
    if (hasFocus)
      background = KColorUtils::mix(tinted, tintedHover, opacity);

  } else if (mouseOver && hasFocus) {
    background = tintedHover;

  } else if (mode == AnimationFocus) {
    background = KColorUtils::mix(background, tinted, opacity);

  } else if (hasFocus) {
    background = tinted;
  }

  return background;
}

//____________________________________________________________________
QColor Helper::toolButtonColor(const QPalette &palette, bool mouseOver,
                               bool hasFocus, bool sunken, qreal opacity,
                               AnimationMode mode) const {
  QColor outline;
  const QColor hoverColor =
      alphaColor(palette.color(QPalette::WindowText), 0.1);
  const QColor focusColor(this->focusColor(palette));
  const QColor sunkenColor =
      alphaColor(palette.color(QPalette::WindowText), 0.2);

  // hover takes precedence over focus
  if (mode == AnimationHover) {
    if (hasFocus)
      outline = KColorUtils::mix(focusColor, hoverColor, opacity);
    else if (sunken)
      outline = sunkenColor;
    else
      outline = alphaColor(hoverColor, opacity);

  } else if (mouseOver) {
    outline = hoverColor;

  } else if (mode == AnimationFocus) {
    if (sunken)
      outline = KColorUtils::mix(sunkenColor, focusColor, opacity);
    else
      outline = alphaColor(focusColor, opacity);

  } else if (hasFocus) {
    outline = focusColor;

  } else if (sunken) {
    outline = sunkenColor;
  }

  return outline;
}

//____________________________________________________________________
QColor Helper::sliderOutlineColor(const QPalette &palette, bool mouseOver,
                                  bool hasFocus, qreal opacity,
                                  AnimationMode mode) const {
  QColor outline(KColorUtils::mix(palette.color(QPalette::Window),
                                  palette.color(QPalette::WindowText), 0.4));

  // hover takes precedence over focus
  if (mode == AnimationHover) {
    const QColor hover(hoverColor(palette));
    const QColor focus(focusColor(palette));
    if (hasFocus)
      outline = KColorUtils::mix(focus, hover, opacity);
    else
      outline = KColorUtils::mix(outline, hover, opacity);

  } else if (mouseOver) {
    outline = hoverColor(palette);

  } else if (mode == AnimationFocus) {
    const QColor focus(focusColor(palette));
    outline = KColorUtils::mix(outline, focus, opacity);

  } else if (hasFocus) {
    outline = focusColor(palette);
  }

  if (hasFocus || mouseOver)
    outline = QColor(255, 255, 255, 30);
  else
    outline = QColor();
  return outline;
}

//____________________________________________________________________
QColor Helper::scrollBarHandleColor(const QPalette &palette, bool mouseOver,
                                    bool hasFocus, qreal opacity,
                                    AnimationMode mode) const {
  QColor color(alphaColor(palette.color(QPalette::WindowText), 0.5));

  // hover takes precedence over focus
  if (mode == AnimationHover) {
    const QColor hover(hoverColor(palette));
    const QColor focus(focusColor(palette));
    if (hasFocus)
      color = KColorUtils::mix(focus, hover, opacity);
    else
      color = KColorUtils::mix(color, hover, opacity);

  } else if (mouseOver) {
    color = hoverColor(palette);

  } else if (mode == AnimationFocus) {
    const QColor focus(focusColor(palette));
    color = KColorUtils::mix(color, focus, opacity);

  } else if (hasFocus) {
    color = focusColor(palette);
  }

  return color;
}

//______________________________________________________________________________
QColor Helper::checkBoxIndicatorColor(const QPalette &palette, bool mouseOver,
                                      bool active, qreal opacity,
                                      AnimationMode mode) const {
  QColor color(KColorUtils::mix(palette.color(QPalette::Window),
                                palette.color(QPalette::WindowText), 0.6));
  if (mode == AnimationHover) {
    const QColor focus(focusColor(palette));
    const QColor hover(hoverColor(palette));
    if (active)
      color = KColorUtils::mix(focus, hover, opacity);
    else
      color = KColorUtils::mix(color, hover, opacity);

  } else if (mouseOver) {
    color = hoverColor(palette);

  } else if (active) {
    color = focusColor(palette);
  }

  return color;
}

//______________________________________________________________________________
QColor Helper::separatorColor(const QPalette &palette) const {
  return isDarkTheme(palette) ? QColor(255, 255, 255, 16) : QColor(0, 0, 0, 16);
}

//______________________________________________________________________________
QPalette Helper::disabledPalette(const QPalette &source, qreal ratio) const {
  QPalette copy(source);

  const QList<QPalette::ColorRole> roles = {
      QPalette::Window,     QPalette::Highlight, QPalette::WindowText,
      QPalette::ButtonText, QPalette::Text,      QPalette::Button};
  foreach (const QPalette::ColorRole &role, roles) {
    copy.setColor(role, KColorUtils::mix(source.color(QPalette::Active, role),
                                         source.color(QPalette::Disabled, role),
                                         1.0 - ratio));
  }

  return copy;
}

//____________________________________________________________________
QColor Helper::alphaColor(QColor color, qreal alpha) const {
  if (alpha >= 0 && alpha < 1.0) {
    color.setAlphaF(alpha * color.alphaF());
  }
  return color;
}

//______________________________________________________________________________
void Helper::renderDebugFrame(QPainter *painter, const QRect &rect) const {
  painter->save();
  painter->setRenderHints(QPainter::Antialiasing);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(Qt::red);
  painter->drawRect(strokedRect(rect));
  painter->restore();
}

//______________________________________________________________________________
void Helper::renderFocusLine(QPainter *painter, const QRect &rect,
                             const QColor &color) const {
  if (!color.isValid())
    return;

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(color);

  painter->translate(0, 2);
  painter->drawLine(rect.bottomLeft(), rect.bottomRight());
  painter->restore();
}

//______________________________________________________________________________
void Helper::renderFrame(QPainter *painter, const QRect &rect,
                         const QColor &color, const bool windowActive,
                         const bool enabled) const {
  painter->setRenderHint(QPainter::Antialiasing);

  const qreal radius = frameRadius(PenWidth::NoPen);

  painter->setPen(Qt::NoPen);
  painter->setBrush(color);
  painter->drawRoundedRect(QRectF(rect), radius, radius);

  const QColor borderColor(
      alphaColor(QApplication::palette().color(QPalette::WindowText),
                 enabled ? 0.12 : 0.08));
  QPen borderPen(borderColor, 1);
  borderPen.setCosmetic(true);
  painter->setPen(borderPen);
  painter->setBrush(Qt::NoBrush);
  painter->drawRoundedRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5),
                           radius - 0.5, radius - 0.5);

  Q_UNUSED(windowActive)
}

//______________________________________________________________________________
void Helper::renderSidePanelFrame(QPainter *painter, const QRect &rect,
                                  const QColor &outline, Side side) const {
  // check color
  if (!outline.isValid())
    return;

  // adjust rect
  QRectF frameRect(strokedRect(rect));

  // setup painter
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(outline);

  // render
  switch (side) {
  default:
  case SideLeft:
    painter->drawLine(frameRect.topRight(), frameRect.bottomRight());
    break;

  case SideTop:
    painter->drawLine(frameRect.topLeft(), frameRect.topRight());
    break;

  case SideRight:
    painter->drawLine(frameRect.topLeft(), frameRect.bottomLeft());
    break;

  case SideBottom:
    painter->drawLine(frameRect.bottomLeft(), frameRect.bottomRight());
    break;

  case AllSides: {
    const qreal radius(frameRadius(PenWidth::Frame, -1));
    painter->drawRoundedRect(frameRect, radius, radius);
    break;
  }
  }
}

//______________________________________________________________________________
void Helper::renderMenuFrame(QPainter *painter, const QRect &rect,
                             const QColor &color, const QColor &outline,
                             bool roundCorners) const {
  // set brush
  if (color.isValid())
    painter->setBrush(color);
  else
    painter->setBrush(Qt::NoBrush);

  if (roundCorners) {
    painter->setRenderHint(QPainter::Antialiasing);

    QRectF frameRect = QRectF(rect);

    qreal radius(StyleConfigData::menuItemRadius());

    painter->setPen(Qt::NoPen);

    // render
    painter->drawRoundedRect(frameRect, radius, radius);

    // outline
    if (outline.isValid()) {
      painter->setPen(outline);
      painter->setBrush(Qt::NoBrush);
      frameRect = strokedRect(frameRect);
      radius += 0.5; // enhance pixel aligment
      painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

      painter->drawRoundedRect(frameRect, radius, radius);
    }

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

//______________________________________________________________________________
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

//______________________________________________________________________________
void Helper::renderOutline(QPainter *painter, const QRectF &rect,
                           const int radius, const int outlineStrength) const {
  painter->setPen(QColor(0, 0, 0, outlineStrength));
  painter->setBrush(Qt::NoBrush);
  QRectF frameRect(QRectF(rect.left() - 1, rect.top() - 1, rect.width() + 2,
                          rect.height() + 2));
  frameRect.adjust(0.5, 0.5, -0.5, -0.5);
  painter->drawRoundedRect(frameRect, radius, radius);
  painter->setPen(Qt::NoPen);
}

//______________________________________________________________________________
void Helper::renderBoxShadow(QPainter *painter, const QRect &rect,
                             const int xOffset, const int yOffset,
                             const int size, const QColor &color,
                             const int cornerRadius, const bool active,
                             TileSet::Tiles tiles) const {
  if (!StyleConfigData::widgetDrawShadow())
    return;
  Q_UNUSED(active)
  // if (!active) {renderOutline(painter, rect, cornerRadius, 30);return;}
  CustomShadowParams params =
      CustomShadowParams(QPoint(xOffset, yOffset), size, color);
  TileSet shadow = ShadowHelper::shadowTiles(cornerRadius, params);
  shadow.render(rect.adjusted(-params.radius, -params.radius,
                              params.radius + params.offset.x(),
                              params.radius + params.offset.y()),
                painter, tiles);
  // qDebug() << "shadow on: " << rect.adjusted(-params.radius, -params.radius,
  // params.radius, params.radius);
}

//______________________________________________________________________________
void Helper::renderEllipseShadow(QPainter *painter, const QRectF &rect,
                                 QColor color, const int size,
                                 const float param1, const float param2,
                                 const int xOffset, const int yOffset,
                                 const bool outline,
                                 const int outlineStrength) const {
  if (!StyleConfigData::widgetDrawShadow())
    return;

  painter->setPen(Qt::NoPen);

  if (outline) {
    painter->setBrush(QColor(0, 0, 0, outlineStrength));
    painter->drawEllipse(QRect(rect.left() - 1, rect.top() - 1,
                               rect.width() + 2, rect.height() + 2));
  }

  if (size < 1)
    return;
  if (color.alphaF() < 0.01)
    return;

  // temporaty values
  int tx = rect.left() - size + xOffset;
  int ty = rect.top() - size + yOffset;
  int tw = rect.width() + size * 2;
  int th = rect.height() + size * 2;
  float alpha = color.alphaF();

  while (tx <= rect.left() + qMax(xOffset, yOffset)) {
    color.setAlpha(alpha);
    painter->setBrush(color);

    painter->drawEllipse(QRect(tx, ty, tw, th));

    tx++;
    ty++;
    tw -= 2;
    th -= 2;
    alpha += param1 + alpha / param2;
  }
}

//______________________________________________________________________________
void Helper::topHighlight(QPainter *painter, const QRectF &rect,
                          const int radius, const QColor &color) const {
  QPixmap pixmap = QPixmap(rect.width(), rect.height());
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);

  p.setRenderHint(QPainter::Antialiasing);

  p.setPen(Qt::NoPen);
  p.setBrush(color);
  p.drawRoundedRect(QRect(0, 0, rect.width(), rect.height()), radius, radius);

  p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
  p.setBrush(Qt::black);

  if (StyleConfigData::fullOutline()) {
    p.drawRoundedRect(QRect(1, 1, rect.width() - 2, rect.height() - 2), radius,
                      radius);
    painter->drawPixmap(QRect(rect.x(), rect.y(), rect.width(), rect.height()),
                        pixmap);
  } else {
    p.drawRoundedRect(QRect(0, 1, rect.width(), rect.height()), radius, radius);
    painter->drawPixmap(QRect(rect.x(), rect.y(), rect.width(), rect.height()),
                        pixmap);
  }
}

//______________________________________________________________________________
void Helper::renderButtonFrame(QPainter *painter, const QRect &rect,
                               const QColor &color, const QPalette &palette,
                               const bool hasFocus, const bool sunken,
                               const bool mouseOver, const bool enabled,
                               const bool windowActive,
                               const AnimationMode mode, const qreal opacity,
                               const QPointF &ripplePos,
                               const qreal pressOpacity) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  // reduce the size of the actual button, the rest will be the shadow
  QRectF frameRect(
      rect.adjusted(Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth,
                    -Metrics::Frame_FrameWidth, -Metrics::Frame_FrameWidth));
  qreal maxRadius = qMin(frameRect.height(), frameRect.width()) / 2.0;
  qreal radius =
      qMin(buttonFrameRadius() - qreal(Metrics::Frame_FrameWidth), maxRadius);

  // shift the whole frame down 1px when pressed
  if (sunken)
    frameRect.translate(0, 1);

  QColor fill(color.isValid() ? color : palette.color(QPalette::Button));
  if (sunken)
    fill = fill.darker(105);
  else if (mouseOver)
    fill = fill.lighter(102);

  QColor outline(
      alphaColor(palette.color(QPalette::WindowText), mouseOver ? 0.22 : 0.16));
  if (!enabled) {
    fill = KColorUtils::mix(fill, palette.color(QPalette::Window), 0.4);
    outline = alphaColor(outline, 0.5);
  } else if (hasFocus && !sunken) {
    // use accent-colored border for focus, no separate halo needed
    outline = mode == AnimationFocus
                  ? KColorUtils::mix(
                        alphaColor(palette.color(QPalette::WindowText), 0.16),
                        focusColor(palette), opacity)
                  : focusColor(palette);
  }

  // base fill
  painter->setBrush(fill.isValid() ? fill : Qt::NoBrush);
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(frameRect, radius, radius);

  // border
  if (outline.isValid()) {
    QPen borderPen(outline, 1);
    borderPen.setCosmetic(true);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    const QRectF borderRect(frameRect.adjusted(0.5, 0.5, -0.5, -0.5));
    painter->drawRoundedRect(borderRect, radius, radius);
  }

  const qreal pOpacity = (pressOpacity >= 0.0)        ? pressOpacity
                         : (mode == AnimationPressed) ? opacity
                                                      : -1.0;
  if (pOpacity >= 0.0) {
    painter->save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(frameRect, radius, radius);
    painter->setClipPath(clipPath, Qt::IntersectClip);
    painter->setPen(Qt::NoPen);

    const QPointF center =
        (ripplePos.isNull() || !frameRect.contains(ripplePos))
            ? frameRect.center()
            : ripplePos;

    qreal maxDist = 0;
    for (const QPointF &c : {frameRect.topLeft(), frameRect.topRight(),
                             frameRect.bottomLeft(), frameRect.bottomRight()}) {
      const QPointF d = c - center;
      maxDist = qMax(maxDist, qSqrt(d.x() * d.x() + d.y() * d.y()));
    }
    const qreal finalRadius = maxDist;
    const qreal initRadius = qMin(frameRect.width(), frameRect.height()) / 4.0;

    if (sunken) {
      const qreal r = initRadius + (finalRadius - initRadius) * pOpacity;
      painter->setBrush(
          alphaColor(focusColor(palette), 0.4 * (1.0 - pOpacity)));
      painter->drawEllipse(center, r, r);
    } else {
      painter->setBrush(
          alphaColor(focusColor(palette), 0.2 * qSin(pOpacity * M_PI)));
      painter->drawRoundedRect(frameRect, radius, radius);
    }

    painter->restore();
  }

  Q_UNUSED(windowActive)
}

//______________________________________________________________________________
void Helper::renderToolButtonFrame(QPainter *painter, const QRect &rect,
                                   const QColor &color, bool /*sunken*/) const {
  // do nothing for invalid color
  if (!color.isValid())
    return;

  // setup painter
  painter->setRenderHints(QPainter::Antialiasing);

  const QRectF baseRect(rect.adjusted(1, 1, -1, -1));

  const qreal radius = qMin(buttonFrameRadius(PenWidth::NoPen),
                            0.5 * qMin(baseRect.width(), baseRect.height()));
  painter->setPen(Qt::NoPen);
  painter->setBrush(color);
  painter->drawRoundedRect(baseRect, radius, radius);
}

//______________________________________________________________________________
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

//______________________________________________________________________________
void Helper::renderTabWidgetFrame(QPainter *painter, const QRect &rect,
                                  const QColor &color, Corners corners,
                                  const bool windowActive) const {
  painter->setRenderHint(QPainter::Antialiasing);

  // QRectF frameRect( rect.adjusted( 1, 1, -1, -1 ) );
  QRectF frameRect(
      rect.adjusted(Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth,
                    -Metrics::Frame_FrameWidth, -Metrics::Frame_FrameWidth));
  qreal radius(frameRadius(PenWidth::NoPen, -1));

  // shadow
  renderBoxShadow(painter, frameRect, 0, 1, 5, QColor(0, 0, 0, 115), radius,
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

//______________________________________________________________________________
QMargins Helper::itemViewItemMargins(const QStyleOptionViewItem *option) const {
  QMargins margins(
      Metrics::ItemView_ItemMarginLeft, Metrics::ItemView_ItemMarginTop,
      Metrics::ItemView_ItemMarginRight, Metrics::ItemView_ItemMarginBottom);
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
    margins.setTop(Metrics::ItemView_FirstItemTopMarginHeight);
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

//______________________________________________________________________________
void Helper::renderSelection(QPainter *painter, const QRect &rect,
                             const QColor &color, Corners corners) const {
  renderSelection(painter, rect, color, corners,
                  StyleConfigData::itemViewRadius());
}

//______________________________________________________________________________
void Helper::renderSelection(QPainter *painter, const QRect &rect,
                             const QColor &color, Corners corners,
                             qreal radius) const {
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);
  painter->setBrush(color);

  QPainterPath path(roundedPath(rect, corners, radius));
  painter->drawPath(path);
}

//______________________________________________________________________________
void Helper::renderSeparator(QPainter *painter, const QRect &rect,
                             const QColor &color, bool vertical) const {
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(color);

  if (vertical) {
    painter->translate(rect.width() * 0.5, 0);
    painter->drawLine(rect.topLeft(), rect.bottomLeft());

  } else {
    painter->translate(0, rect.height() * 0.5);
    painter->drawLine(rect.topLeft(), rect.topRight());
  }
}

//______________________________________________________________________________
void Helper::renderLineEdit(QPainter *painter, const QRect &rect,
                            const QColor &background, const QColor &outline,
                            const bool hasFocus, const bool mouseOver,
                            bool enabled, const bool windowActive,
                            const AnimationMode mode,
                            const qreal opacity) const {
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  QRectF frameRect(
      rect.adjusted(Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth,
                    -Metrics::Frame_FrameWidth, -Metrics::Frame_FrameWidth));
  qreal radius(inputFrameRadius(PenWidth::NoPen, -1));

  QColor border(KColorUtils::mix(background, outline, 0.15));
  if (mouseOver && !hasFocus)
    border = KColorUtils::mix(background, outline, 0.3);
  if (hasFocus && mode != AnimationFocus)
    border = KColorUtils::mix(background, outline, 0.65);
  if (mode == AnimationFocus && opacity >= 0) {
    // Smooth fade in both directions: opacity 0 = unfocused, 1 = focused
    border = KColorUtils::mix(background, outline, 0.15 + 0.5 * opacity);
  }

  if (!enabled)
    border = KColorUtils::mix(background, outline, 0.08);

  // focus ring - stroked path for smooth corners (avoids jagged fill in
  // Slint/Qt Quick)
  if (enabled && (hasFocus || (mode == AnimationFocus && opacity > 0))) {
    const qreal ringOpacity = (mode == AnimationFocus) ? opacity : 1.0;
    painter->setBrush(Qt::NoBrush);
    QPen ringPen(alphaColor(outline, 0.5 * ringOpacity), 2, Qt::SolidLine,
                 Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(ringPen);
    const QRectF ringPath(frameRect.adjusted(-1, -1, 1, 1));
    painter->drawRoundedRect(ringPath, radius + 1, radius + 1);
  }

  // base fill + border
  painter->setBrush(background.isValid() ? background : Qt::NoBrush);
  painter->setPen(border.isValid() ? QPen(border, 1) : Qt::NoPen);
  painter->drawRoundedRect(frameRect, radius, radius);

  Q_UNUSED(windowActive)
}

//______________________________________________________________________________
void Helper::renderGroupBox(QPainter *painter, const QRect &rect,
                            const QColor &color, const bool mouseOver) const {
  Q_UNUSED(mouseOver)
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);

  QRectF frameRect(
      rect.adjusted(Metrics::Frame_FrameWidth, Metrics::Frame_FrameWidth,
                    -Metrics::Frame_FrameWidth, -Metrics::Frame_FrameWidth));
  qreal radius(frameRadius(PenWidth::NoPen, -1));

  // set brush
  if (color.isValid())
    painter->setBrush(color);
  else
    painter->setBrush(Qt::NoBrush);

  // render
  painter->drawRoundedRect(frameRect, radius, radius);

  const QColor borderColor(
      alphaColor(QApplication::palette().color(QPalette::WindowText), 0.16));
  QPen borderPen(borderColor, 1);
  borderPen.setCosmetic(true);
  painter->setPen(borderPen);
  painter->setBrush(Qt::NoBrush);
  const QRectF borderRect(frameRect.adjusted(0.5, 0.5, -0.5, -0.5));
  painter->drawRoundedRect(borderRect, radius, radius);
}

//______________________________________________________________________________
void Helper::renderCheckBox(QPainter *painter, const QRect &rect,
                            const QPalette &palette, const bool isInMenu,
                            bool sunken, const bool mouseOver,
                            CheckBoxState state, const bool windowActive,
                            qreal animation) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);

  // copy rect and radius
  QRectF frameRect(rect);
  frameRect.adjust(Metrics::Frame_FrameWidth - 1, Metrics::Frame_FrameWidth - 1,
                   -Metrics::Frame_FrameWidth + 1,
                   -Metrics::Frame_FrameWidth + 1);
  qreal radius(qRound(frameRadius(PenWidth::NoPen) / 2.0));

  const QColor color(palette.color(QPalette::HighlightedText));
  QColor background((state == CheckOn || state == CheckAnimated)
                        ? palette.color(QPalette::Highlight)
                        : palette.color(QPalette::Button));
  const QColor offBackground(palette.color(QPalette::Button));
  if (isInMenu)
    background = background.lighter(110);

  if (sunken)
    frameRect.translate(1, 1);

  QColor border(KColorUtils::mix(
      background, palette.color(QPalette::WindowText), mouseOver ? 0.16 : 0.1));

  if (state == CheckOff) {
    painter->setBrush(mouseOver ? background.lighter(105) : background);
    painter->setPen(QPen(border, 1));
    painter->drawRoundedRect(frameRect, radius, radius);

  } else if (state == CheckOn) { // mark
    painter->setBrush(mouseOver ? background.lighter(105) : background);
    painter->setPen(QPen(border, 1));
    painter->drawRoundedRect(frameRect, radius, radius);

    // draw check mark
    const int x = frameRect.x();
    const int y = frameRect.y();

    QPen pen = QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPainterPath check;
    if (StyleConfigData::useNewCheckBox()) {
      check.moveTo(5 + x, 8.5 + y);
      check.lineTo(7 + x, 11 + y);
      check.lineTo(12 + x, 5 + y);
    } else {
      check.moveTo(5 + x, 7 + y);
      check.lineTo(6 + x, 11 + y);
      check.lineTo(12 + x, 5 + y);
    }
    painter->drawPath(check);

  } else if (state == CheckPartial) {
    painter->setBrush(mouseOver ? background.lighter(105) : background);
    painter->setPen(QPen(border, 1));
    painter->drawRoundedRect(frameRect, radius, radius);

    const int x = frameRect.x();
    const int y = frameRect.y();

    painter->setBrush(color);
    painter->drawEllipse(3 + x, 7 + y, 2, 2);
    painter->drawEllipse(7 + x, 7 + y, 2, 2);
    painter->drawEllipse(11 + x, 7 + y, 2, 2);

  } else if (state == CheckAnimated) {
    if (animation == -1)
      animation = 1.0;

    QColor animatedBackground(
        KColorUtils::mix(offBackground, palette.color(QPalette::Highlight),
                         qBound(0.0, animation, 1.0)));
    background =
        mouseOver ? animatedBackground.lighter(105) : animatedBackground;
    border = KColorUtils::mix(background, palette.color(QPalette::WindowText),
                              mouseOver ? 0.22 : 0.15);
    painter->setBrush(background);
    painter->setPen(QPen(border, 1));
    painter->drawRoundedRect(frameRect, radius, radius);

    // draw check mark
    const int x = frameRect.x();
    const int y = frameRect.y();

    QPen pen = QPen(alphaColor(color, 1.0 * animation), 2, Qt::SolidLine,
                    Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPainterPath check;
    if (StyleConfigData::useNewCheckBox()) {
      check.moveTo(5 + x, 8.5 + y);
      check.lineTo(5 + 2 * animation + x, 8.5 + 2.5 * animation + y);
      check.lineTo(7 + 5 * animation + x, 11 - 4 * animation * 1.5 + y);
    } else {
      check.moveTo(animation * 5 + x, 7 + y);
      check.lineTo(animation * 6 + x, 11 + y);
      check.lineTo(animation * 12 + x, 5 + y);
    }
    painter->drawPath(check);
  }
  Q_UNUSED(windowActive)
}

//______________________________________________________________________________
void Helper::renderSwitch(QPainter *painter, const QRect &rect,
                          const QPalette &palette, bool sunken,
                          const bool mouseOver, CheckBoxState state,
                          qreal animation) const {
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);

  QRectF trackRect(rect);
  trackRect.adjust(Metrics::Frame_FrameWidth - 1, Metrics::Frame_FrameWidth - 1,
                   -Metrics::Frame_FrameWidth + 1,
                   -Metrics::Frame_FrameWidth + 1);
  if (sunken)
    trackRect.translate(1, 1);

  const qreal radius = trackRect.height() / 2.0;
  const int margin = Metrics::Switch_ThumbMargin;
  const qreal thumbDiameter = trackRect.height() - 2 * margin;
  const qreal travel = trackRect.width() - thumbDiameter - 2 * margin;

  // Thumb position: 0 = off (left), 1 = on (right)
  qreal t = 0.0;
  if (state == CheckOn)
    t = 1.0;
  else if (state == CheckAnimated)
    t = (animation >= 0) ? qBound(0.0, animation, 1.0) : 1.0;

  QColor trackColor = palette.color(QPalette::Button);
  if (state == CheckOn || state == CheckAnimated) {
    QColor accent = palette.color(QPalette::Highlight);
    if (state == CheckAnimated && animation >= 0)
      trackColor = KColorUtils::mix(palette.color(QPalette::Button), accent,
                                    qBound(0.0, animation, 1.0));
    else
      trackColor = accent;
  }
  if (mouseOver)
    trackColor = trackColor.lighter(105);

  QColor borderColor = KColorUtils::mix(
      trackColor, palette.color(QPalette::WindowText), mouseOver ? 0.16 : 0.1);
  painter->setBrush(trackColor);
  painter->setPen(QPen(borderColor, 1));
  painter->drawRoundedRect(trackRect, radius, radius);

  // Thumb (circle)
  const qreal thumbX = trackRect.x() + margin + t * travel;
  const qreal thumbY = trackRect.y() + margin;
  QRectF thumbRect(thumbX, thumbY, thumbDiameter, thumbDiameter);

  QColor thumbColor = palette.color(QPalette::Window);
  QColor thumbBorder =
      KColorUtils::mix(thumbColor, palette.color(QPalette::WindowText), 0.12);
  painter->setBrush(thumbColor);
  painter->setPen(QPen(thumbBorder, 1));
  painter->drawEllipse(thumbRect);
}

//______________________________________________________________________________
void Helper::renderRadioButton(QPainter *painter, const QRect &rect,
                               const QPalette &palette, const bool mouseOver,
                               bool sunken, RadioButtonState state,
                               const bool isInMenu, qreal animation) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);

  // setup colors
  const QColor color(palette.color(QPalette::HighlightedText));
  QColor baseBackground(palette.color(QPalette::Button));
  QColor activeBackground(palette.color(QPalette::Highlight));
  if (isInMenu) {
    baseBackground = baseBackground.lighter(110);
    activeBackground = activeBackground.lighter(110);
  }

  // copy rect
  QRectF frameRect(rect);
  frameRect.adjust(Metrics::Frame_FrameWidth - 1, Metrics::Frame_FrameWidth - 1,
                   -Metrics::Frame_FrameWidth + 1,
                   -Metrics::Frame_FrameWidth + 1);

  if (sunken)
    frameRect.translate(1, 1);

  const auto borderBase =
      KColorUtils::mix(baseBackground, palette.color(QPalette::WindowText),
                       mouseOver ? 0.2 : 0.14);

  if (state == RadioAnimated) {
    if (animation == -1)
      animation = 1.0;

    const qreal clamped = qBound(0.0, animation, 1.0);
    const QColor animatedBackground(
        KColorUtils::mix(baseBackground, activeBackground, clamped));
    const QColor border(KColorUtils::mix(animatedBackground,
                                         palette.color(QPalette::WindowText),
                                         mouseOver ? 0.2 : 0.14));

    painter->setBrush(mouseOver ? animatedBackground.lighter(105)
                                : animatedBackground);
    painter->setPen(QPen(border, 1));
    painter->drawEllipse(frameRect);

    const QRectF markerRect(frameRect.adjusted(4, 4, -4, -4));
    painter->setPen(Qt::NoPen);
    painter->setBrush(alphaColor(color, clamped));
    painter->drawEllipse(markerRect);

  } else if (state == RadioOn) {
    const QColor background(activeBackground);
    const QColor border(KColorUtils::mix(background,
                                         palette.color(QPalette::WindowText),
                                         mouseOver ? 0.2 : 0.14));
    painter->setBrush(mouseOver ? background.lighter(105) : background);
    painter->setPen(QPen(border, 1));
    painter->drawEllipse(frameRect);

    const QRectF markerRect(frameRect.adjusted(4, 4, -4, -4));
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(markerRect);

  } else {
    painter->setBrush(mouseOver ? baseBackground.lighter(105) : baseBackground);
    painter->setPen(QPen(borderBase, 1));
    painter->drawEllipse(frameRect);
  }
}

//______________________________________________________________________________
void Helper::renderSliderGroove(QPainter *painter, const QRect &rect,
                                const QColor &color) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);
  const qreal radius(0.5 * static_cast<qreal>(Metrics::Slider_GrooveThickness));

  // content
  if (color.isValid()) {
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(baseRect, radius, radius);
  }
}

//______________________________________________________________________________
void Helper::renderDialGroove(QPainter *painter, const QRect &rect,
                              const QColor &color, qreal first,
                              qreal last) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);

  // content
  if (color.isValid()) {
    const qreal penWidth(Metrics::Slider_GrooveThickness - 2);
    const QRectF grooveRect(rect.adjusted(penWidth / 2, penWidth / 2,
                                          -penWidth / 2, -penWidth / 2));

    // setup angles
    const int angleStart(first * 180 * 16 / M_PI);
    const int angleSpan((last - first) * 180 * 16 / M_PI);

    // setup pen
    if (angleSpan != 0) {
      QPen pen(color, penWidth);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawArc(grooveRect, angleStart, angleSpan);
    }
  }
}

//______________________________________________________________________________
void Helper::renderDialContents(QPainter *painter, const QRect &rect,
                                const QColor &color, qreal first,
                                qreal second) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);

  // content
  if (color.isValid()) {
    // setup groove rect
    const qreal penWidth(Metrics::Slider_GrooveThickness);
    const QRectF grooveRect(rect.adjusted(penWidth / 2, penWidth / 2,
                                          -penWidth / 2, -penWidth / 2));

    // setup angles
    const int angleStart(first * 180 * 16 / M_PI);
    const int angleSpan((second - first) * 180 * 16 / M_PI);

    // setup pen
    if (angleSpan != 0) {
      QPen pen(color, penWidth);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawArc(grooveRect, angleStart, angleSpan);
    }
  }
}

//______________________________________________________________________________
void Helper::renderSliderHandle(QPainter *painter, const QRect &rect,
                                const QColor &color, const QColor &outline,
                                qreal hoverOpacity, bool sunken) const {
  painter->setRenderHint(QPainter::Antialiasing, true);

  // hover circle behind handle: grows from handle size outward
  if (hoverOpacity > 0.0 && outline.isValid()) {
    constexpr qreal maxExpand = Metrics::Slider_HoverMargin;
    const qreal expand = maxExpand * hoverOpacity;
    QRectF hoverRect = QRectF(rect).adjusted(-expand, -expand, expand, expand);
    QColor hoverColor(outline);
    hoverColor.setAlpha(qRound(50 * hoverOpacity));
    painter->setBrush(hoverColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(hoverRect);
  }

  // handle circle on top
  QRectF frameRect(rect);
  frameRect.adjust(3, 3, -3, -3);
  if (sunken)
    frameRect.translate(0, 2);

  QColor fill(color.isValid() ? color : Qt::transparent);
  if (sunken)
    fill = fill.darker(103);
  if (fill.isValid())
    fill.setAlpha(255);
  painter->setBrush(fill);
  if (outline.isValid())
    painter->setPen(QPen(outline, 1));
  else
    painter->setPen(Qt::NoPen);
  painter->drawEllipse(frameRect);
}

//______________________________________________________________________________
void Helper::renderProgressBarGroove(QPainter *painter, const QRect &rect,
                                     const QColor &color) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);

  const qreal radius(0.5 * static_cast<qreal>(Metrics::ProgressBar_Thickness));

  // content
  if (color.isValid()) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(baseRect, radius, radius);
  }
}

//______________________________________________________________________________
void Helper::renderProgressBarBusyContents(QPainter *painter, const QRect &rect,
                                           const QColor &first,
                                           const QColor &second,
                                           bool horizontal, bool reverse,
                                           int progress) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);
  const qreal radius(0.5 * static_cast<qreal>(Metrics::ProgressBar_Thickness));

  // setup brush
  QPixmap pixmap(horizontal ? 2 * Metrics::ProgressBar_BusyIndicatorSize : 1,
                 horizontal ? 1 : 2 * Metrics::ProgressBar_BusyIndicatorSize);
  pixmap.fill(second);
  if (horizontal) {
    QPainter painter(&pixmap);
    painter.setBrush(first);
    painter.setPen(Qt::NoPen);

    progress %= 2 * Metrics::ProgressBar_BusyIndicatorSize;
    if (reverse)
      progress = 2 * Metrics::ProgressBar_BusyIndicatorSize - progress - 1;
    painter.drawRect(QRect(0, 0, Metrics::ProgressBar_BusyIndicatorSize, 1)
                         .translated(progress, 0));

    if (progress > Metrics::ProgressBar_BusyIndicatorSize) {
      painter.drawRect(
          QRect(0, 0, Metrics::ProgressBar_BusyIndicatorSize, 1)
              .translated(progress - 2 * Metrics::ProgressBar_BusyIndicatorSize,
                          0));
    }

  } else {
    QPainter painter(&pixmap);
    painter.setBrush(first);
    painter.setPen(Qt::NoPen);

    progress %= 2 * Metrics::ProgressBar_BusyIndicatorSize;
    progress = 2 * Metrics::ProgressBar_BusyIndicatorSize - progress - 1;
    painter.drawRect(QRect(0, 0, 1, Metrics::ProgressBar_BusyIndicatorSize)
                         .translated(0, progress));

    if (progress > Metrics::ProgressBar_BusyIndicatorSize) {
      painter.drawRect(
          QRect(0, 0, 1, Metrics::ProgressBar_BusyIndicatorSize)
              .translated(0, progress -
                                 2 * Metrics::ProgressBar_BusyIndicatorSize));
    }
  }

  painter->setPen(Qt::NoPen);
  painter->setBrush(pixmap);
  painter->drawRoundedRect(baseRect, radius, radius);
}

//______________________________________________________________________________
void Helper::renderScrollBarHandle(QPainter *painter, const QRectF &rect,
                                   const QColor &color) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);
  const qreal radius(0.5 * std::min({baseRect.width(), baseRect.height()}));

  // content
  if (color.isValid()) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(baseRect, radius, radius);
  }
}

//______________________________________________________________________________
void Helper::renderScrollBarBorder(QPainter *painter, const QRect &rect,
                                   const QColor &color) const {
  // content
  if (color.isValid()) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRect(rect);
  }
}

//______________________________________________________________________________
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

//______________________________________________________________________________
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

//______________________________________________________________________________
void Helper::renderArrow(QPainter *painter, const QRect &rect,
                         const QColor &color,
                         ArrowOrientation orientation) const {
  // define polygon
  QPolygonF arrow;
  switch (orientation) {
  /* The inner points of the normal arrows are not on half pixels because
   * they need to have an even width (up/down) or height (left/right).
   * An even width/height makes them easier to align with other UI elements.
   */
  case ArrowUp:
    arrow =
        QVector<QPointF>{QPointF(-4.5, 1.5), QPointF(0, -3), QPointF(4.5, 1.5)};
    break;
  case ArrowDown:
    arrow = QVector<QPointF>{QPointF(-4.5, -1.5), QPointF(0, 3),
                             QPointF(4.5, -1.5)};
    break;
  case ArrowLeft:
    arrow =
        QVector<QPointF>{QPointF(1.5, -4.5), QPointF(-3, 0), QPointF(1.5, 4.5)};
    break;
  case ArrowRight:
    arrow = QVector<QPointF>{QPointF(-1.5, -4.5), QPointF(3, 0),
                             QPointF(-1.5, 4.5)};
    break;
  case ArrowDownSmall:
    arrow = QVector<QPointF>{QPointF(1.5, 3.5), QPointF(3.5, 5.5),
                             QPointF(5.5, 3.5)};
    break;
  default:
    break;
  }

  painter->save();
  painter->setRenderHints(QPainter::Antialiasing);
  painter->translate(QRectF(rect).center());
  painter->setBrush(Qt::NoBrush);
  QPen pen(color, PenWidth::Symbol);
  pen.setCapStyle(Qt::SquareCap);
  pen.setJoinStyle(Qt::MiterJoin);
  painter->setPen(pen);
  painter->drawPolyline(arrow);
  painter->restore();
}

//______________________________________________________________________________
void Helper::renderDecorationButton(QPainter *painter, const QRect &rect,
                                    const QColor &color, ButtonType buttonType,
                                    bool inverted) const {
  painter->save();
  painter->setViewport(rect);
  painter->setWindow(0, 0, 18, 18);
  painter->setRenderHints(QPainter::Antialiasing);

  // initialize pen
  QPen pen;
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::MiterJoin);

  if (inverted) {
    // render circle
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(QRectF(0, 0, 18, 18));

    // take out the inner part
    painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    painter->setBrush(Qt::NoBrush);
    pen.setColor(Qt::black);

  } else {
    painter->setBrush(Qt::NoBrush);
    pen.setColor(color);
  }

  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::MiterJoin);
  pen.setWidthF(PenWidth::Symbol * qMax(1.0, 18.0 / rect.width()));
  painter->setPen(pen);

  switch (buttonType) {
  case ButtonClose: {
    painter->drawLine(QPointF(5, 5), QPointF(13, 13));
    painter->drawLine(13, 5, 5, 13);
    break;
  }

  case ButtonMaximize: {
    painter->drawPolyline(
        QVector<QPointF>{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)});
    break;
  }

  case ButtonMinimize: {
    painter->drawPolyline(
        QVector<QPointF>{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)});
    break;
  }

  case ButtonRestore: {
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5),
                                          QPointF(13.5, 9), QPointF(9, 13.5)});
    break;
  }

  default:
    break;
  }

  painter->restore();
}

//______________________________________________________________________________
void Helper::renderTransparentArea(QPainter *painter, const QRect &rect) const {
  painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
  painter->fillRect(rect, Qt::black);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

//______________________________________________________________________________
bool Helper::isX11() {
#if BLOSSOMUI_HAVE_X11
  static const bool s_isX11 = KWindowSystem::isPlatformX11();
  return s_isX11;
#endif

  return false;
}

//______________________________________________________________________________
bool Helper::isWayland() {
  static const bool s_isWayland = KWindowSystem::isPlatformWayland();
  return s_isWayland;
}

//______________________________________________________________________________
QRectF Helper::strokedRect(const QRectF &rect, const qreal penWidth) const {
  /* With a pen stroke width of 1, the rectangle should have each of its
   * sides moved inwards by half a pixel. This allows the stroke to be
   * pixel perfect instead of blurry from sitting between pixels and
   * prevents the rectangle with a stroke from becoming larger than the
   * original size of the rectangle.
   */
  qreal adjustment = 0.5 * penWidth;
  return QRectF(rect).adjusted(adjustment, adjustment, -adjustment,
                               -adjustment);
}

//______________________________________________________________________________
QPainterPath Helper::roundedPath(const QRectF &rect, Corners corners,
                                 qreal radius) const {
  radius = qMin(radius, 0.5 * qMin(rect.width(), rect.height()));

  QPainterPath path;

  // simple cases
  if (corners == 0) {
    path.addRect(rect);
    return path;
  }

  if (corners == AllCorners) {
    path.addRoundedRect(rect, radius, radius);
    return path;
  }

  const QSizeF cornerSize(2 * radius, 2 * radius);

  // rotate counterclockwise
  // top left corner
  if (corners & CornerTopLeft) {
    path.moveTo(rect.topLeft() + QPointF(radius, 0));
    path.arcTo(QRectF(rect.topLeft(), cornerSize), 90, 90);

  } else
    path.moveTo(rect.topLeft());

  // bottom left corner
  if (corners & CornerBottomLeft) {
    path.lineTo(rect.bottomLeft() - QPointF(0, radius));
    path.arcTo(QRectF(rect.bottomLeft() - QPointF(0, 2 * radius), cornerSize),
               180, 90);

  } else
    path.lineTo(rect.bottomLeft());

  // bottom right corner
  if (corners & CornerBottomRight) {
    path.lineTo(rect.bottomRight() - QPointF(radius, 0));
    path.arcTo(QRectF(rect.bottomRight() - QPointF(2 * radius, 2 * radius),
                      cornerSize),
               270, 90);

  } else
    path.lineTo(rect.bottomRight());

  // top right corner
  if (corners & CornerTopRight) {
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.arcTo(QRectF(rect.topRight() - QPointF(2 * radius, 0), cornerSize), 0,
               90);

  } else
    path.lineTo(rect.topRight());

  path.closeSubpath();
  return path;
}

//______________________________________________________________________________
QRegion Helper::roundedRectRegion(int w, int h, int radius) {
  return roundedRectRegion(w, h, radius, 1.0);
}

//______________________________________________________________________________
QRegion Helper::roundedRectRegion(int w, int h, int radius,
                                  qreal devicePixelRatio) {
  if (w <= 0 || h <= 0)
    return QRegion();
  if (devicePixelRatio > 1.0) {
    // Align to physical pixels for fractional scaling
    const int pw = qRound(qRound((w)*devicePixelRatio) / devicePixelRatio);
    const int ph = qRound(qRound((h)*devicePixelRatio) / devicePixelRatio);
    w = qMax(1, pw);
    h = qMax(1, ph);
  }
  radius = qBound(0, radius, qMin(w, h) / 2);
  QPainterPath path;
  path.addRoundedRect(QRectF(0, 0, w, h), qreal(radius), qreal(radius));
  return QRegion(path.toFillPolygon().toPolygon());
}

//______________________________________________________________________________
QRegion Helper::roundedRectRegionBottomCorners(int w, int h, int radius,
                                               qreal devicePixelRatio) {
  if (w <= 0 || h <= 0)
    return QRegion();
  if (devicePixelRatio > 1.0) {
    const int pw = qRound(qRound(w * devicePixelRatio) / devicePixelRatio);
    const int ph = qRound(qRound(h * devicePixelRatio) / devicePixelRatio);
    w = qMax(1, pw);
    h = qMax(1, ph);
  }
  radius = qBound(0, radius, qMin(w, h) / 2);
  const QRectF rect(0, 0, w, h);
  const qreal r = qreal(radius);
  const QSizeF cornerSize(2 * r, 2 * r);
  QPainterPath path;
  path.moveTo(rect.topLeft());
  path.lineTo(rect.bottomLeft() - QPointF(0, r));
  path.arcTo(QRectF(rect.bottomLeft() - QPointF(0, 2 * r), cornerSize), 180,
             90);
  path.lineTo(rect.bottomRight() - QPointF(r, 0));
  path.arcTo(QRectF(rect.bottomRight() - QPointF(2 * r, 2 * r), cornerSize),
             270, 90);
  path.lineTo(rect.topRight());
  path.lineTo(rect.topLeft());
  path.closeSubpath();
  return QRegion(path.toFillPolygon().toPolygon());
}

//________________________________________________________________________________________________________
bool Helper::compositingActive() const {
#if BLOSSOMUI_HAVE_X11
  if (isX11()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return KWindowSystem::compositingActive();
#elif __has_include(<KX11Extras>)
    return KX11Extras::compositingActive();
#endif
  }
#endif

  return true;
}

//____________________________________________________________________
bool Helper::hasAlphaChannel(const QWidget *widget) const {
  return compositingActive() && widget &&
         widget->testAttribute(Qt::WA_TranslucentBackground);
}

//____________________________________________________________________
bool Helper::shouldWindowHaveAlpha(const QPalette &palette,
                                   bool isDolphin) const {
  if (_activeTitleBarColor.alphaF() < 1.0 ||
      (StyleConfigData::dolphinSidebarOpacity() < 100 && isDolphin) ||
      palette.color(QPalette::Window).alpha() < 255) {
    return true;
  }
  return false;
}

//______________________________________________________________________________________
qreal Helper::devicePixelRatio(const QPixmap &pixmap) const {
  return pixmap.devicePixelRatio();
}

QPixmap Helper::coloredIcon(const QIcon &icon, const QPalette &palette,
                            const QSize &size, qreal devicePixelRatio,
                            QIcon::Mode mode, QIcon::State state) {
  const QPalette activePalette = KIconLoader::global()->customPalette();
  const bool changePalette = activePalette != palette;
  if (changePalette) {
    KIconLoader::global()->setCustomPalette(palette);
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QPixmap pixmap = icon.pixmap(size, devicePixelRatio, mode, state);
#else
  Q_UNUSED(devicePixelRatio);
  const QPixmap pixmap = icon.pixmap(size, mode, state);
#endif
  if (changePalette) {
    if (activePalette == QPalette()) {
      KIconLoader::global()->resetPalette();
    } else {
      KIconLoader::global()->setCustomPalette(activePalette);
    }
  }
  return pixmap;
}

bool Helper::shouldDrawToolsArea(const QWidget *widget) const {
  if (!widget) {
    return false;
  }
  static bool isAuto = false;
  static QString borderSize;
  if (!_cachedAutoValid) {
    KConfigGroup kdecorationGroup(
        _config->group(QStringLiteral("org.kde.kdecoration3")));
    isAuto = kdecorationGroup.readEntry("BorderSizeAuto", true);
    borderSize = kdecorationGroup.readEntry("BorderSize", "Normal");
    _cachedAutoValid = true;
  }
  if (isAuto) {
    auto window = widget->window();
    if (qobject_cast<const QDialog *>(widget)) {
      return true;
    }
    if (window) {
      auto handle = window->windowHandle();
      if (handle) {
        auto toolbar = qobject_cast<const QToolBar *>(widget);
        if (toolbar) {
          if (toolbar->isFloating()) {
            return false;
          }
        }
        return true;
      }
    } else {
      return false;
    }
  }
  if (borderSize != "None" && borderSize != "NoSides") {
    return false;
  }
  return true;
}

//______________________________________________________________________________________
QColor Helper::transparentBarBgColor(QColor bgColor, QPainter *painter,
                                     const QRect &rect, BarType barType) const {
  switch (barType) {
  case BarType::MenuBar: {
    if (StyleConfigData::menuBarOpacity() == 100) {
      // opacity is at 100%
      bgColor.setAlphaF(1.0);
    } else if (StyleConfigData::menuBarOpacity() == 0) {
      // fully transparent
      bgColor.setAlphaF(0.0);
      renderTransparentArea(painter, rect);
    } else if (StyleConfigData::menuBarOpacity() < 100 &&
               StyleConfigData::menuBarOpacity() > 0) {
      // lower the opacity
      bgColor.setAlphaF(StyleConfigData::menuBarOpacity() / 100.0);
      renderTransparentArea(painter, rect);
    }
    return bgColor;
  }
  case BarType::ToolBar: {
    if (StyleConfigData::toolBarOpacity() == 100) {
      // opacity is at 100%
      bgColor.setAlphaF(1.0);
    } else if (StyleConfigData::toolBarOpacity() == 0) {
      // fully transparent
      bgColor.setAlphaF(0.0);
      renderTransparentArea(painter, rect);
    } else if (StyleConfigData::toolBarOpacity() < 100 &&
               StyleConfigData::toolBarOpacity() > 0) {
      // lower the opacity
      bgColor.setAlphaF(StyleConfigData::toolBarOpacity() / 100.0);
      renderTransparentArea(painter, rect);
    }
    return bgColor;
  }
  case BarType::TabBar: {
    if (StyleConfigData::tabBarOpacity() == 100) {
      // opacity is at 100%
      bgColor.setAlphaF(1.0);
    } else if (StyleConfigData::tabBarOpacity() == 0) {
      // fully transparent
      bgColor.setAlphaF(0.0);
      renderTransparentArea(painter, rect);
    } else if (StyleConfigData::tabBarOpacity() < 100 &&
               StyleConfigData::tabBarOpacity() > 0) {
      // lower the opacity
      bgColor.setAlphaF(StyleConfigData::tabBarOpacity() / 100.0);
      renderTransparentArea(painter, rect);
    }
    return bgColor;
  }
  default:
    return bgColor;
  }
}
} // namespace BlossomUI
