<?php

    include 'sitedetails.php';

	$mysqli = new mysqli($server, $username, $password, $database);

	if ($mysqli->connect_errno) {
	    echo "Failed to connect to MySQL: " . $mysqli->connect_error;
	}

    //time params

    $timeHistory = setVarFromIndex("time", 3600);
    $window = setVarFromIndex("window",600);

    //overall image params
    $width = setVarFromIndex("width", 780);
    $height = setVarFromIndex("height", 500);

    //display params

    $option_errorbars = setVarFromIndex("error", 1);
    $option_max = setVarFromIndex("max", 1);
    $option_min =setVarFromIndex("min", 1);
    $option_markers = setVarFromIndex("markers", 0);
    $option_average = setVarFromIndex("average", 1);
    $option_pointSize = setVarFromIndex("point", 1);
    $option_timeDisplay = setVarFromIndex("timeDisplay", true);
    $option_nDir = setVarFromIndex("nDir", false);


    $widthMargin = 10;
    $heightMargin = 20;
    $speedMarginFactor = 1.1;



	$maxRecords = (int)($timeHistory / ($station_PERIOD * $station_AVG_NUM)) + 1;


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



    date_default_timezone_set($site_timezone);


	$currentTime = round(microtime(true) * 1000); //current time in milliseconds since the epoch
	$earlyTime = $currentTime - ($timeHistory * 1000);//timeHistory seconds earlier than the current time in milliseconds since the epoch


	$sql = "SELECT * FROM ( SELECT * FROM `$data_table` ORDER BY id DESC LIMIT " . $maxRecords . " ) sub WHERE `TimeMillis` >= " . $earlyTime . " ORDER by id ASC";

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

//first - preprocess the data to get some parameters and put the data into windArray

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
    $earlyWindspeedArray = array();
    array_push($earlyWindspeedArray, $windspeed);//include the current one

    $j = $i;
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
    }


    $row['WindowSpeedMax'] = $tmpWindowMax;
    $row['WindowSpeedMin'] = $tmpWindowMin;
    $row['WindowSpeedStdDev'] = stats_standard_deviation($earlyWindspeedArray);
    $row['WindowSpeedMean'] = stats_mean($earlyWindspeedArray);


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
    $graphics->setFillColor('black' );
    $fontSize = 12.0 * $height / 480;
  
    $graphics->setFontSize($fontSize);

    $graphics->setFont("arial.ttf");



    $graphics->line($plotRight + 1, $plotTop, $plotRight + 1, $plotBottom );



//mph scale

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

//knt scale
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

