# Clean all the old variables
file(GLOB_RECURSE MOD_CHARACTER_MANAGER_OLD_SOURCES "src/*")
foreach(MOD_CHARACTER_MANAGER_OLD_SOURCE ${MOD_CHARACTER_MANAGER_OLD_SOURCES})
    string(REPLACE "src/" "" MOD_CHARACTER_MANAGER_OLD_SOURCE ${MOD_CHARACTER_MANAGER_OLD_SOURCE})
    set(MOD_CHARACTER_MANAGER_SRC_${MOD_CHARACTER_MANAGER_OLD_SOURCE} "")
endforeach()

# Define the module name
set(MOD_CHARACTER_MANAGER_NAME "mod-character-manager")

# Define the sources
set(MOD_CHARACTER_MANAGER_SOURCES
    src/mod-character-manager.cpp
)

# Define the SQL scripts
set(MOD_CHARACTER_MANAGER_SQL_DB_AUTH
    data/sql/db-auth/account_character_limit.sql
)

set(MOD_CHARACTER_MANAGER_SQL_DB_WORLD
    data/sql/db-world/acore_string.sql
)

set(MOD_CHARACTER_MANAGER_SQL_DB_CHARACTERS
    data/sql/db-characters/mod-character-manager.sql
)

# Define the configuration files
set(MOD_CHARACTER_MANAGER_CONFIG_FILES
    conf/mod_character_manager.conf.dist
)

# Include directories
set(MOD_CHARACTER_MANAGER_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/server/game/Scripting/ScriptDefines
)

# Include directories
set(MOD_CHARACTER_MANAGER_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/server/game/Scripting
)