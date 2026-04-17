#ifndef blossomuiexceptionlist_h
#define blossomuiexceptionlist_h

// SPDX-License-Identifier: MIT
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>

#pragma once
#include "blossomui.h"
#include "blossomuisettings.h"

#include <KSharedConfig>

namespace BlossomUI
{

class ExceptionList
{
public:
    explicit ExceptionList(const InternalSettingsList &exceptions = InternalSettingsList())
        : _exceptions(exceptions)
    {
    }

    const InternalSettingsList &get(void) const
    {
        return _exceptions;
    }

    void readConfig(KSharedConfig::Ptr);
    void writeConfig(KSharedConfig::Ptr);

protected:
    static QString exceptionGroupName(int index);
    static void readConfig(KCoreConfigSkeleton *, KConfig *, const QString &);
    static void writeConfig(KCoreConfigSkeleton *, KConfig *, const QString &);

private:
    InternalSettingsList _exceptions;
};

}

#endif
