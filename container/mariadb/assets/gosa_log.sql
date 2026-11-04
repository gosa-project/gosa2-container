CREATE DATABASE /*!32312 IF NOT EXISTS*/ `gosa_log` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci */;

USE `gosa_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `golog` (
  `time_stamp` datetime DEFAULT NULL,
  `host` varchar(50) DEFAULT NULL,
  `message` varchar(255) DEFAULT NULL,
  `log_level` varchar(15) DEFAULT NULL,
  `matched_dn` varchar(255) DEFAULT NULL,
  `matched_ts` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `gosa_locations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `gosa_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` int(10) NOT NULL,
  `user` text NOT NULL,
  `action` varchar(255) NOT NULL,
  `objecttype` varchar(255) NOT NULL,
  `object` text NOT NULL,
  `changes` blob NOT NULL,
  `result` longtext NOT NULL,
  `location_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `action` (`action`),
  KEY `timestamp` (`timestamp`),
  KEY `objecttype` (`objecttype`),
  KEY `result` (`result`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
INSERT INTO `gosa_locations` VALUES(1, "dc=example,dc=com");
