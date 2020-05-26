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

#include "AutoTypeWayland.h"
#include "AutoTypeWayland_p.h"

#include "KWinWaylandPlatform.h"

AutoTypePlatformWayland::AutoTypePlatformWayland(QObject* parent)
    : QObject(parent)
{
    // TODO: support other compositors
    m_impl.reset(new KWinWaylandPlatform());
    connect(m_impl.data(), &WaylandPlatformBase::globalShortcutTriggered, this, &AutoTypePlatformWayland::globalShortcutTriggered);
}

bool AutoTypePlatformWayland::isAvailable()
{
    return m_impl && m_impl->isAvailable();
}

void AutoTypePlatformWayland::unload()
{
    Q_ASSERT(m_impl);
    m_impl->unload();
    m_impl.reset();
}

QStringList AutoTypePlatformWayland::windowTitles()
{
    Q_ASSERT(m_impl);
    return m_impl->windowTitles();
}

WId AutoTypePlatformWayland::activeWindow()
{
    Q_ASSERT(m_impl);
    return m_impl->activeWindow();
}

QString AutoTypePlatformWayland::activeWindowTitle()
{
    Q_ASSERT(m_impl);
    return m_impl->activeWindowTitle();
}

bool AutoTypePlatformWayland::registerGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    Q_ASSERT(m_impl);
    return m_impl->registerGlobalShortcut(key, modifiers);
}
void AutoTypePlatformWayland::unregisterGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    Q_ASSERT(m_impl);
    m_impl->unregisterGlobalShortcut(key, modifiers);
}

int AutoTypePlatformWayland::platformEventFilter(void* event)
{
    Q_ASSERT(m_impl);
    return m_impl->platformEventFilter(event);
}

bool AutoTypePlatformWayland::raiseWindow(WId window)
{
    Q_ASSERT(m_impl);
    return m_impl->raiseWindow(window);
}

AutoTypeExecutor* AutoTypePlatformWayland::createExecutor()
{
    Q_ASSERT(m_impl);
    return m_impl->createExecutor();
}
