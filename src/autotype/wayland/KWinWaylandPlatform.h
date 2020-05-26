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

#ifndef KEEPASSXC_KWINWAYLANDPLATFORM_H
#define KEEPASSXC_KWINWAYLANDPLATFORM_H

#include "AutoTypeWayland_p.h"

#include <QScopedPointer>

#include <xkbcommon/xkbcommon.h>

namespace KWayland {
    namespace Client {
        class ConnectionThread;
        class Registry;
        class FakeInput;
        class PlasmaWindowManagement;
        class Keyboard;
    }
}
class QAction;
class KeyboardState;
class KWinWaylandPlatform : public WaylandPlatformBase
{
    Q_OBJECT
public:
    KWinWaylandPlatform(QObject *parent = nullptr);
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

    void sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void sendChar(QChar ch);

private:
    void sendKey(xkb_keysym_t keysym, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    void sendKeyEvent(xkb_keycode_t keycode, bool pressed);
    void sendModifierEvent(Qt::KeyboardModifiers modifiers, bool pressed);

private:
    KeyboardState *m_kbstate{nullptr};

    KWayland::Client::ConnectionThread* m_connection{nullptr};
    KWayland::Client::Registry* m_registry{nullptr};
    KWayland::Client::FakeInput* m_fakeInput{nullptr};
    KWayland::Client::Keyboard* m_keyboard{nullptr};
    KWayland::Client::PlasmaWindowManagement* m_plasmaWindowManagement{nullptr};

    QAction* m_action{nullptr};
};

class AutoTypeExecutorKWinWayland : public AutoTypeExecutor
{
public:
    explicit AutoTypeExecutorKWinWayland(KWinWaylandPlatform& platform);

    void execChar(AutoTypeChar* action) override;
    void execKey(AutoTypeKey* action) override;
    void execClearField(AutoTypeClearField* action) override;

private:
    KWinWaylandPlatform& m_platform;
};

#endif // KEEPASSXC_KWINWAYLANDPLATFORM_H
