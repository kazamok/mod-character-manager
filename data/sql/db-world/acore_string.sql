-- filename: acore_string.sql

-- filename: acore_string.sql

DELETE FROM `acore_string` WHERE `entry` IN (35501, 35502);

INSERT INTO `acore_string` (`entry`, `content_default`, `locale_koKR`) VALUES
(35501, '생성 가능한 케릭터 수는 {}개 입니다', '생성 가능한 케릭터 수는 {}개 입니다'),
(35502, '추가 생성 가능한 시기는 {}일 후 입니다', '추가 생성 가능한 시기는 {}일 후 입니다');
 