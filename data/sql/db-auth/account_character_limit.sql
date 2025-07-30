-- filename: account_character_limit.sql

CREATE TABLE IF NOT EXISTS `account_character_limit` (
	`accountId` INT UNSIGNED NOT NULL,
	`limit_hit_timestamp` BIGINT NOT NULL,
	PRIMARY KEY (`accountId`) USING BTREE
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;
