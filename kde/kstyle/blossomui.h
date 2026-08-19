#ifndef blossomui_h
#define blossomui_h

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

#include <QFlags>
#include <QPointer>
#include <QScopedPointer>
#include <QWeakPointer>

namespace BlossomUI {

template <typename T> using WeakPointer = QPointer<T>;

template <typename T>
using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

// Pen stroke widths.
namespace PenWidth {
// Slightly above 1.0 so symbol strokes don't look skewed at non-integer DPR.
static constexpr qreal Symbol = 1.01;
static constexpr qreal Frame = 1.0;
static constexpr qreal Shadow = 1.0;
static constexpr qreal NoPen = 0.0;
} // namespace PenWidth

// Opacity/alpha values used for mixing and tinting throughout the style.
// Keeps magic numbers out of render code and makes the design system explicit.
namespace Alpha {
// outline / border
static constexpr qreal OutlineBase = 0.12;  // default border alpha
static constexpr qreal OutlineHover = 0.16; // border on hover
static constexpr qreal OutlineEmph =
    0.22; // emphasized border (e.g. focus ring)
static constexpr qreal OutlineFocusMix = 0.15; // focus color mix into outline

// button
static constexpr qreal ButtonSunken = 0.20;    // pressed background mix
static constexpr qreal ButtonTint = 0.28;      // tinted background mix
static constexpr qreal ButtonTintHover = 0.38; // tinted + hover

// checkbox / radio
static constexpr qreal CheckboxBorder = 0.10;
static constexpr qreal CheckboxBorderHover = 0.16;

// switch
static constexpr qreal SwitchBorder = 0.10;
static constexpr qreal SwitchBorderHover = 0.16;
static constexpr qreal SwitchThumbBorder = 0.12;

// radio button
static constexpr qreal RadioBorder = 0.14;
static constexpr qreal RadioBorderHover = 0.20;

// scrollbar
static constexpr qreal ScrollbarHandle = 0.5;

// hover/resting tabs and item views
static constexpr qreal ItemViewResting = 0.10;
static constexpr qreal ItemViewHover = 0.20;

// generic half-dimension used in radius/geometry calculations
static constexpr qreal Half = 0.5;

} // namespace Alpha

// Animation mode flags.
enum AnimationMode {
  AnimationNone = 0,
  AnimationHover = 0x1,
  AnimationFocus = 0x2,
  AnimationEnable = 0x4,
  AnimationPressed = 0x8
};

Q_DECLARE_FLAGS(AnimationModes, AnimationMode)

// Extra animation parameters for custom widget animations.
enum AnimationParameter {
  AnimationDefault = 0,
  AnimationForwardOnly = 0x1,
  AnimationOutBack = 0x2,
  AnimationLongDuration = 0x4
};

Q_DECLARE_FLAGS(AnimationParameters, AnimationParameter)

enum Corner {
  CornerTopLeft = 0x1,
  CornerTopRight = 0x2,
  CornerBottomLeft = 0x4,
  CornerBottomRight = 0x8,
  CornersTop = CornerTopLeft | CornerTopRight,
  CornersBottom = CornerBottomLeft | CornerBottomRight,
  CornersLeft = CornerTopLeft | CornerBottomLeft,
  CornersRight = CornerTopRight | CornerBottomRight,
  AllCorners =
      CornerTopLeft | CornerTopRight | CornerBottomLeft | CornerBottomRight
};

Q_DECLARE_FLAGS(Corners, Corner)

enum Side {
  SideLeft = 0x1,
  SideTop = 0x2,
  SideRight = 0x4,
  SideBottom = 0x8,
  AllSides = SideLeft | SideTop | SideRight | SideBottom
};

Q_DECLARE_FLAGS(Sides, Side)

enum CheckBoxState { CheckOff, CheckPartial, CheckOn, CheckAnimated };

enum RadioButtonState { RadioOff, RadioOn, RadioAnimated };

enum ArrowOrientation {
  ArrowNone,
  ArrowUp,
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  ArrowDownSmall,
};

enum ButtonType { ButtonClose, ButtonMaximize, ButtonMinimize, ButtonRestore };

// Which bar type is being queried for opacity/blur settings.
enum BarType { MenuBar, ToolBar, TabBar };

} // namespace BlossomUI

Q_DECLARE_OPERATORS_FOR_FLAGS(BlossomUI::AnimationModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(BlossomUI::AnimationParameters)
Q_DECLARE_OPERATORS_FOR_FLAGS(BlossomUI::Corners)
Q_DECLARE_OPERATORS_FOR_FLAGS(BlossomUI::Sides)

#endif
