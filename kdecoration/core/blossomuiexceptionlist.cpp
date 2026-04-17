// SPDX-License-Identifier: MIT
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>

#include "blossomuiexceptionlist.h"

namespace BlossomUI
{

void ExceptionList::readConfig(KSharedConfig::Ptr config)
{
    _exceptions.clear();

    QString groupName;
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
        InternalSettings exception;
        readConfig(&exception, config.data(), groupName);

        InternalSettingsPtr configuration(new InternalSettings());
        configuration.data()->load();

        configuration->setEnabled(exception.enabled());
        configuration->setExceptionType(exception.exceptionType());
        configuration->setExceptionPattern(exception.exceptionPattern());
        configuration->setMask(exception.mask());

        if (exception.mask() & BorderSize)
            configuration->setBorderSize(exception.borderSize());
        configuration->setHideTitleBar(exception.hideTitleBar());

        _exceptions.append(configuration);
    }
}

void ExceptionList::writeConfig(KSharedConfig::Ptr config)
{
    QString groupName;
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
        config->deleteGroup(groupName);
    }

    int index = 0;
    for (const InternalSettingsPtr &exception : std::as_const(_exceptions)) {
        writeConfig(exception.data(), config.data(), exceptionGroupName(index));
        ++index;
    }
}

QString ExceptionList::exceptionGroupName(int index)
{
    return QString("Windeco Exception %1").arg(index);
}

void ExceptionList::writeConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName)
{
    const QStringList keys = {"Enabled", "ExceptionPattern", "ExceptionType", "HideTitleBar", "Mask", "BorderSize"};

    for (const auto &key : keys) {
        KConfigSkeletonItem *item(skeleton->findItem(key));
        if (!item)
            continue;
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        KConfigGroup configGroup(config, item->group());
        configGroup.writeEntry(item->key(), item->property());
    }
}

void ExceptionList::readConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName)
{
    for (KConfigSkeletonItem *item : skeleton->items()) {
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        item->readConfig(config);
    }
}

} // namespace BlossomUI
