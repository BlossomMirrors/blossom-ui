#ifndef blossomui_helper_h
#define blossomui_helper_h

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

#include "blossomui.h"
#include "blossomuianimationdata.h"
#include "blossomuistyleconfigdata.h"

#include "blossomuishadowhelper.h"

#include <KColorScheme>
#include <KSharedConfig>
#include <KStatefulBrush>
#include <QAbstractItemView>
#include <QFrame>
#include <QIcon>
#include <QMargins>
#include <QPainterPath>
#include <QRegion>
#include <QStyleOption>
#include <QToolBar>
#include <QWidget>

namespace BlossomUI {
class PaletteChangedEventFilter;

//* blossomui style helper class.
/** contains utility functions used at multiple places in both blossomui style
 * and blossomui window decoration */
class Helper : public QObject {
  Q_OBJECT

public:
  //* constructor
  explicit Helper(KSharedConfig::Ptr, QObject *parent = nullptr);

  //* destructor
  virtual ~Helper() {}

  //* load configuration
  virtual void loadConfig();

  //* pointer to shared config
  KSharedConfig::Ptr config() const;

  //*@name color utilities
  //@{

  //* add alpha channel multiplier to color
  QColor alphaColor(QColor color, qreal alpha) const;

  //* mouse over color
  QColor hoverColor(const QPalette &palette) const {
    return _viewHoverBrush.brush(palette).color();
  }

  //* focus color
  QColor focusColor(const QPalette &palette) const {
    return _viewFocusBrush.brush(palette).color();
  }

  //* negative text color (used for close button)
  QColor negativeText(const QPalette &palette) const {
    return _viewNegativeTextBrush.brush(palette).color();
  }

  //* alternate window background
  QColor windowAlternateBackground(const QPalette &palette) const {
    return _windowAlternateBackgroundBrush.brush(palette).color();
  }

  //* alternate button background (used for hover)
  QColor buttonAlternateBackground(const QPalette &palette) const {
    return _buttonAlternateBackgroundBrush.brush(palette).color();
  }

  //* shadow
  QColor shadowColor(const QPalette &palette) const {
    return alphaColor(palette.color(QPalette::Shadow), 0.15);
  }

  //* titlebar color
  const QColor &titleBarColor(bool active) const {
    return active ? _activeTitleBarColor : _inactiveTitleBarColor;
  }

  //* titlebar text color
  const QColor &titleBarTextColor(bool active) const {
    return active ? _activeTitleBarTextColor : _inactiveTitleBarTextColor;
  }

  //* frame outline color, using animations
  QColor frameOutlineColor(const QPalette &, bool mouseOver = false,
                           bool hasFocus = false,
                           qreal opacity = AnimationData::OpacityInvalid,
                           AnimationMode = AnimationNone) const;

  //* side panel outline color, using animations
  QColor sidePanelOutlineColor(const QPalette &) const;

  //* frame background color
  QColor frameBackgroundColor(const QPalette &palette) const {
    return frameBackgroundColor(palette, palette.currentColorGroup());
  }

  //* frame background color
  QColor frameBackgroundColor(const QPalette &, QPalette::ColorGroup) const;

  //* arrow outline color
  QColor arrowColor(const QPalette &, QPalette::ColorGroup,
                    QPalette::ColorRole) const;

  //* arrow outline color
  QColor arrowColor(const QPalette &palette, QPalette::ColorRole role) const {
    return arrowColor(palette, palette.currentColorGroup(), role);
  }

  //* arrow outline color, using animations
  QColor arrowColor(const QPalette &, bool mouseOver, bool hasFocus,
                    qreal opacity = AnimationData::OpacityInvalid,
                    AnimationMode = AnimationNone) const;

  //* separator color
  QColor separatorColor(const QPalette &) const;

  //* merge active and inactive palettes based on ratio, for smooth enable state
  //change transition
  QPalette disabledPalette(const QPalette &, qreal ratio) const;

  //@}

  //*@name rendering utilities
  //@{

  //* focus line
  void renderFocusLine(QPainter *, const QRect &, const QColor &) const;

  //* generic frame
  void renderFrame(QPainter *, const QRect &, const QColor &color,
                   const bool windowActive, const bool enabled = true) const;

  //* side panel frame
  void renderSidePanelFrame(QPainter *, const QRect &, const QColor &outline,
                            Side) const;

  //* menu frame
  void renderMenuFrame(QPainter *, const QRect &, const QColor &color,
                       const QColor &outline, bool roundCorners = true) const;

  //* blurred background for in-window popups (e.g. Slint combobox) - grabs
  //window content, blurs, draws
  bool renderBlurredBackground(QPainter *painter, QWidget *window,
                               const QRect &sourceRectInWindow,
                               const QRect &targetRect,
                               int blurRadius = 12) const;

