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
