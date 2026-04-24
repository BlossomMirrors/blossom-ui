// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuidecoration.h"
#include "button/blossomuibutton.h"
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(BlossomUIDecoFactory, "blossomui.json",
                           registerPlugin<BlossomUI::Decoration>();
                           registerPlugin<BlossomUI::Button>();)

#include "blossomuidecoration.moc"