  //* shadow for widgets
  void renderBoxShadow(QPainter *, const QRect &, const int xOffset,
                       const int yOffset, const int size, const QColor &color,
                       const int cornerRadius, const bool active,
                       TileSet::Tiles = TileSet::Ring) const;

  void renderBoxShadow(QPainter *painter, const QRectF &rect, const int xOffset,
                       const int yOffset, const int size, const QColor &color,
                       const int cornerRadius, const bool active,
                       TileSet::Tiles tiles = TileSet::Ring) const {
    QRect copy = QRect(rect.x(), rect.y(), rect.width(), rect.height());
    renderBoxShadow(painter, copy, xOffset, yOffset, size, color, cornerRadius,
                    active, tiles);
  }

  //* toolbutton frame
  void renderToolBoxFrame(QPainter *, const QRect &, int tabWidth,
                          const QColor &color) const;

  //* tab widget frame
  void renderTabWidgetFrame(QPainter *, const QRect &, const QColor &color,
                            Corners corners, const bool windowActive) const;

  //* item view item margins
  QMargins itemViewItemMargins(const QStyleOptionViewItem *) const;

  //* selection frame
  void renderSelection(QPainter *, const QRect &, const QColor &,
                       Corners) const;
  //* selection frame with custom radius (e.g. for Dolphin list rows)
  void renderSelection(QPainter *, const QRect &, const QColor &, Corners,
                       qreal radius) const;

  //* separator
  void renderSeparator(QPainter *, const QRect &, const QColor &,
                       bool vertical = false) const;

  //* line edit
  void renderLineEdit(QPainter *, const QRect &, const QColor &background,
                      const QColor &outline, const bool hasFocus,
                      const bool mouseOver, const bool enabled,
                      const bool windowActive, const AnimationMode mode,
                      const qreal opacity) const;

  //* group box
  void renderGroupBox(QPainter *, const QRect &, const QColor &color,
                      const bool mouseOver) const;

  //* checkbox
  void renderCheckBox(QPainter *, const QRect &, const QPalette &,
                      const bool isInMenu, bool sunken, const bool mouseOver,
                      CheckBoxState state, const bool windowActive,
                      qreal animation = AnimationData::OpacityInvalid) const;

  //* switch (pill-style toggle with accent when on, for QAccessible::Switch /
  //Slint Switch)
  void renderSwitch(QPainter *, const QRect &, const QPalette &, bool sunken,
                    const bool mouseOver, CheckBoxState state,
                    qreal animation = AnimationData::OpacityInvalid) const;

  //* radio button
  void renderRadioButton(QPainter *, const QRect &, const QPalette &,
                         const bool mouseOver, bool sunken,
                         RadioButtonState state, const bool isInMenu,
                         qreal animation = AnimationData::OpacityInvalid) const;

  //* slider groove
  void renderSliderGroove(QPainter *, const QRect &, const QColor &) const;

  //* slider handle
  void renderSliderHandle(QPainter *, const QRect &, const QColor &,
                          const QColor &, qreal hoverOpacity, bool sunken) const;

  //* dial groove
  void renderDialGroove(QPainter *, const QRect &, const QColor &, qreal first,
                        qreal last) const;

  //* dial groove
  void renderDialContents(QPainter *, const QRect &, const QColor &,
                          qreal first, qreal second) const;

  //* progress bar groove
  void renderProgressBarGroove(QPainter *, const QRect &, const QColor &) const;

  //* progress bar contents
  void renderProgressBarContents(QPainter *painter, const QRect &rect,
                                 const QColor &color) const {
    return renderProgressBarGroove(painter, rect, color);
  }

  //* progress bar contents (animated)
  void renderProgressBarBusyContents(QPainter *painter, const QRect &rect,
                                     const QColor &first, const QColor &second,
                                     bool horizontal, bool reverse,
                                     int progress) const;

  //* scrollbar groove
  void renderScrollBarGroove(QPainter *painter, const QRect &rect,
                             const QColor &color) const {
    return renderScrollBarHandle(painter, rect, color);
  }

  //* scrollbar handle
  void renderScrollBarHandle(QPainter *, const QRectF &, const QColor &) const;

  //* toolbar handle
  void renderToolBarHandle(QPainter *painter, const QRect &rect,
                           const QColor &color) const {
    return renderProgressBarGroove(painter, rect, color);
  }

  //* separator between scrollbar and contents
  void renderScrollBarBorder(QPainter *, const QRect &, const QColor &) const;

  //* tabbar tab (radius < 0 = use frameRadius; radius >= 0 = use for pill shape
  //with parallel radii)
  void renderTabBarTab(QPainter *, const QRect &, const QColor &color, Corners,
                       qreal radius = -1) const;

  //* tabbar tab outline (radius < 0 = use frameRadius, must match fill for
  //parallel border)
  void renderTabBarTabOutline(QPainter *, const QRect &,
                              const QColor &outlineColor, Corners,
                              qreal radius = -1) const;

