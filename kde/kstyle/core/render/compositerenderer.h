#ifndef blossomui_core_render_compositerenderer_h
#define blossomui_core_render_compositerenderer_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "contentrenderer.h"
#include "contenttraits.h"
#include "placement.h"
#include "widgetrenderer.h"

#include <QVector>

namespace BlossomUI {
namespace Render {

struct Part {
  enum class Kind { Shape, Content, Glyph, Custom };

  Kind kind = Kind::Shape;
  WidgetSpec shape;
  ContentLayout contentLayout;
  Content content;
  Glyph glyph;
  CustomPart custom;
  Placement placement;
  //* scales the part's painting (not just its bounds) around its rect's
  //center, so centered content actually shrinks in sync with an animated
  //frame instead of an inset that a centered layout would cancel out
  Motion motion;

  static Part fromShape(const WidgetSpec &s, const Placement &p) {
    Part part;
    part.kind = Kind::Shape;
    part.shape = s;
    part.placement = p;
    return part;
  }
  static Part fromContent(const ContentLayout &l, const Content &c,
                          const Placement &p, const Motion &m = Motion()) {
    Part part;
    part.kind = Kind::Content;
    part.contentLayout = l;
    part.content = c;
    part.placement = p;
    part.motion = m;
    return part;
  }
  static Part fromCustom(const CustomPart &c, const Placement &p) {
    Part part;
    part.kind = Kind::Custom;
    part.custom = c;
    part.placement = p;
    return part;
  }
};

class CompositeRenderer {
public:
  explicit CompositeRenderer(Helper *helper) : _shapes(helper), _content(helper) {}

  void render(QPainter *painter, const QRect &rect, const QVector<Part> &parts,
              const WidgetInteractionState &state) const;

private:
  WidgetRenderer _shapes;
  ContentRenderer _content;
};

} // namespace Render
} // namespace BlossomUI

#endif
