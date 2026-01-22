#ifndef BLOSSOMUI_BUTTONS_H
#define BLOSSOMUI_BUTTONS_H

/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
#pragma once
#include "blossomuidecoration.h"
#include <KDecoration3/DecorationButton>

#include <QHash>
#include <QImage>

class QVariantAnimation;

namespace BlossomUI
{

class Button : public KDecoration3::DecorationButton
{
    Q_OBJECT

public:
    explicit Button(QObject *parent, const QVariantList &args);
    virtual ~Button() = default;

    static Button *create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent);

    virtual void paint(QPainter *painter, const QRectF &repaintRegion) override;

    enum Flag {
        FlagNone,
        FlagStandalone,
        FlagFirstInList,
        FlagLastInList
    };

    void setFlag(Flag value)
    {
        m_flag = value;
    }

    bool isStandAlone() const
    {
        return m_flag == FlagStandalone;
    }

    void setOffset(const QPointF &value)
    {
        m_offset = value;
    }

    void setHorizontalOffset(qreal value)
    {
        m_offset.setX(value);
    }

    void setVerticalOffset(qreal value)
    {
        m_offset.setY(value);
    }

    void setIconSize(const QSizeF &size)
    {
        m_iconSize = size;
    }

    void setOpacity(qreal value)
    {
        if (m_opacity == value)
            return;
        m_opacity = value;
        update();
    }

    qreal opacity() const
    {
        return m_opacity;
    }

    void setPadding(const QMargins &value)
    {
        m_padding = value;
    }

    void setLeftPadding(qreal value)
    {
        m_padding.setLeft(value);
    }

    void setRightPadding(qreal value)
    {
        m_padding.setRight(value);
    }

    void setPreferredSize(const QSizeF &size)
    {
        m_preferredSize = size;
    }

    QSizeF preferredSize() const
    {
        return m_preferredSize;
    }

private Q_SLOTS:
    void reconfigure();
    void updateAnimationState(bool);

private:
    explicit Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    void drawIcon(QPainter *) const;
    void drawIconWithMask(QPainter *) const;

    QColor foregroundColor() const;
    QColor backgroundColor() const;

    Flag m_flag = FlagNone;
    QVariantAnimation *m_animation;
    QPointF m_offset;
    qreal m_opacity = 0;
    QMargins m_padding;
    QSizeF m_iconSize;
    QSizeF m_preferredSize;
};

} // namespace

#endif
