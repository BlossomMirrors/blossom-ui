#ifndef blossomui_core_render_widgettraits_h
#define blossomui_core_render_widgettraits_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include <QBrush>
#include <QColor>
#include <QMarginsF>
#include <QPainter>
#include <QPointF>
#include <QSizeF>

namespace BlossomUI {
namespace Render {

enum class Easing { Linear, OutCubic, OutBack };

qreal applyEasing(Easing, qreal t);

struct Fill {
  Fill() = default;
  Fill(const QColor &color) : brush(color) {}
  Fill(const QBrush &brush) : brush(brush) {}

  bool isValid() const { return brush.style() != Qt::NoBrush; }

  QBrush brush;
  qreal opacity = 1.0;
  QPainter::CompositionMode compositeMode = QPainter::CompositionMode_SourceOver;
};

enum class BorderAlign { Inside, Centered, Outside };

struct Border {
  static Border none() { return Border(); }

  Border() = default;
  Border(const QColor &color, qreal width = 1.0,
         BorderAlign align = BorderAlign::Centered)
      : color(color), width(width), align(align) {}

  bool isValid() const { return width > 0.0 && color.isValid(); }

  QColor color;
  qreal width = 0.0;
  BorderAlign align = BorderAlign::Centered;
};

//* value-driven (not state-driven) split fill: progress bar contents,
//slider groove, switch trail
struct Progress {
  qreal fraction = 0.0;
  Fill filled;
  Fill unfilled;
};

//* in-widget drop shadow, not the native window shadow (ShadowHelper)
struct Shadow {
  QPointF offset;
  qreal blurRadius = 0.0;
  qreal spread = 0.0;
  QColor color;

  bool isValid() const { return color.isValid() && blurRadius > 0.0; }
};

//* which StyleConfigData corner radius a frame uses
enum class RadiusRole { Frame, Button, Input, Tab };

enum class Shape { RoundedRect, Ellipse, Arc };

struct Geometry {
  Geometry &frameInset(qreal v) {
    inset = v;
    return *this;
  }
  Geometry &radius(RadiusRole role) {
    radiusRole = role;
    return *this;
  }
  Geometry &fixedRadius(qreal v) {
    radiusOverride = v;
    return *this;
  }
  Geometry &minSize(qreal w, qreal h) {
    minimumSize = QSizeF(w, h);
    return *this;
  }
  Geometry &maxSize(qreal w, qreal h) {
    maximumSize = QSizeF(w, h);
    return *this;
  }
  Geometry &fixedSize(qreal w, qreal h) {
    minimumSize = maximumSize = QSizeF(w, h);
    return *this;
  }
  Geometry &contentPadding(qreal all) {
    padding = QMarginsF(all, all, all, all);
    return *this;
  }
  Geometry &contentPadding(qreal l, qreal t, qreal r, qreal b) {
    padding = QMarginsF(l, t, r, b);
    return *this;
  }
  Geometry &aspectRatio(qreal v) {
    aspect = v;
    return *this;
  }
  Geometry &shape(Shape s) {
    frameShape = s;
    return *this;
  }
  Geometry &arc(qreal startDegrees, qreal spanDegrees) {
    arcStart = startDegrees;
    arcSpan = spanDegrees;
    return *this;
  }

  qreal inset = 0.0;
  RadiusRole radiusRole = RadiusRole::Button;
  qreal radiusOverride = -1.0;
  QSizeF minimumSize;
  QSizeF maximumSize;
  QMarginsF padding;
  qreal aspect = -1.0;
  Shape frameShape = Shape::RoundedRect;
  qreal arcStart = 0.0;
  qreal arcSpan = 360.0;
};

qreal lerp(qreal a, qreal b, qreal t);
QColor lerp(const QColor &a, const QColor &b, qreal t);
Fill lerp(const Fill &a, const Fill &b, qreal t);
Border lerp(const Border &a, const Border &b, qreal t);
Shadow lerp(const Shadow &a, const Shadow &b, qreal t);

} // namespace Render
} // namespace BlossomUI

#endif
