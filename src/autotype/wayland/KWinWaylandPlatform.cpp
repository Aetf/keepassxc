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

#include "KWinWaylandPlatform.h"
#include "KeyboardState.h"

#include "core/Global.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/fakeinput.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/keyboard.h>
#include <KGlobalAccel>

#include <QAction>
#include <QGuiApplication>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QKeySequence>

inline void delay(int millisecondsWait)
{
    QEventLoop loop;
    QTimer t;
    t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(millisecondsWait);
    loop.exec();
}

KWinWaylandPlatform::KWinWaylandPlatform(QObject* parent)
    : WaylandPlatformBase(parent)
    , m_kbstate(new KeyboardState(this))
    , m_connection(KWayland::Client::ConnectionThread::fromApplication(qApp))
    , m_registry(new KWayland::Client::Registry(this))
    , m_action(new QAction(this))
{
    connect(m_registry, &KWayland::Client::Registry::fakeInputAnnounced, this, [this] (quint32 name, quint32 version) {
        m_fakeInput = m_registry->createFakeInput(name, version, this);
        m_fakeInput->authenticate(qApp->applicationDisplayName(), tr("Auto-Type"));
    });
    connect(m_registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced, this, [this] (quint32 name, quint32 version) {
        m_plasmaWindowManagement = m_registry->createPlasmaWindowManagement(name, version, this);
    });
    connect(m_registry, &KWayland::Client::Registry::seatAnnounced, this, [this](quint32 name, quint32 version) {
        auto seat = m_registry->createSeat(name, version, this);
        m_keyboard = seat->createKeyboard(seat);
        connect(m_keyboard, &KWayland::Client::Keyboard::keymapChanged, m_kbstate, &KeyboardState::updateKeymap);
        connect(m_keyboard, &KWayland::Client::Keyboard::modifiersChanged, m_kbstate, &KeyboardState::updateMask);
    });

    m_registry->create(m_connection);
    m_registry->setup();
    // use a local event loop to wait for registry announcing all interfaces
    {
        QEventLoop loop;
        connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, &loop, &QEventLoop::quit);
        loop.exec();
    }

    m_action->setObjectName(tr("Activate Auto-Type"));
    connect(m_action, &QAction::triggered, this, &KWinWaylandPlatform::globalShortcutTriggered);
}

bool KWinWaylandPlatform::isAvailable()
{
    return m_fakeInput && m_plasmaWindowManagement && m_keyboard;
}

// objects will be release on deconstruction
void KWinWaylandPlatform::unload()
{
}

QStringList KWinWaylandPlatform::windowTitles()
{
    if (!m_plasmaWindowManagement) {
        return {};
    }
    auto windows = m_plasmaWindowManagement->windows();
    QStringList titles;
    titles.reserve(windows.size());
    for (const auto win : asConst(windows)) {
        titles << win->title();
    }
    return titles;
}

WId KWinWaylandPlatform::activeWindow()
{
    if (!m_plasmaWindowManagement) {
        return {};
    }
    // internalId is NOT WId in anyway. But since this is only used internally in AutoTypeKWinWayland,
    // we just keep this in mind and use it anyway.
    return m_plasmaWindowManagement->activeWindow()->internalId();
}

QString KWinWaylandPlatform::activeWindowTitle()
{
    if (!m_plasmaWindowManagement) {
        return {};
    }
    return m_plasmaWindowManagement->activeWindow()->title();
}

bool KWinWaylandPlatform::registerGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    return KGlobalAccel::setGlobalShortcut(m_action, QKeySequence(key + modifiers));
}

void KWinWaylandPlatform::unregisterGlobalShortcut(Qt::Key, Qt::KeyboardModifiers)
{
    KGlobalAccel::self()->removeAllShortcuts(m_action);
}

int KWinWaylandPlatform::platformEventFilter(void* event)
{
    Q_UNUSED(event);
    return -1;
}

