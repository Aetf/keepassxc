/*
 *  Copyright (C) 2020 Aetf <aetf@unlimitedcodeworks.xyz>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEEPASSXC_AUTOTYPEWAYLAND_H
#define KEEPASSXC_AUTOTYPEWAYLAND_H

#include "autotype/AutoTypeAction.h"
#include "autotype/AutoTypePlatformPlugin.h"

#include <QtPlugin>
#include <QWindow>

class WaylandPlatformBase;
class AutoTypePlatformWayland : public QObject, public AutoTypePlatformInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.keepassx.AutoTypePlatformWayland")
    Q_INTERFACES(AutoTypePlatformInterface)

public:
    AutoTypePlatformWayland(QObject *parent = nullptr);
    bool isAvailable() override;
    void unload() override;
    QStringList windowTitles() override;
    WId activeWindow() override;
    QString activeWindowTitle() override;
    bool registerGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers) override;
    void unregisterGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers) override;
    int platformEventFilter(void* event) override;
    bool raiseWindow(WId window) override;
    AutoTypeExecutor* createExecutor() override;

signals:
    void globalShortcutTriggered();

private:
    QScopedPointer<WaylandPlatformBase> m_impl;
};

#endif // KEEPASSXC_AUTOTYPEWAYLAND_H
