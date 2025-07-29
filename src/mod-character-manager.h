/*
 * Copyright (C) 2024 WISE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MOD_CHARACTER_MANAGER_H
#define _MOD_CHARACTER_MANAGER_H

#include "Config.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include <optional>

class CharacterManager
{
public:
    static CharacterManager* instance();

    bool IsEnabled() const { return _enabled; }
    uint32 GetMaxCharacters() const { return _maxChars; }
    uint32 GetCooldownDays() const { return _cooldownDays; }

    void LoadConfig();

private:
    CharacterManager();
    ~CharacterManager();

    bool _enabled;
    uint32 _maxChars;
    uint32 _cooldownDays;
};

#define sCharacterManager CharacterManager::instance()

#endif // _MOD_CHARACTER_MANAGER_H