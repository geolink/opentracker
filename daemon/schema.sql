-- MySQL dump 10.14  Distrib 5.5.44-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: tracker
-- ------------------------------------------------------
-- Server version	5.5.44-MariaDB-1ubuntu0.14.04.1-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `config`
--

DROP TABLE IF EXISTS `config`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `config` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `key` varchar(32) COLLATE utf8_bin NOT NULL,
  `server` varchar(16) COLLATE utf8_bin NOT NULL,
  `port` int(2) NOT NULL,
  `uid` int(2) NOT NULL,
  `gid` int(2) NOT NULL,
  `include_key` tinyint(1) NOT NULL,
  `include_timestamp` tinyint(1) NOT NULL,
  `include_gps_date` tinyint(1) NOT NULL,
  `include_gps_time` tinyint(1) NOT NULL,
  `include_latitude` tinyint(1) NOT NULL,
  `include_longitude` tinyint(1) NOT NULL,
  `include_speed` tinyint(1) NOT NULL,
  `include_altitude` tinyint(1) NOT NULL,
  `include_heading` tinyint(1) NOT NULL,
  `include_hdop` tinyint(1) NOT NULL,
  `include_satellites` tinyint(1) NOT NULL,
  `include_battery_level` tinyint(1) NOT NULL,
  `include_ignition_state` tinyint(1) NOT NULL,
  `include_engine_running_time` tinyint(1) NOT NULL,
  `timestamp_use` varchar(16) COLLATE utf8_bin NOT NULL,
  `show_received` tinyint(1) NOT NULL,
  `debug` tinyint(1) NOT NULL,
  `detect_engineoff_movement` tinyint(1) NOT NULL,
  `engineoff_movement_threshold` int(2) NOT NULL,
  `detect_enginestart_athome` tinyint(1) NOT NULL,
  `athome_file` varchar(64) COLLATE utf8_bin NOT NULL,
  `detect_enginestart_overnight` tinyint(1) NOT NULL,
  `enginestart_overnight_from` varchar(5) COLLATE utf8_bin NOT NULL,
  `enginestart_overnight_to` varchar(5) COLLATE utf8_bin NOT NULL,
  `prowl_api_key` varchar(40) COLLATE utf8_bin NOT NULL,
  `log_journeys` tinyint(1) NOT NULL,
  `event_alerts` tinyint(1) NOT NULL,
  `engine_running_voltage` decimal(4,2) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `event`
--

DROP TABLE IF EXISTS `event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `event` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` datetime NOT NULL,
  `event` varchar(64) COLLATE utf8_bin NOT NULL,
  `moved` varchar(64) COLLATE utf8_bin NOT NULL,
  `moved_total` varchar(64) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `journey`
--

DROP TABLE IF EXISTS `journey`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `journey` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `from_timestamp` datetime NOT NULL,
  `from_latitude` decimal(8,6) NOT NULL,
  `from_longitude` decimal(8,6) NOT NULL,
  `from_place` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `to_timestamp` datetime DEFAULT NULL,
  `to_latitude` decimal(8,6) DEFAULT NULL,
  `to_longitude` decimal(8,6) DEFAULT NULL,
  `to_place` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `routed` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `journey_step`
--

DROP TABLE IF EXISTS `journey_step`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `journey_step` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `journey_id` bigint(20) NOT NULL,
  `timestamp` datetime DEFAULT NULL,
  `latitude` decimal(8,6) NOT NULL,
  `longitude` decimal(8,6) NOT NULL,
  `step` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `journey_id_fk` (`journey_id`),
  CONSTRAINT `journey_id_fk` FOREIGN KEY (`journey_id`) REFERENCES `journey` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `log`
--

DROP TABLE IF EXISTS `log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `log` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `timestamp` datetime NOT NULL,
  `latitude` decimal(8,6) NOT NULL,
  `longitude` decimal(8,6) NOT NULL,
  `speed` decimal(5,2) unsigned NOT NULL,
  `altitude` decimal(6,2) NOT NULL,
  `heading` decimal(5,2) NOT NULL,
  `hdop` tinyint(1) DEFAULT NULL,
  `satellites` tinyint(1) DEFAULT NULL,
  `battery_level` decimal(4,2) NOT NULL,
  `ignition_state` tinyint(1) unsigned NOT NULL,
  `engine_running_time` bigint(20) NOT NULL,
  `ip` varchar(16) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2015-09-08 20:06:58
