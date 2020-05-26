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

#include "KeyboardState.h"
#include "UnicodeToKeysyms.h"

#include <QDebug>
#include <iostream>

constexpr ModifierInfo KeyboardState::ModifierInfos[];
constexpr uint64_t KeyboardState::ModifierInfosLen;

KeyboardState::KeyboardState(QObject* parent)
    : QObject(parent)
    , m_context(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
    , m_keymap(xkb_keymap_new_from_names(m_context.get(), nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS))
    , m_state(xkb_state_new(m_keymap.get()))
    , m_keymapFile(new QFile(this))
{
}

void KeyboardState::updateKeymap(int fd, quint32 size)
{
    if (!m_keymapFile->open(fd, QIODevice::ReadOnly, QFileDevice::AutoCloseHandle)) {
        qWarning() << "updateKeymap: Failed to open fd: " << fd;
        return;
    }
    auto buf = m_keymapFile->map(0, size);
    if (!buf) {
        qWarning() << "updateKeymap: Failed to map file";
        return;
    }

    m_keymap.reset(xkb_keymap_new_from_string(m_context.get(), reinterpret_cast<const char*>(buf), XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (!m_keymap) {
        qWarning() << "updateKeymap: Failed to compile keymap, restore to default";
        qDebug() << "updateKeymap: the keymap was";
        std::cerr << qPrintable(QString::fromLocal8Bit(reinterpret_cast<const char*>(buf), size));
        m_keymap.reset(xkb_keymap_new_from_names(m_context.get(), nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS));
        Q_ASSERT(m_keymap);
    }

    m_state.reset(xkb_state_new(m_keymap.get()));
    if (!m_state) {
        qWarning() << "updateKeymap: Failed to create new xkb_state";
    }

    // get mod index and keycode for each mod we care
    for (uint64_t i = 0; i != ModifierInfosLen; ++i) {
        auto &info = ModifierInfos[i];
        m_modIndices[i] = xkb_keymap_mod_get_index(m_keymap.get(), info.name);

        xkb_level_index_t level = 0;
        auto res = keysymToKeycode(info.keysym, m_modKeycodes[i], level);
        if (!res || level != 0) {
            qWarning() << "updateKeymap: Failed to find mod" << info.name << res << m_modKeycodes[i] << level;
            m_modKeycodes[i] = XKB_KEYCODE_INVALID;
        }
    }
    m_capsMod = xkb_keymap_mod_get_index(m_keymap.get(), XKB_MOD_NAME_CAPS);
}

void KeyboardState::updateMask(xkb_mod_mask_t depressedModes,
                               xkb_mod_mask_t latchedModes,
                               xkb_mod_mask_t lockedMods,
                               xkb_layout_index_t effectiveLayout)
{
    if (!m_state) {
        return;
    }

    // we only have effective_layout, which is a combination of depressed|latched|locked layout.
    // but since xkbcommon will combine them anyway, passing effective to depressed should be enough for our purpose.
    xkb_state_update_mask(m_state.get(), depressedModes, latchedModes, lockedMods, effectiveLayout, 0, 0);
    m_effectiveLayout = effectiveLayout;

    Qt::KeyboardModifiers mods = Qt::NoModifier;
    for (uint64_t i = 0; i != ModifierInfosLen; ++i) {
        auto &info = ModifierInfos[i];
        if (xkb_state_mod_index_is_active(m_state.get(), m_modIndices[i], XKB_STATE_MODS_EFFECTIVE) == 1) {
            mods |= info.mask;
        }
    }
    // caps lock also triggers shift mod
    if (xkb_state_mod_index_is_active(m_state.get(), m_capsMod, XKB_STATE_MODS_EFFECTIVE) == 1) {
        mods |= Qt::ShiftModifier;
    }
    m_modifiers = mods;
}

xkb_keysym_t KeyboardState::qtKeyToKeysym(Qt::Key key)
{
    switch (key) {
    case Qt::Key_Tab:
        return XKB_KEY_Tab;
    case Qt::Key_Enter:
        return XKB_KEY_Return;
    case Qt::Key_Space:
        return XKB_KEY_space;
    case Qt::Key_Up:
        return XKB_KEY_Up;
    case Qt::Key_Down:
        return XKB_KEY_Down;
    case Qt::Key_Left:
        return XKB_KEY_Left;
    case Qt::Key_Right:
        return XKB_KEY_Right;
    case Qt::Key_Insert:
        return XKB_KEY_Insert;
    case Qt::Key_Delete:
        return XKB_KEY_Delete;
    case Qt::Key_Home:
        return XKB_KEY_Home;
    case Qt::Key_End:
        return XKB_KEY_End;
    case Qt::Key_PageUp:
        return XKB_KEY_Page_Up;
    case Qt::Key_PageDown:
        return XKB_KEY_Page_Down;
    case Qt::Key_Backspace:
        return XKB_KEY_BackSpace;
    case Qt::Key_Pause:
        return XKB_KEY_Break;
    case Qt::Key_CapsLock:
        return XKB_KEY_Caps_Lock;
    case Qt::Key_Escape:
        return XKB_KEY_Escape;
    case Qt::Key_Help:
        return XKB_KEY_Help;
    case Qt::Key_NumLock:
        return XKB_KEY_Num_Lock;
    case Qt::Key_Print:
        return XKB_KEY_Print;
    case Qt::Key_ScrollLock:
        return XKB_KEY_Scroll_Lock;
    case Qt::Key_Shift:
        return XKB_KEY_Shift_L;
    case Qt::Key_Control:
        return XKB_KEY_Control_L;
    case Qt::Key_Alt:
        return XKB_KEY_Alt_L;
    default:
        if (key >= Qt::Key_F1 && key <= Qt::Key_F16) {
            return XKB_KEY_F1 + (key - Qt::Key_F1);
        } else {
            return XKB_KEY_NoSymbol;
        }
    }
}

template <typename RandomAccessIterator, typename T>
RandomAccessIterator binaryFind(RandomAccessIterator begin, RandomAccessIterator end, const T& value)
{
    RandomAccessIterator it = std::lower_bound(begin, end, value);

    if ((it == end) || (value < *it)) {
        return end;
    } else {
        return it;
    }
}

xkb_keysym_t KeyboardState::charToKeysym(QChar ch)
{
    auto unicode = ch.unicode();

    /* first check for Latin-1 characters (1:1 mapping) */
    if ((unicode >= 0x0020 && unicode <= 0x007e) || (unicode >= 0x00a0 && unicode <= 0x00ff)) {
        return unicode;
    }

    /* mapping table generated from keysymdef.h */
    auto match = binaryFind(UnicodeToKeysymKeys, UnicodeToKeysymKeys + UnicodeToKeysymLen, unicode);
    size_t index = match - UnicodeToKeysymKeys;
    if (index != UnicodeToKeysymLen) {
        return UnicodeToKeysymValues[index];
    }

    if (unicode >= 0x0100) {
        return unicode | 0x01000000;
    }

    return XKB_KEY_NoSymbol;
}

bool KeyboardState::keysymToKeycode(xkb_keysym_t keysym, xkb_keycode_t& keycode, xkb_level_index_t& level)
{
    for (auto key = xkb_keymap_min_keycode(m_keymap.get()),
              maxCode = xkb_keymap_max_keycode(m_keymap.get());
         key != maxCode; ++key) {
        auto numLevels = xkb_keymap_num_levels_for_key(m_keymap.get(), key, m_effectiveLayout);
        for (xkb_level_index_t lvl = 0; lvl != numLevels; ++lvl) {
            const xkb_keysym_t* syms = nullptr;
            auto numSyms = xkb_keymap_key_get_syms_by_level(m_keymap.get(), key, m_effectiveLayout, lvl, &syms);
            if (!numSyms || !syms) {
                continue;
            }
            if (syms[0] == keysym) {
                keycode = key;
                level = lvl;
                return true;
            }
        }
    }

    return false;
}

bool KeyboardState::levelToModifiers(xkb_keycode_t key, xkb_level_index_t targetLevel, Qt::KeyboardModifiers& modifiers)
{
    if (!m_keymap) {
        return false;
    }

    // use a bitmask to loop through all combination of modifiers
    static_assert(ModifierInfosLen < 8, "A larger integer type is needed");
    for (uint8_t bits = 0; bits != 1 << ModifierInfosLen; ++bits) {
        // create a fresh state to do the query
        Qt::KeyboardModifiers needed = Qt::NoModifier;
        xkb_state_ptr state{xkb_state_new(m_keymap.get())};
        // drive the state to have the key
        for (uint64_t i = 0; i != ModifierInfosLen; ++i) {
            if (bits & (1 << i)) {
                xkb_state_update_key(state.get(), m_modKeycodes[i], XKB_KEY_DOWN);
                needed |= ModifierInfos[i].mask;
            }
        }
        // see if we get there
        if (xkb_state_key_get_level(state.get(), key, m_effectiveLayout) == targetLevel) {
            // stuff this into the bitmask
            modifiers = needed;
            return true;
        }
    }

    return false;
}

xkb_keycode_t KeyboardState::qtModifierToKeycode(Qt::KeyboardModifier mod)
{
    for (uint64_t i = 0; i != ModifierInfosLen; ++i) {
        if (ModifierInfos[i].mask == mod) {
            return m_modKeycodes[i];
        }
    }
    return XKB_KEYCODE_INVALID;
}
