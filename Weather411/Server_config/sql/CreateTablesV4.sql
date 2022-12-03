-- phpMyAdmin SQL Dump
-- version 4.9.7
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Apr 30, 2022 at 11:13 AM
-- Server version: 10.3.34-MariaDB
-- PHP Version: 7.3.33

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

-- --------------------------------------------------------

--
-- Table structure for table `mama`
--

CREATE TABLE `mama`(
  `id` bigint(20) NOT NULL,
  `time` timestamp NOT NULL DEFAULT current_timestamp(),
  `Station` int(30) NOT NULL,
  `PowerOnIndex` int(11) NOT NULL,
  `Windspeedmph` float NOT NULL,
  `WindspeedmphMax` float NOT NULL,
  `WindspeedmphMin` float NOT NULL,
  `Winddir` float NOT NULL,
  `QNH` float NOT NULL,
  `Tempc` float NOT NULL,
  `Humidity` float NOT NULL,
  `Battery` float NOT NULL,
  `TimeMillis` bigint(20) NOT NULL,
  `CSQ` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `mama`
--
ALTER TABLE `mama`
  ADD PRIMARY KEY (`id`),
  ADD KEY `TimeMillis` (`TimeMillis`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `mama`
--
ALTER TABLE `mama`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT;
COMMIT;

--
-- Table structure for table `mama_commands`
--

CREATE TABLE `mama_commands`(
  `id` int(11) NOT NULL,
  `command` varchar(120) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `mama_commands`
--
ALTER TABLE `mama_commands`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `mama_commands`
--
ALTER TABLE `mama_commands`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;


/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
