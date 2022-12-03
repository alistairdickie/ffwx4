<?php

include 'sitedetails.php';

$floatTime = microtime(true);
$currentTime = round($floatTime * 1000);
$timestamp = round($floatTime);

//0. Check to see if settings were accidently sent to here on startup. 
if(isset($_GET["MEI"])){
    exit("Thanks for sending settings, but you should send them to http://www.freeflightwx.com/station.php \r\n");
}

//1. Make sure that the station id is correct - this is like a pin code.
$s = $_GET["s"];
if($station_STATIONID != $s){
    exit ("Wrong station id. Check SID setting on station (default is 1234).\r\n");

}

//2. Establish database connection.
mysqli_report(MYSQLI_REPORT_ERROR | MYSQLI_REPORT_STRICT);

try{
    $mysqli = new mysqli($server, $username, $password, $database);
}
catch(Exception $e){
    echo "Failed to connect to MySQL.\r\n";
    echo("Measurement not stored - try again.\r\n");
    exit;
    
}
    
//3. Alter some settings.
//adjust direction
$measuredDir = $_GET["d"];
$measuredDir = normaliseDir($measuredDir += $siteDirAdjust);

//clamp speeds
$measuredSpeed = $_GET["w"];
if($measuredSpeed > $siteMaxSpeed){
    $measuredSpeed = $siteMaxSpeed;
}

$measuredSpeedMax = $_GET["x"];
if($measuredSpeedMax > $siteMaxSpeed){
    $measuredSpeedMax = $siteMaxSpeed;
}

$measuredSpeedMin = $_GET["n"];
if($measuredSpeedMin > $siteMaxSpeed){
    $measuredSpeedMin = $siteMaxSpeed;
}

$val_c = isset($_GET["c"]) ? $_GET["c"] : -99;

if(isset($_GET["e"])){
    if($_GET["e"] == 0 ){
        $val_currentTime = $currentTime;
    } 
    else{
        $val_currentTime = $_GET["e"];
    }
}
else{
   $val_currentTime = $currentTime; 
}

//4. Build the insert string and insert it into the database
$strSQL = "INSERT INTO " . $data_table . " (";

$strSQL = $strSQL . "Station, ";
$strSQL = $strSQL . "PowerOnIndex, ";
$strSQL = $strSQL . "TimeMillis, ";
$strSQL = $strSQL . "Windspeedmph, ";
$strSQL = $strSQL . "WindspeedmphMax, ";
$strSQL = $strSQL . "WindspeedmphMin, ";
$strSQL = $strSQL . "Winddir, ";
$strSQL = $strSQL . "QNH, ";
$strSQL = $strSQL . "Tempc, ";
$strSQL = $strSQL . "Humidity,";
$strSQL = $strSQL . "Battery,";
$strSQL = $strSQL . "CSQ)";


$strSQL = $strSQL . "VALUES(";

$strSQL = $strSQL . $_GET["s"] . ",";
$strSQL = $strSQL . $_GET["p"] . ",";
$strSQL = $strSQL . $val_currentTime . ",";
$strSQL = $strSQL . $_GET["w"] . ",";
$strSQL = $strSQL . $_GET["x"] . ",";
$strSQL = $strSQL . $_GET["n"] . ",";
$strSQL = $strSQL . $measuredDir . ",";
$strSQL = $strSQL . $_GET["q"] . ",";
$strSQL = $strSQL . $_GET["t"] . ",";
$strSQL = $strSQL . $_GET["h"] . ",";
$strSQL = $strSQL . $_GET["b"] . ",";
$strSQL = $strSQL . $val_c . ")";

// The SQL statement is executed

$mysqli->query($strSQL);


//5. Success response from server with current time.
echo "\$FFWX,2," . $_GET["p"] . "," . $currentTime . "\r\n";

//6. And respond with server time occasionally. 
if($_GET["e"] == 0 or $_GET["p"] % 16 == 0){ //sends a CLK if the time is 0, or once every 16 measurements - every 3 minutes at 11.25 seconds between measurement. This keeps the station synced with the server time. Ignore the transmission time delay. 
     //returns a time line this, in UTC
//20/11/28,12:23:16+00
    echo "\$FFWX,3,CLK=" . gmdate("y/m/d,H:i:s+00", $timestamp);
}
    
// Close the database connection
$mysqli->close();


//7. Now process and delete each command in the commands table
try{
    $mysqli = new mysqli($server, $username, $password, $database);
}
catch(Exception $e){
    echo "Failed to connect to MySQL for commands.\r\n";
    echo("Commands not considered - try again.\r\n");
    exit;
    
}

$sql = "SELECT * FROM `$command_table` ORDER by id ASC";

$Result= $mysqli->query($sql);

if (!$Result) {

    throw new Exception("Database Error [{$mysqli->errno}] {$mysqli->error}");
   
}

while($row = $Result->fetch_assoc()){
    $id = $row["id"];
    $command = $row["command"];
    echo "Command id ". $id . "\r\n" . $command . "\r\n";
    
    $sql = "DELETE FROM `$command_table` WHERE `$command_table`.`id` = ". $id;
    $Result2= $mysqli->query($sql);
    if (!$Result2) {
        throw new Exception("Database Error [{$mysqli->errno}] {$mysqli->error}");
    }
}

// Close the database connection
$mysqli->close();

function normaliseDir($dir){
    $dir = $dir % 360;
    if($dir < 0){
        $dir *= -1;
    }
    return $dir;
}



?>
