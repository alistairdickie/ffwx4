<?php
include 'sitedetails.php';

$mysqli = new mysqli($server, $username, $password, $database);

if ($mysqli->connect_errno) {
    echo "Failed to connect to MySQL: " . $mysqli->connect_error;
}


$maxRecords = (int)($timeHistory / ($station_PERIOD * $station_AVG_NUM)) + 1;

date_default_timezone_set($site_timezone);

$currentTime = round(microtime(true) * 1000); //current time in milliseconds since the epoch
$earlyTime = $currentTime - ($timeHistory * 1000);//timeHistory seconds earlier than the current time in milliseconds since the epoch

$sql = "SELECT * FROM ( SELECT * FROM `$data_table` ORDER BY id DESC LIMIT " . $maxRecords . " ) sub WHERE `TimeMillis` >= " . $earlyTime . " ORDER by id ASC";

//echo $sql;

$Result= $mysqli->query($sql);

//some data parameters to use
$maxTime = 0;
$minTime = $currentTime;
$maxWindspeed = 0;

$speedMarginFactor = 1.2;

if($maxWindspeed < $siteSpeedMarginal * $speedMarginFactor){
    $maxWindspeed = $siteSpeedMarginal;

}


$count = 0;
$windArray = array();//the array to hold the data
$windowTimeMillis = $window*1000;


//first - preprocess the data to get some parameters and put the data into windArray


while($row = $Result->fetch_assoc()){

    $time = $row["TimeMillis"]; //milliseconds since the epoch
    if($time > $maxTime){
        $maxTime = $time;
    }
    if($time < $minTime){
        $minTime = $time;
    }


    //Recorded Parameters
    $index   = $row["PowerOnIndex"];
    $time   = $row["TimeMillis"];
    $row['TimeSeconds']=$time/1000.0;
    $windspeed = $row["Windspeedmph"];
    $windspeedMax = $row["WindspeedmphMax"];
    $windspeedMin = $row["WindspeedmphMin"];
    $winddir    = $row["Winddir"];
    $pressure    = $row["QNH"];
    $temp    = $row["Tempc"];
    $humidity    = $row["Humidity"];
    $battery    = $row["Battery"];

    $stationAltM = $siteAltitude * 0.3048;//ft to m
    $QNH = $pressure / pow((1-$stationAltM/44330), 5.25);
    $row['realQNH'] = round($QNH);
    $dewpoint = calculateDewPoint($temp, $humidity);
    $row['DewPoint'] = $dewpoint;

    $row['CloudbaseAGLft'] = 410 * ($temp - $dewpoint);



    if($windspeedMax > $maxWindspeed){
        $maxWindspeed = $windspeedMax;
    }

    //parameters to calculate over the window
    $tmpWindowMax = $windspeedMax;
    $tmpWindowMin = $windspeedMin;
    $windowSpeedAverage = 0;
    $windowTemp = 0;
    $windowHumidity = 0;



    $earlyWindspeedArray = array();
    array_push($earlyWindspeedArray, $windspeed);//include the current one

    $j = count($windArray) - 1;
    //iterate over the window to get averages
    $windowSize = 0;
    while($j >= 0){
        $earlyRow = $windArray[$j];
        $earlyRowTime = $earlyRow["TimeMillis"];

        if($earlyRowTime >= $time - $windowTimeMillis){
            $earlyRowWindspeed = $earlyRow["Windspeedmph"];
            $earlyRowWindspeedMax = $earlyRow["WindspeedmphMax"];
            $earlyRowWindspeedMin = $earlyRow["WindspeedmphMin"];

            array_push($earlyWindspeedArray, $earlyRowWindspeed);
            if($earlyRowWindspeedMin < $tmpWindowMin){
                $tmpWindowMin = $earlyRowWindspeedMin;
            }
            if($earlyRowWindspeedMax > $tmpWindowMax){
                $tmpWindowMax = $earlyRowWindspeedMax;
            }

            $j = $j - 1;
        }
        else{
            $j = -1;
        }

        //summed over the window
        $windowSpeedAverage += $windspeed;
        $windowTemp += $temp;
        $windowHumidity =+ $humidity;


        $windowSize++;
    }

    //add the averages to data
    $row['WindowSpeedMax'] = $tmpWindowMax;
    $row['WindowSpeedMin'] = $tmpWindowMin;
    if($windowSize > 0){
        $row['WindowSpeedAverage'] = $windowSpeedAverage/$windowSize;
        $row['WindowTemp'] = $windowTemp/$windowSize;
        $row['WindowHumidity'] = $windowHumidity/$windowSize;
    }



    $windArray[$count] = $row;
    $count++;

}



function getStringDir($dir){
    //$dir = normaliseDir($dir);

    if($dir > 348.75 || $dir <= 11.25){ return "N  ";}
    else if( $dir <= 33.75){return "NNE";}
    else if( $dir <= 56.25){return "NE ";}
    else if( $dir <= 78.75){return "ENE";}
    else if( $dir <= 101.25){return "E  ";}
    else if( $dir <= 123.75){return "ESE";}
    else if( $dir <= 146.25){return "SE ";}
    else if( $dir <= 168.75){return "SSE";}
    else if( $dir <= 191.25){return "S  ";}
    else if( $dir <= 213.75){return "SSW";}
    else if( $dir <= 236.25){return "SW ";}
    else if( $dir <= 258.75){return "WSW";}
    else if( $dir <= 281.25){return "W  ";}
    else if( $dir <= 303.75){return "WNW";}
    else if( $dir <= 326.25){return "NW ";}
    else if( $dir <= 348.75){return "NNW";}
}

function normaliseDir($dir){
    $dir = $dir % 360;
    if($dir < 0){
        $dir *= -1;
    }
    return $dir;
}

function calculateDewPoint($temperature, $humidity){
    //http://www.faqs.org/faqs/meteorology/temp-dewpoint/
    if ($temperature >= 0) {
        $a = 7.5;
        $b = 237.3;
    } else {
        $a = 7.6;
        $b = 240.7;
    }
    // Saturation steam pressure for temperature
    $SSP = 6.1078 * pow(10, ($a * $temperature) / ($b + $temperature));
    // Steam pressure
    $SP  = $humidity / 100 * $SSP;
    $v   = log($SP / 6.1078, 10);
    return ($b * $v / ($a - $v));
}

?>