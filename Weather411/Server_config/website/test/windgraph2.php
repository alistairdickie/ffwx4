<?php

    include 'sitedetails.php';

    date_default_timezone_set($site_timezone);

	$mysqli = new mysqli($server, $username, $password, $database);

	if ($mysqli->connect_errno) {
	    echo "Failed to connect to MySQL: " . $mysqli->connect_error;
	}


    $currentTime = time();


    //time params

    $beginTime = setVarFromIndex("begin", date("Ymd",($currentTime)));
    $endTime = setVarFromIndex("end", date("Ymd",($currentTime+86400)));

    $beginTime = strtotime($beginTime);//unix time
    $endTime = strtotime($endTime);//unix time



    $window = setVarFromIndex("window",600);

    //overall image params
    $width = setVarFromIndex("width", 1200);
    $height = setVarFromIndex("height", 600);

    //display params

    $option_errorbars = setVarFromIndex("error", 0);
    $option_max = setVarFromIndex("max", 1);
    $option_min =setVarFromIndex("min", 1);
    $option_markers = setVarFromIndex("markers", 0);
    $option_average = setVarFromIndex("average", 1);
    $option_pointSize = setVarFromIndex("point", 1);
    $option_timeDisplay = setVarFromIndex("timeDisplay", true);


    $widthMargin = 10;
    $heightMargin = 20;
    $speedMarginFactor = 1.1;




    $timeHistory = $endTime - $beginTime;



    $timeNum;
    $timeName;
    if($timeHistory <= 3600){
        $timeNum = $timeHistory / 60;
        $timeName = 'minutes';


    }
    else{
        $timeNum = $timeHistory / 3600;
        $timeName = 'hours';
    }

    $maxRecords = (int)(($currentTime - $beginTime) / ($station_PERIOD * $station_AVG_NUM)) + 1;

    $checkStart = microtime(true);

	$sql = "SELECT * FROM ( SELECT * FROM `$data_table` ORDER BY id DESC LIMIT " . $maxRecords . " ) sub WHERE `TimeMillis` BETWEEN " . $beginTime * 1000 . " AND " . $endTime * 1000 ." ORDER by id ASC";




    //echo $sql;

	$Result= $mysqli->query($sql);
    $mysqli->close();



	//some data parameters to use
	$maxTime = 0;
	$minTime = $currentTime;
	$maxWindspeed = 0;

	if($maxWindspeed < $siteSpeedMarginal * $speedMarginFactor){
        $maxWindspeed = $siteSpeedMarginal;

    }


	$count = 0;
	$windArray = array();
	$windowTimeMillis = $window*1000;

//first - put all of the data into windArray
while($row = $Result->fetch_assoc()){
    $windArray[$count] = $row;
    $count++;
}

$Result->data_seek(0);
$Result->free();

//first - process the data

    $i = count($windArray) - 1;


