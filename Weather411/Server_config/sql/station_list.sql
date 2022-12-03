-- phpMyAdmin SQL Dump
-- version 4.9.7
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Dec 04, 2022 at 07:54 AM
-- Server version: 10.3.37-MariaDB
-- PHP Version: 7.4.33

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `freeflightwx_ffwx`
--

-- --------------------------------------------------------

--
-- Table structure for table `station_list`
--

CREATE TABLE `station_list` (
  `time` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp(),
  `MEI` varchar(20) NOT NULL,
  `SID` int(11) NOT NULL,
  `URL` varchar(200) NOT NULL,
  `LOG` tinyint(1) NOT NULL,
  `BCL` float NOT NULL,
  `RSH` int(11) NOT NULL,
  `VER` varchar(20) NOT NULL,
  `AVN` int(11) NOT NULL,
  `site_uniqueName` varchar(30) DEFAULT 'test',
  `site_speedLowMph` float DEFAULT 3,
  `site_speedOnMph` float DEFAULT 10,
  `site_speedMarginalMph` float DEFAULT 17,
  `site_speedMaxMph` float DEFAULT 150,
  `site_centerDeg` float DEFAULT 0,
  `site_halfWidthDeg` float DEFAULT 45,
  `site_centerDeg2` float DEFAULT NULL,
  `site_halfWidthDeg2` float DEFAULT NULL,
  `site_altitudeFt` float DEFAULT 0,
  `site_dirAdjust` float DEFAULT 0,
  `web_name` varchar(30) DEFAULT 'Test',
  `web_group` varchar(30) DEFAULT NULL,
  `web_path` varchar(30) NOT NULL DEFAULT 'test',
  `web_timezone` varchar(60) DEFAULT 'Australia/Sydney',
  `info_simMoble` varchar(30) DEFAULT NULL,
  `info_stationContactName` varchar(100) DEFAULT NULL,
  `info_stationContactEmail` varchar(100) DEFAULT NULL,
  `sql_data_table` varchar(30) DEFAULT 'test',
  `sql_command_table` varchar(30) DEFAULT 'test_commands'
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `station_list`
--
ALTER TABLE `station_list`
  ADD PRIMARY KEY (`MEI`),
  ADD UNIQUE KEY `site_uniqueName` (`site_uniqueName`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
