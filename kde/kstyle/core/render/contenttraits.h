#ifndef blossomui_core_render_contenttraits_h
#define blossomui_core_render_contenttraits_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "statestyle.h"

#include <QPalette>
#include <QSize>

namespace BlossomUI {
namespace Render {

struct TextStyle {
  QPalette::ColorRole colorRole = QPalette::WindowText;
  bool bold = false;
  bool italic = false;
  qreal letterSpacing = 0.0;
};

//* hand-drawn vector mark, e.g. Helper::renderArrow; Icon below is raster
enum class GlyphKind { Arrow, Checkmark, RadioDot, Cross };

struct Glyph {
  GlyphKind kind = GlyphKind::Arrow;
  StateStyle<Fill> color;
  qreal strokeWidth = 1.5;
};

//* tinted QIcon/pixmap (Helper::coloredIcon)
struct Icon {
  StateStyle<Fill> tint;
  QSize size;
  qreal iconTextSpacing = 0.0;
};

} // namespace Render
} // namespace BlossomUI

#endif
