<?php

//script to get settings from the server and put them into variables
//$site_uniqueName must be set elsewhere

//load database parameters
include 'secrets.php';

//0. Establish database connection.
mysqli_report(MYSQLI_REPORT_ERROR | MYSQLI_REPORT_STRICT);

try{
    $mysqli = new mysqli($server, $username, $password, $database);
}
catch(Exception $e){
    echo "Failed to connect to MySQL.\r\n";
    echo("Measurement not stored - try again.\r\n");
    exit;
    
}


$sql = "SELECT * FROM `" . $stationTable . "` WHERE `site_uniqueName` = '$site_uniqueName'";

//echo $sql;

$Result= $mysqli->query($sql);

$row = $Result->fetch_assoc();

//weather station info
$station_BASEURL = $row["URL"];
$station_STATIONID = $row["SID"];
$station_PERIOD = 2.25;
$station_AVG_NUM = $row["AVN"];


//database info

$data_table = $row["sql_data_table"];
$command_table = $row["sql_command_table"];

//site info
$site_name = $row["web_name"];
$site_timezone = $row["web_timezone"];

//wind speed params
$siteSpeedLow = $row["site_speedLowMph"];
$siteSpeedOn = $row["site_speedOnMph"];
$siteSpeedMarginal = $row["site_speedMarginalMph"];

//wind dir params
$siteDirOn = $row["site_centerDeg"];
$siteDirWidth = $row["site_halfWidthDeg"];

//otherParams
$siteAltitude = $row["site_altitudeFt"]; //alt in ft
$siteDirAdjust = $row["site_dirAdjust"];//direction adjustment
$siteMaxSpeed = $row["site_speedMaxMph"]; //max recordable wind speed to cancel out electrical interferrence

//$siteBatteryMultiplier = 1.046512 //Mulitply the measured battery voltage by this number to get the real battery voltage - adjusts for ADC variation.

?>
