/*
    SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIRIGAMIPLASMADESKTOPUNITS_H
#define KIRIGAMIPLASMADESKTOPUNITS_H

#include <QObject>
#include <QPropertyNotifier>

#include <Kirigami/Platform/Units>

class AnimationSpeedProvider;

class PlasmaDesktopUnits : public Kirigami::Platform::Units
{
    Q_OBJECT

public:
    explicit PlasmaDesktopUnits(QObject *parent = nullptr);

    void updateAnimationSpeed();
    void updateCornerRadius();

private:
    std::unique_ptr<AnimationSpeedProvider> m_animationSpeedProvider;
    QPropertyNotifier m_notifier;
};

#endif
