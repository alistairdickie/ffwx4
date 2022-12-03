<?php

    include 'sitedetails.php';

    $mysqli = new mysqli($server, $username, $password, $database);

    if ($mysqli->connect_errno) {
        echo "Failed to connect to MySQL: " . $mysqli->connect_error;
    }


//time params

//http://www.acthpa.org/newwind/rawexport.php?timeHistory=600&averageWindow=60&format=json&file=false
//http://www.acthpa.org/newwind/rawexport.php?timeHistory=600&averageWindow=60&format=csv&file=true


$timeHistory = $_GET["time"];//in seconds
$maxRecords = $timeHistory * 10;
$window = $_GET["average"];//in seconds
$format = $_GET["format"];//csv or josn
$file = $_GET["file"];



	//wind speed params
	$siteSpeedLow = 2;
	$siteSpeedOn = 4;
	$siteSpeedMarginal = 5;

	$speedMarginFactor = 1.2;

    date_default_timezone_set('Australia/Sydney');



	$currentTime = round(microtime(true) * 1000); //current time in milliseconds since the epoch
	$earlyTime = $currentTime - ($timeHistory * 1000);//timeHistory seconds earlier than the current time in milliseconds since the epoch


    $sql = "SELECT * FROM ( SELECT * FROM `$data_table` ORDER BY id DESC LIMIT " . $maxRecords . " ) sub WHERE `TimeMillis` >= " . $earlyTime . " ORDER by id ASC";

	$Result= $mysqli->query($sql);



	//some data parameters to use
	$maxTime = 0;
	$minTime = $currentTime;
	$maxWindspeed = 0;

	if($maxWindspeed < $siteSpeedMarginal * $speedMarginFactor){
        $maxWindspeed = $siteSpeedMarginal;

    }


	$count = 0;
	$windArray = array();
	$windowTimeMillis = $window*1000; //300 seconds


//first - preprocess the data to get some parameters and put the data into windArray


	while($row = $Result->fetch_assoc()){

		$time   = $row["TimeMillis"]; //milliseconds since the epoch
        if($time > $maxTime){
            $maxTime = $time;
        }
        if($time < $minTime){
            $minTime = $time;
        }


		$windspeed = $row["Windspeedmph"];
        if($windspeed > $maxWindspeed){
            $maxWindspeed = $windspeed;
        }

        $tmpWindowMax = $windspeed;
        $tmpWindowMin = $windspeed;
        $earlyWindspeedArray = array();
        array_push($earlyWindspeedArray, $windspeed);

        $j = count($windArray) - 1;
        while($j >= 0){
            $earlyRow = $windArray[$j];
            $earlyRowTime = $earlyRow["TimeMillis"];

            if($earlyRowTime >= $time - $windowTimeMillis){
                $earlyRowWindspeed = $earlyRow["Windspeedmph"];
                array_push($earlyWindspeedArray, $earlyRowWindspeed);
                if($earlyRowWindspeed < $tmpWindowMin){
                    $tmpWindowMin = $earlyRowWindspeed;
                }
                if($earlyRowWindspeed > $tmpWindowMax){
                    $tmpWindowMax = $earlyRowWindspeed;
                }

                $j = $j - 1;
            }
            else{
                $j = -1;
            }
        }


        $row['WindowSpeedMax'] = $tmpWindowMax;
        $row['WindowSpeedMin'] = $tmpWindowMin;
        $row['WindowSpeedStdDev'] = stats_standard_deviation($earlyWindspeedArray);
        $row['WindowSpeedMean'] = stats_mean($earlyWindspeedArray);


        $windArray[$count] = $row;
        $count++;

	}

    if($format == "csv"){
        // output headers so that the file is downloaded rather than displayed
        header('Content-Type: text/csv; charset=utf-8');
        header('Content-Disposition: attachment; filename=data.csv');



        // create a file pointer connected to the output stream
        $output = fopen('php://output', 'w');

        // output the column headings
        fputcsv($output, array('id', 'Station', 'PowerOnIndex', 'TimeMillis', 'Windspeedmph', 'Winddir', 'QNH', 'Tempc', 'Humidity', 'Battery','WindowSpeedMax','WindowSpeedMin','WindowSpeedStdDev','WindowSpeedMean'));

        for($i = 0; $i < count($windArray); $i++){
            $row = $windArray[$i];
            fputcsv($output, $row);
        }

    }
    else if($format == "json"){

        header('Content-Type: application/json; charset=utf-8');
        if($file =="true"){
            // output headers so that the file is downloaded rather than displayed
            header('Content-Disposition: attachment; filename=data.json');
        }

        echo json_encode($windArray);

    }



    function stats_standard_deviation(array $a) {
        $n = count($a);
        if ($n === 0) {
            trigger_error("The array has zero elements", E_USER_WARNING);
            return false;
        }
        $mean = array_sum($a) / $n;

        $carry = 0.0;
        foreach ($a as $val) {
            $d = ((double) $val) - $mean;
            $carry += $d * $d;
        };
        return sqrt($carry / $n);
    }
    function stats_mean(array $a) {
        $n = count($a);
        if ($n === 0) {
            trigger_error("The array has zero elements", E_USER_WARNING);
            return false;
        }
        $mean = array_sum($a) / $n;
        return $mean;
    }
?>