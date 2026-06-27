/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * Copyright 2018  Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "blossomuidecoration.h"

#include "blossomui.h"
#include "blossomuisettingsprovider.h"
#include "config-blossomui.h"

#include <KDecoration3/DecorationShadow>

#include <KColorUtils>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QVariantAnimation>

#if BLOSSOMUI_HAVE_X11
#include <QX11Info>
#endif

namespace BlossomUI
{

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

// Defined in decoration/shadow.cpp
extern int g_sDecoCount;
extern std::shared_ptr<KDecoration3::DecorationShadow> g_sShadow;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
    , m_animation(new QVariantAnimation(this))
{
    g_sDecoCount++;
}

Decoration::~Decoration()
{
    g_sDecoCount--;
    if (g_sDecoCount == 0) {
        g_sShadow.reset();
    }
}

void Decoration::setOpacity(qreal value)
{
    if (m_opacity == value)
        return;
    m_opacity = value;
    update();
}

QColor Decoration::titleBarColor() const
{
    auto c = window();
    if (hideTitleBar())
        return c->color(ColorGroup::Inactive, ColorRole::TitleBar);
    else if (m_animation->state() == QAbstractAnimation::Running)
        return KColorUtils::mix(c->color(ColorGroup::Inactive, ColorRole::TitleBar), c->color(ColorGroup::Active, ColorRole::TitleBar), m_opacity);
    else
        return c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::TitleBar);
}

QColor Decoration::outlineColor() const
{
    auto c(window());
    if (!m_internalSettings->drawTitleBarSeparator())
        return QColor();
    if (m_animation->state() == QAbstractAnimation::Running) {
        QColor color(c->palette().color(QPalette::Highlight));
        color.setAlpha(color.alpha() * m_opacity);
        return color;
    } else if (c->isActive())
        return c->palette().color(QPalette::Highlight);
    else
        return QColor();
}

QColor Decoration::fontColor() const
{
    auto c = window();
    return c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Foreground);
}

bool Decoration::init()
{
    auto c = window();

    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    reconfigure();
    updateTitleBar();
    updateBlur();
    auto s = settings();
    connect(s.get(), &KDecoration3::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);

    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

    connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);

    connect(c, &KDecoration3::DecoratedWindow::captionChanged, this, [this]() {
        update(titleBar());
    });

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateAnimationState);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateTitleBar);
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateTitleBar);

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateBlur);
    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateTitleBar);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateTitleBar);

    connect(c, &KDecoration3::DecoratedWindow::sizeChanged, this, &Decoration::updateBlur);
    connect(c, &KDecoration3::DecoratedWindow::scaleChanged, this, &Decoration::updateScale);

    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);

    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateButtonsGeometryDelayed);

    connect(c, &KDecoration3::DecoratedWindow::paletteChanged, this, [this]() {
        recalculateBorders();
        updateTitleBar();
        updateBlur();
        update();
    });

    createButtons();
    createShadow();
    return true;
}

void Decoration::updateAnimationState()
{
    if (m_internalSettings->animationsEnabled()) {
        auto c = window();
        m_animation->setDirection(c->isActive() ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        if (m_animation->state() != QAbstractAnimation::Running)
            m_animation->start();
    } else {
        update();
    }
}

int Decoration::animationDuration() const
{
    KConfigGroup kdeConfig(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("KDE"));
    const qreal factor = kdeConfig.readEntry(QStringLiteral("AnimationDurationFactor"), 1.0);
    if (factor == 0.0)
        return 0;
    return qRound(m_internalSettings->animationsDuration() * factor);
}

void Decoration::reconfigure()
{
    SettingsProvider::self()->reconfigure();
    KSharedConfig::openConfig(QStringLiteral("kdeglobals"))->reparseConfiguration();

    m_internalSettings = SettingsProvider::self()->internalSettings(this);

    setScaledCornerRadius();

    m_animation->setDuration(animationDuration());

    recalculateBorders();
    createShadow();
    updateBlur();
}

} // namespace BlossomUI