  //* generic arrow
  void renderArrow(QPainter *, const QRect &, const QColor &,
                   ArrowOrientation) const;

  //* generic button (for mdi decorations, tabs and dock widgets)
  void renderDecorationButton(QPainter *, const QRect &, const QColor &,
                              ButtonType, bool inverted) const;

  //* erase the alpha
  void renderTransparentArea(QPainter *, const QRect &) const;

  //@}

  //*@name compositing utilities
  //@{

  //* true if style was compiled for and is running on X11
  static bool isX11();

  //* true if running on platform Wayland
  static bool isWayland();

  //* returns true if compositing is active
  bool compositingActive() const;

  //* returns true if a given widget supports alpha channel
  bool hasAlphaChannel(const QWidget *) const;

  //* returns true if the window should have an alpha channel
  bool shouldWindowHaveAlpha(const QPalette &, bool isDolphin) const;

  //* returns true if a given palette has dark colors
  bool isDarkTheme(const QPalette &palette) const {
    return qGray(palette.color(QPalette::Window).rgb()) > 110 ? false : true;
  }

  //* returns true if the tools area should be drawn
  bool shouldDrawToolsArea(const QWidget *) const;

  //@}

  //* frame radius
  qreal frameRadius(const qreal penWidth = PenWidth::NoPen,
                    const qreal bias = 0) const {
    return qMax(StyleConfigData::cornerRadius() - (0.5 * penWidth) + bias, 0.0);
  }

  //* button frame radius
  qreal buttonFrameRadius(const qreal penWidth = PenWidth::NoPen,
                          const qreal bias = 0) const {
    return qMax(StyleConfigData::buttonRadius() - (0.5 * penWidth) + bias, 0.0);
  }

  //* input field frame radius
  qreal inputFrameRadius(const qreal penWidth = PenWidth::NoPen,
                         const qreal bias = 0) const {
    return qMax(StyleConfigData::inputRadius() - (0.5 * penWidth) + bias, 0.0);
  }

  //* tab bar tab frame radius
  qreal tabFrameRadius(const qreal penWidth = PenWidth::NoPen) const {
    return qMax(qMax(StyleConfigData::cornerRadius() - 4, 2) - (0.5 * penWidth),
                0.0);
  }

  //* frame radius with new pen width
  constexpr qreal frameRadiusForNewPenWidth(const qreal oldRadius,
                                            const qreal penWidth) const {
    return qMax(oldRadius - (0.5 * penWidth), 0.0);
  }

  //* return a QRectF with the appropriate size for a rectangle with a pen
  //stroke
  QRectF strokedRect(const QRectF &rect,
                     const qreal penWidth = PenWidth::Frame) const;

  //* return a QRectF with the appropriate size for a rectangle with a shadow
  //around it
  QRectF shadowedRect(const QRectF &rect,
                      const qreal shadowSize = PenWidth::Shadow) const {
    return rect.adjusted(shadowSize, shadowSize, -shadowSize, -shadowSize);
  }

  QPixmap coloredIcon(const QIcon &icon, const QPalette &palette,
                      const QSize &size, qreal devicePixelRatio,
                      QIcon::Mode mode = QIcon::Normal,
                      QIcon::State state = QIcon::Off);

  // MenuBar, ToolBar, TabBar background color opacity
  QColor transparentBarBgColor(QColor, QPainter *, const QRect &,
                               BarType) const;

  //* Bottom corners only (for file list card - top stays straight under
  //header).
  static QRegion roundedRectRegionBottomCorners(int w, int h, int radius,
                                                qreal devicePixelRatio = 1.0);

protected:
  //* return rounded path in a given rect, with only selected corners rounded,
  //and for a given radius
  QPainterPath roundedPath(const QRectF &, Corners, qreal) const;

private:
  //* configuration
  KSharedConfig::Ptr _config;

  //*@name brushes
  //@{
  KStatefulBrush _viewFocusBrush;
  KStatefulBrush _viewHoverBrush;
  KStatefulBrush _viewNegativeTextBrush;
  KStatefulBrush _windowAlternateBackgroundBrush;
  KStatefulBrush _buttonAlternateBackgroundBrush;
  //@}

  //* event filter
  PaletteChangedEventFilter *_eventFilter;

  //*@name windeco colors
  //@{
  QColor _activeTitleBarColor;
  QColor _activeTitleBarTextColor;
  QColor _inactiveTitleBarColor;
  QColor _inactiveTitleBarTextColor;
  //@}

  mutable bool _cachedAutoValid = false;
  friend class ToolsAreaManager;
  friend class PaletteChangedEventFilter;
};

class PaletteChangedEventFilter : public QObject {
  Q_OBJECT

public:
  explicit PaletteChangedEventFilter(Helper *);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  Helper *_helper;
};

} // namespace BlossomUI

#endif
