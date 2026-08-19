#ifndef blossomui_core_render_radiusresolver_h
#define blossomui_core_render_radiusresolver_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

namespace BlossomUI {
class Helper;

namespace Render {

class RadiusResolver {
public:
  explicit RadiusResolver(const Helper *helper) : _helper(helper) {}

  qreal resolve(const Geometry &geometry) const;

private:
  const Helper *_helper;
};

} // namespace Render
} // namespace BlossomUI

#endif
