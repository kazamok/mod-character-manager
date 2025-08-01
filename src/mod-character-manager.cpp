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
#include "WorldSession.h"
#include "WorldSessionMgr.h"
#include "Mail.h"
#include <time.h>

CharacterManager::CharacterManager() : _enabled(false), _accessControlEnabled(false), _maxChars(10), _cooldownDays(5) { }

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
    _accessControlEnabled = sConfigMgr->GetOption<bool>("mod_character_manager.AccessControl.Enabled", false, "mod_character_manager");
    _maxChars = sConfigMgr->GetOption<uint32>("CharacterCreationLimit.MaxChars", 10, "mod_character_manager");
    _cooldownDays = sConfigMgr->GetOption<uint32>("CharacterCreationLimit.CooldownDays", 5, "mod_character_manager");
}

namespace
{
    // This script handles the character creation attempts.
    class ModCharacterManagerAccountScript : public AccountScript
    {
    public:
        ModCharacterManagerAccountScript() : AccountScript("ModCharacterManagerAccountScript") { }

        bool CanAccountCreateCharacter(uint32 accountId, uint8 /*race*/, uint8 /*class_*/) override
        {
            WorldSession* session = sWorldSessionMgr->FindSession(accountId);
            if (!session)
            {
                return false; // Failsafe: If no session, block creation.
            }

            if (sCharacterManager->IsAccessControlEnabled())
            {
                if (QueryResult accessResult = LoginDatabase.Query("SELECT access_level FROM account_character_access WHERE accountId = {}", accountId))
                {
                    Field* fields = accessResult->Fetch();
                    uint8 accessLevel = fields[0].Get<uint8>();

                    if (accessLevel == 2) // Blacklisted
                    {
                        ChatHandler(session).PSendSysMessage(35503); // Custom message for blacklist
                        session->SendCharCreate(CHAR_CREATE_ERROR);
                        return false;
                    }
                    
                    if (accessLevel == 1) // Whitelisted
                    {
                        return true; // Bypass all other checks
                    }
                }
            }

            if (!sCharacterManager->IsEnabled())
            {
                return true;
            }

            // Check if a cooldown is already active for this account.
            if (QueryResult cooldownResult = LoginDatabase.Query("SELECT limit_hit_timestamp FROM account_character_limit WHERE accountId = {}", accountId))
            {
                Field* fields = cooldownResult->Fetch();
                time_t limitHitTimestamp = fields[0].Get<time_t>();
                time_t now = time(nullptr);
                time_t cooldownEnd = limitHitTimestamp + (sCharacterManager->GetCooldownDays() * DAY);

                if (now < cooldownEnd)
                {
                    // Cooldown is active and not expired, block creation.
                    uint32 remainingDays = (cooldownEnd - now) / DAY + 1;
                    ChatHandler(session).PSendSysMessage(35502, remainingDays);
                    session->SendCharCreate(CHAR_CREATE_ERROR);
                    return false;
                }
                else
                {
                    // Cooldown has expired, remove the record from the database.
                    LoginDatabase.Execute("DELETE FROM account_character_limit WHERE accountId = {}", accountId);
                }
            }

            // If not in cooldown, allow creation. OnPlayerCreate will handle starting cooldown if limit is reached.
            // We still send the message about remaining slots if any.
            QueryResult charResult = CharacterDatabase.Query("SELECT guid FROM characters WHERE account = {}", accountId);
            uint32 charCount = charResult ? charResult->GetRowCount() : 0;
            uint32 maxChars = sCharacterManager->GetMaxCharacters();

            if (charCount < maxChars)
            {
                ChatHandler(session).PSendSysMessage(35501, maxChars - charCount);
            }

            return true;
        }
    };

    // This script handles the character creation event to start the cooldown.
    class ModCharacterManagerPlayerScript : public PlayerScript
    {
    public:
        ModCharacterManagerPlayerScript() : PlayerScript("ModCharacterManagerPlayerScript") { }

        void OnPlayerCreate(Player* player) override
        {
            if (!sCharacterManager->IsEnabled())
            {
                return;
            }

            uint32 accountId = player->GetSession()->GetAccountId();

            // Get the actual current character count for this account.
            QueryResult charResult = CharacterDatabase.Query("SELECT guid FROM characters WHERE account = {}", accountId);
            uint32 charCount = charResult ? charResult->GetRowCount() : 0;
            uint32 maxChars = sCharacterManager->GetMaxCharacters();

            // If the account has reached or exceeded the maximum character limit, start the cooldown.
            if (charCount >= maxChars)
            {
                LoginDatabase.Execute("REPLACE INTO account_character_limit (accountId, limit_hit_timestamp) VALUES ({}, UNIX_TIMESTAMP())", accountId);

                // Send in-game mail to the player
                std::string subject = "캐릭터 생성 제한 안내";
                std::string body = "총 생성 가능한 캐릭터 수는 삭제된 캐릭터를 포함하여 총 " + std::to_string(sCharacterManager->GetMaxCharacters()) + "개 입니다.\n";
                body += "생성 가능한 캐릭터 수를 초과하여 새로운 캐릭터를 생성할 수 없습니다.\n";
                body += "추가 생성이 가능한 시기는 " + std::to_string(sCharacterManager->GetCooldownDays()) + "일 후 입니다.";

                MailDraft draft(subject, body);
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                draft.SendMailTo(trans, MailReceiver(player), MailSender(MAIL_NORMAL, 0), MAIL_CHECK_MASK_HAS_BODY);
                CharacterDatabase.CommitTransaction(trans);
            }
        }

        // OnPlayerDelete is not used for starting cooldown, as OnPlayerCreate handles it.
        // The cooldown record will persist until CanAccountCreateCharacter clears it.
        void OnPlayerDelete(ObjectGuid /*guid*/, uint32 /*accountId*/) override
        {
            // No action needed here for cooldown logic.
        }
    };
}

void Addmod_character_managerScripts()
{
    sCharacterManager->LoadConfig();
    new ModCharacterManagerAccountScript();
    new ModCharacterManagerPlayerScript();
}

