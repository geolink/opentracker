CREATE TABLE `log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `lat` varchar(64) COLLATE utf8_bin NOT NULL,
  `lon` varchar(64) COLLATE utf8_bin NOT NULL,
  `speed` varchar(64) COLLATE utf8_bin NOT NULL,
  `altitude` varchar(64) COLLATE utf8_bin NOT NULL,
  `heading` varchar(64) COLLATE utf8_bin NOT NULL,
  `timestamp` datetime NOT NULL,
  `engineStatus` varchar(64) COLLATE utf8_bin NOT NULL,
  `deviceStatus` varchar(64) COLLATE utf8_bin NOT NULL,
  `gsmLevel` varchar(64) COLLATE utf8_bin NOT NULL,
  `runningTime` bigint(20) unsigned NOT NULL,
  `vehicleBat` varchar(64) COLLATE utf8_bin NOT NULL,
  `engineLock` varchar(64) COLLATE utf8_bin NOT NULL,
  `ip` varchar(16) COLLATE utf8_bin DEFAULT NULL,
  `runningTimeOffset` bigint(20) unsigned NOT NULL,
  `hdop` tinyint(1) DEFAULT NULL,
  `satellites` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE `event` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` datetime NOT NULL,
  `event` varchar(64) COLLATE utf8_bin NOT NULL,
  `moved` varchar(64) COLLATE utf8_bin NOT NULL,
  `moved_total` varchar(64) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
