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

#ifndef KEEPASSXC_AUTOTYPEWAYLAND_P_H
#define KEEPASSXC_AUTOTYPEWAYLAND_P_H

#include "autotype/AutoTypePlatformPlugin.h"

#include <QObject>

class WaylandPlatformBase : public QObject, public AutoTypePlatformInterface {
    Q_OBJECT
    Q_INTERFACES(AutoTypePlatformInterface)
public:
    using QObject::QObject;

signals:
    void globalShortcutTriggered();
};

#endif // KEEPASSXC_AUTOTYPEWAYLAND_P_H
