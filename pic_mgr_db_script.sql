-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               10.4.10-MariaDB - mariadb.org binary distribution
-- Server OS:                    Win64
-- HeidiSQL Version:             10.2.0.5599
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dumping database structure for pic_mgr
CREATE DATABASE IF NOT EXISTS `pic_mgr` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci */;
USE `pic_mgr`;

-- Dumping structure for table pic_mgr.directory
CREATE TABLE IF NOT EXISTS `directory` (
  `dir_id` int(11) NOT NULL AUTO_INCREMENT,
  `dir` varchar(512) COLLATE utf8_unicode_ci NOT NULL,
  `has_layers` int(11) DEFAULT NULL,
  `is_recursive` int(11) DEFAULT NULL,
  PRIMARY KEY (`dir_id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.folder
CREATE TABLE IF NOT EXISTS `folder` (
  `folder_id` int(11) NOT NULL AUTO_INCREMENT,
  `dir_id` int(11) NOT NULL,
  `folder_name` varchar(512) COLLATE utf8_unicode_ci NOT NULL,
  `folder_added` datetime DEFAULT NULL,
  `folder_opened` datetime DEFAULT NULL,
  PRIMARY KEY (`folder_id`),
  KEY `folder_directories` (`dir_id`)
) ENGINE=InnoDB AUTO_INCREMENT=161 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.folder_site
CREATE TABLE IF NOT EXISTS `folder_site` (
  `folder_id` int(11) NOT NULL,
  `web_site_id` int(11) NOT NULL,
  `current_url` varchar(1024) COLLATE utf8_unicode_ci DEFAULT NULL,
  `last_activity` datetime DEFAULT NULL,
  PRIMARY KEY (`folder_id`,`web_site_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.hashtag_folder
CREATE TABLE IF NOT EXISTS `hashtag_folder` (
  `hashtag_id` int(11) NOT NULL AUTO_INCREMENT,
  `folder_id` int(11) NOT NULL,
  `name` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`hashtag_id`),
  KEY `hashtag_folder_ndx` (`folder_id`)
) ENGINE=InnoDB AUTO_INCREMENT=487 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for procedure pic_mgr.hashtag_folder_insert_proc
DELIMITER //
CREATE DEFINER=`root`@`%` PROCEDURE `hashtag_folder_insert_proc`(
 iName     VARCHAR(255),
 iFolderID INT 
)
BEGIN

INSERT INTO hashtag_folder(name,folder_id) VALUES (iName, iFolderID);  
SELECT LAST_INSERT_ID();

END//
DELIMITER ;

-- Dumping structure for table pic_mgr.hashtag_global
CREATE TABLE IF NOT EXISTS `hashtag_global` (
  `hashtag_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`hashtag_id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.picture
CREATE TABLE IF NOT EXISTS `picture` (
  `picture_id` int(11) NOT NULL AUTO_INCREMENT,
  `folder_id` int(11) NOT NULL,
  `file_name` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
  `picture_added` datetime DEFAULT NULL,
  `width` int(11) NOT NULL,
  `height` int(11) NOT NULL,
  `sub_dir` varchar(1024) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`picture_id`),
  KEY `picture_folders` (`folder_id`)
) ENGINE=InnoDB AUTO_INCREMENT=47809 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.picture_folder_hashtag
CREATE TABLE IF NOT EXISTS `picture_folder_hashtag` (
  `picture_id` int(11) NOT NULL,
  `hashtag_id` int(11) NOT NULL,
  UNIQUE KEY `Index 2` (`picture_id`,`hashtag_id`),
  KEY `picture_folder_hashtag_ndx` (`picture_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.picture_folder_subtag
CREATE TABLE IF NOT EXISTS `picture_folder_subtag` (
  `picture_id` int(11) NOT NULL,
  `hashtag_id` int(11) NOT NULL,
  `sub_hashtag_id` int(11) NOT NULL,
  UNIQUE KEY `Index 2` (`picture_id`,`hashtag_id`,`sub_hashtag_id`),
  KEY `Index 1` (`picture_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.picture_global_hashtag
CREATE TABLE IF NOT EXISTS `picture_global_hashtag` (
  `picture_id` int(11) NOT NULL,
  `hashtag_id` int(11) NOT NULL,
  UNIQUE KEY `Index 2` (`picture_id`,`hashtag_id`),
  KEY `picture_global_hashtag_ndx` (`picture_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

-- Dumping structure for procedure pic_mgr.picture_insert_proc
DELIMITER //
CREATE DEFINER=`root`@`%` PROCEDURE `picture_insert_proc`(
 iFldr     INT,
 iName     VARCHAR(1024),
 iWidth    INT,
 iHeight   INT
)
BEGIN

INSERT INTO picture(folder_id, file_name, picture_added, width, height) VALUES (iFldr,iName, NOW(), iWidth, iHeight); 
SELECT LAST_INSERT_ID();

END//
DELIMITER ;

-- Dumping structure for table pic_mgr.picture_thumbnail
CREATE TABLE IF NOT EXISTS `picture_thumbnail` (
  `picture_id` int(11) NOT NULL,
  `thumbnail` blob NOT NULL DEFAULT '',
  PRIMARY KEY (`picture_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Data exporting was unselected.

-- Dumping structure for table pic_mgr.user_settings
CREATE TABLE IF NOT EXISTS `user_settings` (
  `user_name` varchar(16) NOT NULL,
  `app_name` varchar(50) NOT NULL,
  `setting_name` varchar(120) NOT NULL,
  `setting_value` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`user_name`,`app_name`,`setting_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Data exporting was unselected.

-- Dumping structure for procedure pic_mgr.user_settings_update_proc
DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `user_settings_update_proc`(
 app VARCHAR(50),
 usr VARCHAR(16),
 stn VARCHAR(120),
 val VARCHAR(256)
 )
BEGIN
 
 DECLARE numrows INT;

 SELECT COUNT(*) INTO numrows FROM user_settings WHERE app_name=app AND user_name=usr AND setting_name=stn;
 if numrows > 0 THEN
   UPDATE user_settings SET setting_value=val
   WHERE app_name=app AND user_name=usr AND setting_name=stn;
 ELSE
   INSERT INTO user_settings 
	  (app_name, user_name, setting_name, setting_value) 
   VALUES 
	  (app, usr, stn, val);
 END IF;

END//
DELIMITER ;

-- Dumping structure for table pic_mgr.web_site
CREATE TABLE IF NOT EXISTS `web_site` (
  `web_site_id` int(11) NOT NULL AUTO_INCREMENT,
  `web_site_name` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `web_url` varchar(256) COLLATE utf8_unicode_ci DEFAULT NULL,
  `walk_settings` int(11) DEFAULT NULL,
  `walk_strings` varchar(256) COLLATE utf8_unicode_ci DEFAULT NULL,
  `image_settings` int(11) DEFAULT NULL,
  `image_strings` varchar(256) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`web_site_id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Data exporting was unselected.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
