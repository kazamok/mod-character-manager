-- filename: account_character_limit.sql

CREATE TABLE IF NOT EXISTS `account_character_limit` (
  `accountId` INT(10) UNSIGNED NOT NULL,
  `limit_hit_timestamp` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`accountId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
