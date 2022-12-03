<?php

//database connection settings
include 'secrets.php';

//Example data to process sent at weather station startup
//http://www.freeflightwx.com/station.php?MEI=868020030948920&SID=1234&UR=freeflightwx.com/test/up4.php&LOG=0&BCL=0.0039487&RSH=9&AVN=5&VLD=1&VER=4.1_013"


if($_GET["VLD"] == "0"){//settings are not valid, so do not store them
    echo "Settings NOT stored at server\r\n";
}
else{
    
   
    $val_MEI = $_GET["MEI"];
    $val_SID = $_GET["SID"];
    $val_URL = $_GET["URL"];
    $val_URL = "\"" . $val_URL . "\"";
    $val_LOG = $_GET["LOG"];
    $val_BCL = $_GET["BCL"];
    $val_AVN = isset($_GET["AVN"]) ? $_GET["AVN"] : 5;
    $val_RSH = $_GET["RSH"];
    $val_VER = $_GET["VER"];
    $val_VER = "\"" . $val_VER . "\"";
     
    $mysqli = new mysqli($server, $username, $password, $database);
    
    if ($mysqli->connect_errno) {
        echo "Failed to connect to MySQL: " . $mysqli->connect_error;
        // echo "\$RST*\r\n";
    }
    
    $currentTime = round(microtime(true) * 1000);
    
    /// The SQL statement is built

        
    $strSQL = "INSERT INTO " . $stationTable . " (";
    
    $strSQL = $strSQL . "MEI, ";
    $strSQL = $strSQL . "SID, ";
    $strSQL = $strSQL . "URL, ";
    $strSQL = $strSQL . "LOG, ";
    $strSQL = $strSQL . "BCL, ";
    $strSQL = $strSQL . "AVN, ";
    $strSQL = $strSQL . "RSH, ";
    $strSQL = $strSQL . "VER)";
    
    
    $strSQL = $strSQL . " VALUES (";
    
    $strSQL = $strSQL . $val_MEI . ",";
    $strSQL = $strSQL . $val_SID . ",";
    $strSQL = $strSQL . $val_URL . ",";
    $strSQL = $strSQL . $val_LOG . ",";
    $strSQL = $strSQL . $val_BCL . ",";
    $strSQL = $strSQL . $val_AVN . ",";
    $strSQL = $strSQL . $val_RSH . ",";
    $strSQL = $strSQL . $val_VER . ")";
    
    $strSQL = $strSQL . "ON DUPLICATE KEY UPDATE ";

    $strSQL = $strSQL . "SID = " . $val_SID . " ,";
    $strSQL = $strSQL . "URL = " . $val_URL . " ,";
    $strSQL = $strSQL . "LOG = " . $val_LOG . " ,";
    $strSQL = $strSQL . "BCL = " . $val_BCL . " ,";
    $strSQL = $strSQL . "AVN = " . $val_AVN . " ,";
    $strSQL = $strSQL . "RSH = " . $val_RSH . " ,";
    $strSQL = $strSQL . "VER = " . $val_VER ;
    
     
    
    // The SQL statement is executed
    $mysqli->query($strSQL);
    
    // Close the database connection
    $mysqli->close();
    
    
    //send the response
    
    //echo $strSQL;
    //echo "\r\n";
    
    echo "Settings stored at server\r\n";
}

if($_GET["VLD"] == "0"){ //settings are not valid, so send them from the server
    echo "Sending settings from server\r\n";
    $mysqli = new mysqli($server, $username, $password, $database);
    
    if ($mysqli->connect_errno) {
        echo "Failed to connect to MySQL: " . $mysqli->connect_error;
        // echo "\$RST*\r\n";
    }
    
    
    $sql = "SELECT * FROM " . $station_list_table . " WHERE MEI=" . $_GET["MEI"];
    $Result = $mysqli->query($sql);
    
    while($row = $Result->fetch_assoc()){
         
         echo "\$FFWX,4,SID=" . $row['SID'] . "\r\n";
         echo "\$FFWX,4,URL=" . $row['URL'] . "\r\n";
         echo "\$FFWX,4,LOG=" . $row['LOG'] . "\r\n";
         echo "\$FFWX,4,BCL=" . $row['BCL'] . "\r\n";
         echo "\$FFWX,4,RSH=" . $row['RSH'] . "\r\n";
         echo "\$FFWX,4,VLD=1\r\n";
    }
    //Close the database connection
    $mysqli->close();
    
}

$floatTime = microtime(true);
$timestamp = round($floatTime);
     //returns a time line this, in UTC
//20/11/28,12:23:16+00
    echo "\$FFWX,3,CLK=" . gmdate("y/m/d,H:i:s+00", $timestamp);




?>
