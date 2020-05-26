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

#ifndef KEEPASSXC_KEYBOARDSTATE_H
#define KEEPASSXC_KEYBOARDSTATE_H

#include <QObject>
#include <QFile>

#include <xkbcommon/xkbcommon.h>

#include <memory>

template <typename D, D fn>
using deleter_fn = std::integral_constant<D, fn>;

using xkb_context_unref_fn = deleter_fn<decltype(&xkb_context_unref), &xkb_context_unref>;
using xkb_context_ptr = std::unique_ptr<xkb_context, xkb_context_unref_fn>;

using xkb_keymap_unref_fn = deleter_fn<decltype(&xkb_keymap_unref), &xkb_keymap_unref>;
using xkb_keymap_ptr = std::unique_ptr<xkb_keymap, xkb_keymap_unref_fn>;

using xkb_state_unref_fn = deleter_fn<decltype(&xkb_state_unref), &xkb_state_unref>;
using xkb_state_ptr = std::unique_ptr<xkb_state, xkb_state_unref_fn>;

struct ModifierInfo
{
    Qt::KeyboardModifier mask;
    const char *name;
    xkb_keysym_t keysym;
};

class KeyboardState: public QObject
{
    Q_OBJECT
public:
    static constexpr ModifierInfo ModifierInfos[] = {
        {Qt::ShiftModifier, XKB_MOD_NAME_SHIFT, XKB_KEY_Shift_L},
        {Qt::ControlModifier, XKB_MOD_NAME_CTRL, XKB_KEY_Control_L},
        {Qt::AltModifier, XKB_MOD_NAME_ALT, XKB_KEY_Alt_L},
        {Qt::MetaModifier, XKB_MOD_NAME_LOGO, XKB_KEY_Meta_L},
    };
    static constexpr uint64_t ModifierInfosLen = sizeof(ModifierInfos) / sizeof(ModifierInfo);

    explicit KeyboardState(QObject *parent = nullptr);

    /**
     * From QChar to linux keycode.
     * This is done by first convert unicode char to xkb_keysym_t, then from xkb_keysym_t to xkb_keycode_t by searching over keymap for all valid keycodes
     * and use using xkb_keymap_key_get_syms_by_level
     * @param ch
     * @return Linux keycode is xkb_keycode_t - 8
     */
    xkb_keysym_t charToKeysym(QChar ch);

    /**
     * From Qt::Key to xkb_keysym_t.
     * Only common ones are handled
     * @param key
     * @return xkb_keysym_t
     */
    xkb_keysym_t qtKeyToKeysym(Qt::Key key);



    /**
     * Find the corresponding keycode for the given keysym under the current layout.
     * If not found, keycode and lvl will not be changed.
     * @param keysym the keysym to find
     * @param keycode return the found keycode
     * @param lvl return the found level
     * @return true if found, false otherwise
     */
    bool keysymToKeycode(xkb_keysym_t keysym, xkb_keycode_t &keycode, xkb_level_index_t &level);

    /**
     * Find out what set of modifiers is needed to reach the targetLevel of key.
     * This queries purely the keymap and doesn't take into account current state.
     * @param key
     * @param targetLevel
     * @param modifiers
     * @return true if found
     */
    bool levelToModifiers(xkb_keycode_t key, xkb_level_index_t targetLevel, Qt::KeyboardModifiers &modifiers);

    /**
     * @return current modifiers
     */
    Qt::KeyboardModifiers modifiers() const { return m_modifiers; }

    xkb_keycode_t qtModifierToKeycode(Qt::KeyboardModifier mod);

public slots:
    void updateKeymap(int fd, quint32 size);
    void updateMask(xkb_mod_mask_t depressedModes, xkb_mod_mask_t latchedModes, xkb_mod_mask_t lockedMods, xkb_layout_index_t effectiveLayout);

private:
    xkb_context_ptr m_context = nullptr;
    xkb_keymap_ptr m_keymap = nullptr;
    xkb_state_ptr m_state = nullptr;
    xkb_layout_index_t m_effectiveLayout = 0;
    Qt::KeyboardModifiers m_modifiers = Qt::NoModifier;

    xkb_mod_index_t m_capsMod = XKB_MOD_INVALID;
    xkb_mod_index_t m_modIndices[ModifierInfosLen] = {};
    xkb_keycode_t m_modKeycodes[ModifierInfosLen] = {};

    QFile *m_keymapFile = nullptr;
};

#endif // KEEPASSXC_KEYBOARDSTATE_H