//kmh scale
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
            $currentTimeS = $currentTime / 1000;
            $earlyTimeS = $earlyTime / 1000;

            $timeMajorTickSizeMins = 60;//minutes
            $timeMinorTickSizeMins = 10;//minutes

            $secondsSinceTick = $currentTimeS % (60*$timeMajorTickSizeMins);
            $secondsSinceMinorTick = $currentTimeS % (60*$timeMinorTickSizeMins);
            $lastTickTime = $currentTimeS - $secondsSinceTick;
            $lastMinorTickTime = $currentTimeS - $secondsSinceMinorTick;

            $y1 = $plotBottom + 1;
            $y2 = $plotBottom + 8;

            for($timeTickSec = $lastTickTime; $timeTickSec >= $earlyTimeS; $timeTickSec -= $timeMajorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($currentTimeS - $timeTickSec) / $timeHistory;

                $graphics->line($x, $y1, $x, $y2);

                if($timeTickSec != 0){
                    $graphics->annotation($x - 7, $y2 + 15, date('H', $timeTickSec));
                }

                
            }

            $y2 = $plotBottom + 4;

            for($timeTickSec = $lastMinorTickTime; $timeTickSec >= $earlyTimeS; $timeTickSec -= $timeMinorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($currentTimeS - $timeTickSec) / $timeHistory;

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
            $currentTimeS = $currentTime / 1000;
            $earlyTimeS = $earlyTime / 1000;

            $timeMajorTickSizeMins = 5;//minutes
            $timeMinorTickSizeMins = 1;//minutes

            $secondsSinceTick = $currentTimeS % (60*$timeMajorTickSizeMins);
            $secondsSinceMinorTick = $currentTimeS % (60*$timeMinorTickSizeMins);
            $lastTickTime = $currentTimeS - $secondsSinceTick;
            $lastMinorTickTime = $currentTimeS - $secondsSinceMinorTick;
            $y1 = $plotBottom + 1;
            $y2 = $plotBottom + 8;

            for($timeTickSec = $lastTickTime; $timeTickSec >= $earlyTimeS; $timeTickSec -= $timeMajorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($currentTimeS - $timeTickSec) / $timeHistory;

                $graphics->line($x, $y1, $x, $y2);

                if($timeTickSec != 0){
                    $graphics->annotation($x - 15, $y2 + 15, date('H:i', $timeTickSec));
                }


            }

            $y2 = $plotBottom + 4;

            for($timeTickSec = $lastMinorTickTime; $timeTickSec >= $earlyTimeS; $timeTickSec -= $timeMinorTickSizeMins * 60){
                $x = $plotRight - $plotWidth * ($currentTimeS - $timeTickSec) / $timeHistory;

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
    if(!$option_timeDisplay){
        $graphics->annotation($plotRight - 16, $y2 + 15, "Now");
    }


//draw windspeed background
   
	$graphics->setFillColor('#ffffffd0');
	$graphics->setStrokeColor( "#00000000" );//fully transparent
	$graphics->setStrokeWidth(0.0);



	$graphics->pathStart();
	$startPath = true;
    for($i = 0; $i < count($windArray); $i++){
        $row = $windArray[$i];
		/* Get the data from the query result */
		$time   = $row["TimeMillis"];

		$windspeed = $row["Windspeedmph"];





        $x = $plotLeft + $plotWidth * ($time - $earlyTime)/($currentTime - $earlyTime);
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

    $graphics->setFillColor('#00000080' );
    $graphics->setStrokeColor( "#00000000" );//fully transparent
    $graphics->setStrokeWidth(0.0);

    for($i = 0; $i < count($windArray); $i++){
        $row = $windArray[$i];
        /* Get the data from the query result */
        $time   = $row["TimeMillis"];

        $windspeed = $row["Windspeedmph"];

        $windspeedMax = $row["WindspeedmphMax"];
        $windspeedMin = $row["WindspeedmphMin"];

        $x = $plotLeft + $plotWidth * ($time - $earlyTime)/($currentTime - $earlyTime);
        $y = $plotBottom - $windspeed * $yPixel;
        if($option_errorbars){
            $graphics->setStrokeColor('#00000080');
            $graphics->setStrokeWidth(1.5);
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


     
        $graphics->setFillColor("#00000000");//fully transparent
        
        
        


//draw MovingAverage line
    if($option_average){

      
        
        $graphics->setStrokeColor('#a4610fff');
        $graphics->setStrokeWidth(3.0);
        
        
        
        $startPath = true;
        for($i = 0; $i < count($windArray); $i++){
            $row = $windArray[$i];

            /* Get the data from the query result */
            $time   = $row["TimeMillis"];
            $windspeed = $row["Windspeedmph"];
            $windowSpeedMean = $row["WindowSpeedMean"];


            $x = $plotLeft + $plotWidth  * ($time - $earlyTime)/($currentTime - $earlyTime);
            $y = $plotBottom - $windowSpeedMean * $yPixel;
            if($time >=  $earlyTime + $windowTimeMillis ){
                if($startPath){
                    $graphics->pathStart();
                    $graphics->pathMoveToAbsolute($x,$y);
                    $startPath = false;
                }
                else{
                    $graphics->pathLineToAbsolute($x,$y);
                }
            }

        }
        if(!$startPath){
            $graphics->pathFinish();
        }
        
    }

//drawMovingMax line
    if($option_max){


  
        $graphics->setStrokeColor('#ff00ff80');
        $graphics->setStrokeWidth(2.0);
     

        
        $startPath = true;

        for($i = 0; $i < count($windArray); $i++){
            $row = $windArray[$i];

            $time   = $row["TimeMillis"];
            $windowSpeedMax = $row["WindowSpeedMax"];
            $windspeed = $row["Windspeedmph"];
            $windowSpeedMean = $row["WindowSpeedMean"];
            $max;

                $max = $windowSpeedMax;



            $x = $plotLeft + $plotWidth  * ($time - $earlyTime)/($currentTime - $earlyTime);

            $y = $plotBottom - $max * $yPixel;

            if($time >=  $earlyTime + $windowTimeMillis ){
                if($startPath){
                    $graphics->pathStart();
                    $graphics->pathMoveToAbsolute($x,$y);
                    $startPath = false;
                }
                else{
                    $graphics->pathLineToAbsolute($x,$y);
                }
            }
        }
        if(!$startPath){
            $graphics->pathFinish();
        }
    }

//drawMovingMin line
    if($option_min){



        $graphics->setStrokeColor('#0000ff80');
        $graphics->setStrokeWidth(2.0);


        
        $startPath = true;

        for($i = 0; $i < count($windArray); $i++){
            $row = $windArray[$i];

            $time   = $row["TimeMillis"];
            $windowSpeedMin = $row["WindowSpeedMin"];
            $windspeed = $row["Windspeedmph"];
            $windowSpeedMean = $row["WindowSpeedMean"];

            $min = $windowSpeedMin;

            if($min < 0){
                $min = 0;
            }

            $x = $plotLeft + $plotWidth * ($time - $earlyTime)/($currentTime - $earlyTime);
            $y = $plotBottom - $min * $yPixel;

            if($time >=  $earlyTime + $windowTimeMillis ){
                if($startPath){
                    $graphics->pathStart();
                    $graphics->pathMoveToAbsolute($x,$y);
                    $startPath = false;
                }
                else{
                    $graphics->pathLineToAbsolute($x,$y);
                }
            }
        }
        if(!$startPath){
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

    
    if($option_nDir){
          $dirTop = 0; 
    }
    else if(isset($siteDirOnCentre)){
      $dirTop = normaliseDir($siteDirOnCentre - 180); 
    }
    else{
        $dirTop = normaliseDir($siteDirOn - 180);
        
    }
  
    
 
    if(isset($siteDirOnCentre)){
        if($siteDirOn1-$siteDirWidth1 < 0 or $siteDirOn1+$siteDirWidth1 > 360){
            $graphics->rectangle($plotLeft, dirToPixel(0), $plotRight,  dirToPixel(normaliseDir($siteDirOn1+$siteDirWidth1)));
            
            $graphics->rectangle($plotLeft, dirToPixel(normaliseDir($siteDirOn1-$siteDirWidth1)), $plotRight,  dirToPixel(360));
        }
        else{
            $graphics->rectangle($plotLeft, dirToPixel($siteDirOn1-$siteDirWidth1), $plotRight,  dirToPixel($siteDirOn1+$siteDirWidth1));
        }
        
        if($siteDirOn2-$siteDirWidth2 < 0 or $siteDirOn2+$siteDirWidth2 > 360){
            $graphics->rectangle($plotLeft, dirToPixel(0), $plotRight,  dirToPixel(normaliseDir($siteDirOn2+$siteDirWidth2)));
            
            $graphics->rectangle($plotLeft, dirToPixel(normaliseDir($siteDirOn2-$siteDirWidth2)), $plotRight,  dirToPixel(360));
        }
        else{
            $graphics->rectangle($plotLeft, dirToPixel($siteDirOn2-$siteDirWidth2), $plotRight,  dirToPixel($siteDirOn2+$siteDirWidth2));
        }
    }
    else{
        if($siteDirOn-$siteDirWidth < 0 or $siteDirOn+$siteDirWidth > 360){
            $graphics->rectangle($plotLeft, dirToPixel(0), $plotRight, dirToPixel(normaliseDir($siteDirOn+$siteDirWidth)));
            
            $graphics->rectangle($plotLeft, dirToPixel($siteDirOn-$siteDirWidth), $plotRight,  dirToPixel(360));
        }
        else{
            $graphics->rectangle($plotLeft, dirToPixel($siteDirOn-$siteDirWidth), $plotRight,  dirToPixel(normaliseDir($siteDirOn+$siteDirWidth)));
        }
        
    }
   
    


//draw dir scale

    $graphics->setFillColor('black' );//fully transparent
    $fontSize = 12.0 * $height / 480;
    $graphics->setFontSize($fontSize);

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
    }

//drawDir Markers

    $graphics->setFillColor('yellow');
	$graphics->setStrokeColor( "#00000000" );//fully transparent
	$graphics->setStrokeWidth(0.0);

    for($i = 0; $i < count($windArray); $i++){
        $row = $windArray[$i];
        /* Get the data from the query result */
        $time   = $row["TimeMillis"];

        $windDir = $row["Winddir"];

        $x = $plotLeft + $plotWidth * ($time - $earlyTime)/($currentTime - $earlyTime);
        $y = dirToPixel($windDir);

        $graphics->ellipse($x, $y, $option_pointSize, $option_pointSize, 0, 360);
      
    }



    /* Create a new canvas object and a white image */
    $image= new Imagick();
    $image->newImage($width, $height, "white");

//draw Time


 	$graphics->setFillColor('#000000c0');
 	$graphics->setStrokeColor( "#00000000" );//fully transparent
 	$graphics->setStrokeWidth(0.0);

 	
    $fontSize = 16.0 * $height / 480.0;
 	$graphics->setFontSize($fontSize);
 	$graphics->annotation($plotLeft, $plotTop - 5, $site_name . ": last " . $timeNum . " " . $timeName);

    $formattedDate = date('l j M Y H:i:s');
    $metrics = $image->queryFontMetrics ($graphics, $formattedDate . " ");
    $textwidth = $metrics['textWidth'];
    $graphics->annotation ($plotRight - $textwidth, $plotTop - 5, $formattedDate);

//draw bounding Rectangle
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
        global $dirTop;
        global $plotDirTop;
        global $plotDirHeight;
      
        // $d = normaliseDir($dir);
        $d = $dir;
        
        if($d < $dirTop){
        	$d +=360;
        }
        $prop = ($d - $dirTop)/360.0;


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