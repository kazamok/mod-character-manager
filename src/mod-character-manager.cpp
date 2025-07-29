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

#include "mod-character-manager.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "Language.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "World.h"
#include "WorldSessionMgr.h"
#include <time.h>

CharacterManager::CharacterManager() : _enabled(false), _maxChars(10), _cooldownDays(5) { }

CharacterManager::~CharacterManager() { }

CharacterManager* CharacterManager::instance()
{
    static CharacterManager instance;
    return &instance;
}

void CharacterManager::LoadConfig()
{
    if (!sConfigMgr->LoadModulesConfigs("mod_character_manager.conf"))
    {
        return;
    }

    _enabled = sConfigMgr->GetOption<bool>("CharacterCreationLimit.Enabled", false, "mod_character_manager");
    _maxChars = sConfigMgr->GetOption<uint32>("CharacterCreationLimit.MaxChars", 10, "mod_character_manager");
    _cooldownDays = sConfigMgr->GetOption<uint32>("CharacterCreationLimit.CooldownDays", 5, "mod_character_manager");
}

namespace
{
    class ModCharacterManagerScript : public AccountScript
    {
    public:
        ModCharacterManagerScript() : AccountScript("ModCharacterManagerScript") { }

        bool CanAccountCreateCharacter(uint32 accountId, uint8 /*race*/, uint8 /*class_*/) override
        {
            if (!sCharacterManager->IsEnabled())
            {
                return true;
            }

            if (WorldSession* session = sWorldSessionMgr->FindSession(accountId))
            {
                QueryResult charResult = CharacterDatabase.Query("SELECT guid FROM characters WHERE account = {}", accountId);
                uint32 charCount = charResult ? charResult->GetRowCount() : 0;
                uint32 maxChars = sCharacterManager->GetMaxCharacters();

                if (charCount < maxChars)
                {
                    ChatHandler(session).PSendSysMessage(35501, maxChars - charCount);
                    return true;
                }

                // At character limit, check cooldown
                QueryResult cooldownResult = LoginDatabase.Query("SELECT limit_hit_timestamp FROM account_character_limit WHERE accountId = {}", accountId);

                if (!cooldownResult)
                {
                    // First time hitting the limit
                    LoginDatabase.Execute("INSERT INTO account_character_limit (accountId, limit_hit_timestamp) VALUES ({}, NOW())", accountId);
                    ChatHandler(session).PSendSysMessage(35502, sCharacterManager->GetCooldownDays());
                    session->SendCharCreate(CHAR_CREATE_ERROR);
                    return false;
                }

                Field* fields = cooldownResult->Fetch();
                time_t limitHitTimestamp = fields[0].Get<time_t>();
                time_t now = time(nullptr);
                time_t cooldownEnd = limitHitTimestamp + (sCharacterManager->GetCooldownDays() * DAY);

                if (now < cooldownEnd)
                {
                    // Still in cooldown
                    uint32 remainingDays = (cooldownEnd - now) / DAY + 1;
                    ChatHandler(session).PSendSysMessage(35502, remainingDays);
                    session->SendCharCreate(CHAR_CREATE_ERROR);
                    return false;
                }

                // Cooldown finished, allow creation and reset the timestamp
                LoginDatabase.Execute("UPDATE account_character_limit SET limit_hit_timestamp = NOW() WHERE accountId = {}", accountId);
            }

            return true;
        }
    };
}

void Addmod_character_managerScripts()
{
    sCharacterManager->LoadConfig();
    new ModCharacterManagerScript();
}