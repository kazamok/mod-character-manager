--
-- Table structure for table `account_character_access`
--
DROP TABLE IF EXISTS `account_character_access`;
CREATE TABLE `account_character_access` (
  `accountId` int(10) unsigned NOT NULL,
  `access_level` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '0: Normal, 1: Whitelist, 2: Blacklist',
  PRIMARY KEY (`accountId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Account Character Creation Access Control';

--
-- Whitelist GM accounts (ID 1-20)
--
INSERT INTO `account_character_access` (`accountId`, `access_level`) VALUES
(1, 1), (2, 1), (3, 1), (4, 1), (5, 1),
(6, 1), (7, 1), (8, 1), (9, 1), (10, 1),
(11, 1), (12, 1), (13, 1), (14, 1), (15, 1),
(16, 1), (17, 1), (18, 1), (19, 1), (20, 1)
ON DUPLICATE KEY UPDATE `access_level`=VALUES(`access_level`);