bool KWinWaylandPlatform::raiseWindow(WId window)
{
    if (!m_plasmaWindowManagement) {
        return false;
    }

    for (const auto win : m_plasmaWindowManagement->windows()) {
        if (win->internalId() == window) {
            win->requestActivate();
            return true;
        }
    }
    return false;
}

AutoTypeExecutor* KWinWaylandPlatform::createExecutor()
{
    return new AutoTypeExecutorKWinWayland(*this);
}

void KWinWaylandPlatform::sendKey(xkb_keysym_t keysym, Qt::KeyboardModifiers modifiers)
{
    if (!m_fakeInput) {
        return;
    }


    xkb_keycode_t keycode{};
    xkb_level_index_t level{};
    if (!m_kbstate->keysymToKeycode(keysym, keycode, level)) {
        qWarning() << "No such key: keysym =" << hex << keysym;
        return;
    }

    Qt::KeyboardModifiers wantedMods = Qt::NoModifier;
    if (!m_kbstate->levelToModifiers(keycode, level, wantedMods)) {
        qWarning() << "Cannot find target level: keysym =" << hex << keysym << "keycode =" << hex << keycode << "level =" << level;
        return;
    }
    wantedMods |= modifiers;

    auto originalMods = m_kbstate->modifiers();
    // modifiers that need to be pressed but aren't
    auto needPress = wantedMods & ~originalMods;
    // modifiers that are pressed but maybe shouldn't
    auto needRelease = originalMods & ~wantedMods;

    sendModifierEvent(needRelease, false);
    sendModifierEvent(needPress, true);

    // actually send key, press and release
    sendKeyEvent(keycode, true);
    sendKeyEvent(keycode, false);

    // restore to previous modifiers state
    sendModifierEvent(needPress, false);
    sendModifierEvent(needRelease, true);
}

void KWinWaylandPlatform::sendKeyEvent(xkb_keycode_t keycode, bool pressed)
{
    if (!m_fakeInput) {
        return;
    }

    if (keycode == XKB_KEYCODE_INVALID) {
        return;
    }

    // KWin passes linuxKey + 8 to xkb_state_update_key, so we just pass keycode - 8 to it.
    // See https://phabricator.kde.org/D23766#527078
    quint32 linuxKey = keycode - 8;
    if (pressed) {
        m_fakeInput->requestKeyboardKeyPress(linuxKey);
    } else {
        m_fakeInput->requestKeyboardKeyRelease(linuxKey);
    }
}

void KWinWaylandPlatform::sendModifierEvent(Qt::KeyboardModifiers modifiers, bool pressed)
{
    for (auto &modInfo : KeyboardState::ModifierInfos) {
        if (modifiers.testFlag(modInfo.mask)) {
            sendKeyEvent(m_kbstate->qtModifierToKeycode(modInfo.mask), pressed);
        }
    }
}

void KWinWaylandPlatform::sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    sendKey(m_kbstate->qtKeyToKeysym(key), modifiers);
}

void KWinWaylandPlatform::sendChar(QChar ch)
{
    sendKey(m_kbstate->charToKeysym(ch));
}

AutoTypeExecutorKWinWayland::AutoTypeExecutorKWinWayland(KWinWaylandPlatform& platform)
    : m_platform(platform)
{
}

void AutoTypeExecutorKWinWayland::execChar(AutoTypeChar* action)
{
    m_platform.sendChar(action->character);
}

void AutoTypeExecutorKWinWayland::execKey(AutoTypeKey* action)
{
    m_platform.sendKey(action->key);
}

void AutoTypeExecutorKWinWayland::execClearField(AutoTypeClearField*)
{
    m_platform.sendKey(Qt::Key_Home);
    delay(25);

    m_platform.sendKey(Qt::Key_End, Qt::ControlModifier | Qt::ShiftModifier);
    delay(25);

    m_platform.sendKey(Qt::Key_Backspace);
    delay(25);
}