while($i >= 0){

    $row = $windArray[$i];

    $time = $row["TimeMillis"]; //
    if($time > $maxTime){
        $maxTime = $time;
    }
    if($time < $minTime){
        $minTime = $time;
    }


    $windspeed = $row["Windspeedmph"];
    $windspeedMax = $row["WindspeedmphMax"];
    $windspeedMin = $row["WindspeedmphMin"];

    if($windspeedMax > $maxWindspeed){
        $maxWindspeed = $windspeedMax;
    }

    $tmpWindowMax = $windspeedMax;
    $tmpWindowMin = $windspeedMin;

    $j = $i;
    //include the current one
    $windowRecords = 1;
    $windowSum = $windspeed;

    while($j >= 0){


        $earlyRow = $windArray[$j];

        $earlyRowTime = $earlyRow["TimeMillis"];

        if($earlyRowTime >= $time - $windowTimeMillis){
            $earlyRowWindspeed = $earlyRow["Windspeedmph"];
            $earlyRowWindspeedMax = $earlyRow["WindspeedmphMax"];
            $earlyRowWindspeedMin = $earlyRow["WindspeedmphMin"];

            $windowSum += $earlyRowWindspeed;
            $windowRecords ++;

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
    }


    $row['WindowSpeedMax'] = $tmpWindowMax;
    $row['WindowSpeedMin'] = $tmpWindowMin;
    $row['WindowSpeedMean'] = $windowSum / $windowRecords;


    $windArray[$i] = $row;
    $i--;
}






    /* Create imagickdraw object */
    $graphics = new ImagickDraw();


    $plotLeft = $widthMargin;
    $plotRight =$width - ($widthMargin * 6);
    $plotWidth = $plotRight - $plotLeft;

    $plotTop = $heightMargin;
    $plotBottom = $height / 2;;
    $plotHeight = $plotBottom - $plotTop;

	$yPixel = $plotHeight / ($maxWindspeed * $speedMarginFactor);

	$yLowMax = $plotBottom - $siteSpeedLow * $yPixel;
	$yOnMax =  $plotBottom - $siteSpeedOn * $yPixel;
	$yMarginalMax =  $plotBottom - $siteSpeedMarginal * $yPixel;



//set up background for wind

	$graphics->setStrokeColor( "#00000000" );//fully transparent

	//Low Rectangle
	$graphics->setFillColor('SkyBlue');    // Set up some colors to use for fill and outline
	$graphics->rectangle($plotLeft, $yLowMax, $plotRight,  $plotBottom);

	//On Rectangle
	$graphics->setFillColor('green');    // Set up some colors to use for fill and outline
	$graphics->rectangle($plotLeft, $yOnMax, $plotRight,  $yLowMax);

	//Marginal Rectangle
	$graphics->setFillColor('orange');    // Set up some colors to use for fill and outline
	$graphics->rectangle($plotLeft, $yMarginalMax, $plotRight,  $yOnMax);

	//Off Rectangle
	$graphics->setFillColor('red');    // Set up some colors to use for fill and outline
	$graphics->rectangle($plotLeft, $plotTop, $plotRight,  $yMarginalMax);

//draw scale


    $graphics->setStrokeColor("#00000000" );//fully transparent
    $graphics->setStrokeWidth(0.0);
    $graphics->setFillColor('black' );//fully transparent
    $fontSize = 12.0 * $height / 480;
    $graphics->setFontSize($fontSize);
    $graphics->setFont("arial.ttf");

    $graphics->line($plotRight + 1, $plotTop, $plotRight + 1, $plotBottom );



    //mph

//    $offsetRight = 0;
//    $speedConversionFactor = 1;
//
//    $windMinorTickSize = 1;
//    for($windTick = 0; $windTick <= $maxWindspeed * $speedMarginFactor / $speedConversionFactor; $windTick += $windMinorTickSize){
//        $x1 = $offsetRight + $plotRight + 1;
//        $x2 = $offsetRight + $plotRight + 3;
//        $y = $plotBottom - $windTick * $yPixel * $speedConversionFactor;
//        $graphics->line($x1, $y, $x2, $y);
//
//    }
//
//    $windMajorTickSize = 5;
//    for($windTick = 0; $windTick <= $maxWindspeed * $speedMarginFactor / $speedConversionFactor; $windTick += $windMajorTickSize){
//        $x1 = $offsetRight + $plotRight + 1;
//        $x2 = $offsetRight+ $plotRight + 6;
//        $y = $plotBottom - $windTick * $yPixel * $speedConversionFactor;
//        $graphics->line($x1, $y, $x2, $y);
//        $graphics->annotation($x2 + 2, $y + 4, $windTick);
//    }
//
//    $windSpeedUnitName = 'mph';
//
//    $graphics->annotation($x1, $plotTop - 5, $windSpeedUnitName);

    //knt
    $offsetRight = 25;
    $speedConversionFactor = 1.15078;

    $windMinorTickSize = 1;
    for($windTick = 0; $windTick <= $maxWindspeed * $speedMarginFactor / $speedConversionFactor; $windTick += $windMinorTickSize){
        $x1 = $offsetRight + $plotRight + 1;
        $x2 = $offsetRight + $plotRight + 3;
        $y = $plotBottom - $windTick * $yPixel * $speedConversionFactor;
        $graphics->line($x1, $y, $x2, $y);

    }

    $windMajorTickSize = 5;
    for($windTick = 0; $windTick <= $maxWindspeed * $speedMarginFactor / $speedConversionFactor; $windTick += $windMajorTickSize){
        $x1 = $offsetRight + $plotRight + 1;
        $x2 = $offsetRight+ $plotRight + 6;
        $y = $plotBottom - $windTick * $yPixel * $speedConversionFactor;
        $graphics->line($x1, $y, $x2, $y);
        $graphics->annotation($x2 + 2, $y + 4, $windTick);
    }

    $windSpeedUnitName = 'knt';

    $graphics->annotation($x1, $plotTop - 5, $windSpeedUnitName);

    //kmh
    $offsetRight = 0;
    $speedConversionFactor = 0.621371;

    $windMinorTickSize = 1;
    for($windTick = 0; $windTick <= $maxWindspeed * $speedMarginFactor / $speedConversionFactor; $windTick += $windMinorTickSize){
        $x1 = $offsetRight + $plotRight + 1;
        $x2 = $offsetRight + $plotRight + 3;
        $y = $plotBottom - $windTick * $yPixel * $speedConversionFactor;
        $graphics->line($x1, $y, $x2, $y);

    }

    $windMajorTickSize = 5;
    for($windTick = 0; $windTick <= $maxWindspeed * $speedMarginFactor / $speedConversionFactor; $windTick += $windMajorTickSize){
        $x1 = $offsetRight + $plotRight + 1;
        $x2 = $offsetRight+ $plotRight + 6;
        $y = $plotBottom - $windTick * $yPixel * $speedConversionFactor;
        $graphics->line($x1, $y, $x2, $y);
        $graphics->annotation($x2 + 2, $y + 4, $windTick);
    }

    $windSpeedUnitName = 'kmh';

    $graphics->annotation($x1, $plotTop - 5, $windSpeedUnitName);




    $graphics->line($plotLeft, $plotBottom + 1, $plotRight + 1, $plotBottom + 1 );


    if($timeName == 'hours'){
        if($option_timeDisplay){


            $timeMajorTickSizeMins = 60;//minutes
            $timeMinorTickSizeMins = 10;//minutes

            $secondsSinceTick = $endTime % (60*$timeMajorTickSizeMins);
            $secondsSinceMinorTick = $endTime % (60*$timeMinorTickSizeMins);
            $lastTickTime = $endTime - $secondsSinceTick;
            $lastMinorTickTime = $endTime - $secondsSinceMinorTick;

//            $graphics->annotation(100,100, $lastTickTime);

            $y1 = $plotBottom + 1;
            $y2 = $plotBottom + 8;

            for($timeTickSec = $lastTickTime; $timeTickSec >= $beginTime; $timeTickSec -= $timeMajorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($endTime - $timeTickSec) / $timeHistory;

                $graphics->line($x, $y1, $x, $y2);

                if($timeTickSec != 0){
                    $graphics->annotation($x - 7, $y2 + 15, date('H', $timeTickSec));
                }

                
            }

            $y2 = $plotBottom + 4;

            for($timeTickSec = $lastMinorTickTime; $timeTickSec >= $beginTime; $timeTickSec -= $timeMinorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($endTime - $timeTickSec) / $timeHistory;

                $graphics->line($x, $y1, $x, $y2);

            }




        }
        else{
            $timeMinorTickSizeMins = 10;//minutes
            for($timeTickSec = 0; $timeTickSec <= $timeHistory; $timeTickSec += $timeMinorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * $timeTickSec / $timeHistory;
                $y1 =$plotBottom + 1;
                $y2 =$plotBottom + 4;
                $graphics->line($x, $y1, $x, $y2);

            }

            $timeMajorTickSizeMins = 60;//minutes
            for($timeTickSec = 0; $timeTickSec <= $timeHistory; $timeTickSec += $timeMajorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * $timeTickSec / $timeHistory;
                $y1 = $plotBottom + 1;
                $y2 = $plotBottom + 8;
                $graphics->line($x, $y1, $x, $y2);

                if($timeTickSec != 0){
                    $graphics->annotation($x - 5, $y2 + 15, -1*$timeTickSec / 60 / 60);
                }
            }
        }

    }
    else{
        if($option_timeDisplay){


            $timeMajorTickSizeMins = 5;//minutes
            $timeMinorTickSizeMins = 1;//minutes

            $secondsSinceTick = $endTime % (60*$timeMajorTickSizeMins);
            $secondsSinceMinorTick = $endTime % (60*$timeMinorTickSizeMins);
            $lastTickTime = $endTime - $secondsSinceTick;
            $lastMinorTickTime = $endTime - $secondsSinceMinorTick;

//            $graphics->annotation(100,100, $lastTickTime);

            $y1 = $plotBottom + 1;
            $y2 = $plotBottom + 8;

            for($timeTickSec = $lastTickTime; $timeTickSec >= $beginTime; $timeTickSec -= $timeMajorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($endTime - $timeTickSec) / $timeHistory;

                $graphics->line($x, $y1, $x, $y2);

                if($timeTickSec != 0){
                    $graphics->annotation($x - 15, $y2 + 15, date('H:i', $timeTickSec));
                }


            }

            $y2 = $plotBottom + 4;

            for($timeTickSec = $lastMinorTickTime; $timeTickSec >= $beginTime; $timeTickSec -= $timeMinorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($endTime - $timeTickSec) / $timeHistory;

                $graphics->line($x, $y1, $x, $y2);

            }




        }
        else{
            $timeMinorTickSizeMins = 1;//minutes
            for($timeTickSec = 0; $timeTickSec <= $timeHistory; $timeTickSec += $timeMinorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * $timeTickSec / $timeHistory;
                $y1 =$plotBottom + 1;
                $y2 =$plotBottom + 4;
                $graphics->line($x, $y1, $x, $y2);

            }

            $timeMajorTickSizeMins = 5;//minutes
            for($timeTickSec = 0; $timeTickSec <= $timeHistory; $timeTickSec += $timeMajorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * $timeTickSec / $timeHistory;
                $y1 = $plotBottom + 1;
                $y2 = $plotBottom + 8;
                $graphics->line($x, $y1, $x, $y2);

                if($timeTickSec != 0){
                    $graphics->annotation($x - 5, $y2 + 15, -1*$timeTickSec / 60);
                }
            }
        }

    }
//    if(!$option_timeDisplay){
//        $graphics->annotation($plotRight - 16, $y2 + 15, "Now");
//    }


//draw windspeed background
	$graphics->setFillColor('#ffffffd0');

	$graphics->setStrokeColor( "#00000000" );//fully transparent
	$graphics->setStrokeWidth(0.0);



	$graphics->pathStart();
	$startPath = true;
    for($i = 0; $i < count($windArray); $i++){
        $row = $windArray[$i];
		/* Get the data from the query result */
		$time   = $row["TimeMillis"] /1000;

		$windspeed = $row["Windspeedmph"];

        $x = $plotLeft + $plotWidth * ($time - $beginTime)/($timeHistory);
        $y = $plotBottom- $windspeed * $yPixel;
        if($startPath){
            $graphics->pathMoveToAbsolute($plotLeft,$plotTop);
            $graphics->pathLineToAbsolute($plotLeft,$y);
            $graphics->pathLineToAbsolute($x,$y);
            $startPath = false;

        }
        else{

            $graphics->pathLineToAbsolute($x,$y);

        }
	}


	$graphics->pathLineToAbsolute($plotRight,$y);
	$graphics->pathLineToAbsolute($plotRight,$plotTop);
	$graphics->pathClose();
	$graphics->pathFinish();

//drawWindspeed Markers

    $graphics->setFillColor('black' );
 
    $graphics->setStrokeColor( "#00000000" );//fully transparent
    $graphics->setStrokeWidth(0.0);

    for($i = 0; $i < count($windArray); $i++){
        $row = $windArray[$i];
        /* Get the data from the query result */
        $time   = $row["TimeMillis"] / 1000;

        $windspeed = $row["Windspeedmph"];

        $windspeedMax = $row["WindspeedmphMax"];
        $windspeedMin = $row["WindspeedmphMin"];

        $x = $plotLeft + $plotWidth * ($time - $beginTime)/($timeHistory);
        $y = $plotBottom - $windspeed * $yPixel;
        if($option_errorbars){
            $graphics->setStrokeColor('black');
            $graphics->setStrokeWidth(1.0);
            $graphics->line($x, $plotBottom - $windspeedMax * $yPixel, $x, $plotBottom - $windspeedMin * $yPixel);
            $graphics->line($x - $option_pointSize, $plotBottom - $windspeedMax * $yPixel, $x + $option_pointSize, $plotBottom - $windspeedMax * $yPixel);
            $graphics->line($x - $option_pointSize, $plotBottom - $windspeedMin * $yPixel, $x + $option_pointSize, $plotBottom - $windspeedMin * $yPixel);
        }
        if($option_markers){
            $graphics->setStrokeColor( "#00000000" );//fully transparent
            $graphics->setStrokeWidth(0.0);
            $graphics->circle($x, $y, $x + $option_pointSize, $y + $option_pointSize);
        }


    }

    if(count($windArray) > 0){
    
    
    //draw MovingAverage line
        if($option_average){
    
    
            $graphics->setFillColor("#00000000" );//fully transparent
            $graphics->setStrokeColor("#8B4513");
         
            $graphics->setStrokeWidth(2.0);
    
    
            $graphics->pathStart();
            $startPath = true;
            for($i = 0; $i < count($windArray); $i++){
                $row = $windArray[$i];
    
                /* Get the data from the query result */
                $time  = $row["TimeMillis"]/1000;
                $windspeed = $row["Windspeedmph"];
                $windowSpeedMean = $row["WindowSpeedMean"];
    //            $windowSpeedMean = 30;
    
    
                $x = $plotLeft + $plotWidth  * ($time - $beginTime)/($timeHistory);
                $y = $plotBottom - $windowSpeedMean * $yPixel;
    
                if($time >=  $beginTime + $window){
                    if($startPath){
                        $graphics->pathMoveToAbsolute($x,$y);
                        $startPath = false;
                    }
                    else{
                        $graphics->pathLineToAbsolute($x,$y);
                    }
                }
    
            }
            $graphics->pathFinish();
        }
    
    //drawMovingMax line
        if($option_max){
    
    
            $graphics->setFillColor("#00000000" );//fully transparent
            $graphics->setStrokeColor("#FF00FF");
        
            $graphics->setStrokeWidth(2.0);
    
            $graphics->pathStart();
            $startPath = true;
    
            for($i = 0; $i < count($windArray); $i++){
                $row = $windArray[$i];
    
                $time   = $row["TimeMillis"] /1000;
                $windowSpeedMax = $row["WindowSpeedMax"];
                $windspeed = $row["Windspeedmph"];
    //            $windStdDev = $row["WindowSpeedStdDev"];
                $windowSpeedMean = $row["WindowSpeedMean"];
                $max;
    //            if($option_bol){
    //                $max = $windowSpeedMean + ($windStdDev * 2.0);
    //            }
    //            else{
                    $max = $windowSpeedMax;
    //            }
    
    
                $x = $plotLeft + $plotWidth  * ($time - $beginTime)/($timeHistory);
    
                $y = $plotBottom - $max * $yPixel;
    
                if($time >=   $beginTime + $window ){
                    if($startPath){
                        $graphics->pathMoveToAbsolute($x,$y);
                        $startPath = false;
                    }
                    else{
                        $graphics->pathLineToAbsolute($x,$y);
                    }
                }
            }
            $graphics->pathFinish();
        }
    
    //drawMovingMin line
        if($option_min){
    
    
            $graphics->setFillColor("#00000000" );//fully transparent
            $graphics->setStrokeColor("blue");//fully transparent
    
            $graphics->setStrokeWidth(2.0);
    
            $graphics->pathStart();
            $startPath = true;
    
            for($i = 0; $i < count($windArray); $i++){
                $row = $windArray[$i];
    
                $time   = $row["TimeMillis"]/1000;
                $windowSpeedMin = $row["WindowSpeedMin"];
                $windspeed = $row["Windspeedmph"];
    //            $windStdDev = $row["WindowSpeedStdDev"];
                $windowSpeedMean = $row["WindowSpeedMean"];
    
                $min;
    //            if($option_bol){
    //                $min = $windowSpeedMean - ($windStdDev * 2.0);
    //            }
    //            else{
                    $min = $windowSpeedMin;
    //            }
    
    
    
                if($min < 0){
                    $min = 0;
                }
    
                $x = $plotLeft + $plotWidth * ($time - $beginTime)/($timeHistory);
                $y = $plotBottom - $min * $yPixel;
    
                if($time >=  $beginTime + $window){
                    if($startPath){
                        $graphics->pathMoveToAbsolute($x,$y);
                        $startPath = false;
                    }
                    else{
                        $graphics->pathLineToAbsolute($x,$y);
                    }
                }
            }
            $graphics->pathFinish();
        }
    }

//draw dirs


    $plotDirTop = $plotBottom + $heightMargin * 1.5;
    $plotDirBottom = $height - $heightMargin;
    $plotDirHeight = $plotDirBottom - $plotDirTop;

    //set up background for windDir

    $graphics->setStrokeColor( "#00000000" );//fully transparent

    //Dir Rectangle
    $graphics->setFillColor(new ImagickPixel("#8B4726"));    // Set up some colors to use for fill and outline
    $graphics->rectangle($plotLeft, $plotDirTop, $plotRight,  $plotDirBottom);

    //Dir On Rectangle
    $graphics->setFillColor('green');    // Set up some colors to use for fill and outline
    $graphics->rectangle($plotLeft, dirToPixel($siteDirOn-$siteDirWidth), $plotRight,  dirToPixel($siteDirOn+$siteDirWidth));


//draw dir scale


    $graphics->setStrokeColor("#00000000" );//fully transparent
    $graphics->setStrokeWidth(0.0);
    $graphics->setFillColor('black' );//fully transparent
    $fontSize = 12.0 * $height / 480;
    $graphics->setFontSize($fontSize);
    $graphics->setFont("arial.ttf");

    $graphics->line($plotRight + 1, $plotDirTop, $plotRight + 1, $plotDirBottom );

    $dirTickSize = 1;
    $dirs = array("N", "NE" , "E", "SE", "S", "SW", "W", "NW");
    $offsetRight = 35;
    for($i = 0; $i < count($dirs); $i++){
        $y = dirToPixel($i * 45);
        $x1 = $plotRight + 1;
        $x2 = $plotRight + 6;
        $graphics->line($x1, $y, $x2, $y);
        $graphics->annotation($x2 + 2, $y + 4, $dirs[$i]);

//        $x1 = $offsetRight + $plotRight + 1;
//        $x2 = $offsetRight + $plotRight + 6;
//        $graphics->line($x1, $y, $x2, $y);
//        $graphics->annotation($x2 + 2, $y + 4, $i * 45);

    }

//drawDir Markers

    $graphics->setFillColor(new ImagickPixel("yellow"));

	$graphics->setStrokeColor( "#00000000" );//fully transparent
	$graphics->setStrokeWidth(0.0);

    for($i = 0; $i < count($windArray); $i++){
        $row = $windArray[$i];
        /* Get the data from the query result */
        $time   = $row["TimeMillis"]/1000;

        $windDir = $row["Winddir"];

        $x = $plotLeft + $plotWidth * ($time - $beginTime)/($timeHistory);
        $y = dirToPixel($windDir);

        $graphics->ellipse($x, $y, $option_pointSize, $option_pointSize, 0, 360);
    }



    /* Create a new canvas object and a white image */
    $image= new Imagick();
    $image->newImage($width, $height, "white");

//draw Time


	$graphics->setFillColor('black');

	$graphics->setStrokeColor( "#00000000" );//fully transparent
	$graphics->setStrokeWidth(0.0);

	$formattedDate = date('l j M Y', $beginTime);
    $fontSize = 16.0 * $height / 480.0;
	$graphics->setFontSize($fontSize);

    $graphics->annotation($plotLeft, $plotTop - 5, $site_name . " " . $formattedDate);


// //draw bounding Rectangle
    $graphics->setFillColor(new ImagickPixel("#00000000"));

    $graphics->setStrokeColor(new ImagickPixel("grey"));
    $graphics->setStrokeWidth(1.0);
    $graphics->rectangle(0,0,$width-1, $height-1);


	/* Draw the ImagickDraw on to the canvas */
	$image->drawImage($graphics);


    /* Set the format to PNG */
    $image->setImageFormat('png');

    /* Output the image */
    header("Content-Type: image/png");


    echo $image;

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

    function dirToPixel($dir){  //must be in range 0 to 360;
        global $siteDirOn;
        global $plotDirTop;
        global $plotDirHeight;
        
        $dirTop = normaliseDir($siteDirOn - 180);
        
        if($dir < $dirTop){
        	$dir +=360;
        }
        $prop = ($dir - $dirTop)/360.0;


        return $plotDirTop + $prop * $plotDirHeight;

    }

    function normaliseDir($dir){
        $dir = $dir % 360;
        if($dir < 0){
            $dir += 360;
        }
        return $dir;
    }

    function setVarFromIndex($index, $default){

        if(isset($_GET[$index])) {
            return $_GET[$index];
        }
        else{
            return $default;
        }
    }
?>