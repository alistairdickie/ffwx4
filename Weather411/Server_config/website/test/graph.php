<?php
require_once ("../jpgraph/src/jpgraph.php");   
require_once ("../jpgraph/src/jpgraph_line.php");
require_once ("../jpgraph/src/jpgraph_date.php");


//time params
$hours = $_GET["h"];
$param = $_GET["param"];
if($hours > 240){
    $hours = 240;
}
$timeHistory = $hours*3600;//in hours converted to seconds
$window = 600;

include 'dataprocessor.php';

//param options

//$row["Windspeedmph"];
//$row["WindspeedmphMax"];
//$row["WindspeedmphMin"];
//$row["Winddir"];
//$row["QNH"];
//$row["Tempc"];
//$row["Humidity"];
//$row["Battery"];
//$row['DewPoint'];
//$row['CloudbaseAGLft'];
//$row['WindowSpeedMax'];
//$row['WindowSpeedMin'];
//$row['WindowSpeedAverage'];
//$row['WindowTemp'];
//$row['WindowHumidity'];

$xdata = array();
$ydata = array();


for($i = 0; $i < count($windArray); $i++){
    $row = $windArray[$i];
   
    $xdata[$i] = $row["TimeSeconds"];
    $ydata[$i] = $row[$param];
}


/*
Let's create a Graph instance and set some variables (width, height, cache filename, cache timeout). If the last argument "inline" is true the image is streamed back to the browser, otherwise it's only created in the cache.
*/ 
$graph = new Graph(1200, 800);//, 'auto', 10, true); 

$current_time = microtime(true);

// Setting what axises to use
$graph->SetScale('datlin',0,0,$current_time - $timeHistory, $current_time); 

/*
Next, we need to create a LinePlot with some example parameters.
*/ 
$lineplot = new LinePlot($ydata, $xdata); 

// Setting the LinePlot color
//$lineplot->SetColor('forestgreen'); 

// Adding LinePlot to graphic 
$graph->Add($lineplot); 

// Giving graphic a name
$graph->title->Set('Weather Station Data (last ' . $hours.' hours)' ); 


// Naming axises 
$graph->xaxis->title->Set('Time'); 
$graph->yaxis->title->Set($param); 

// Set the angle for the labels to 90 degrees
$graph->xaxis->SetLabelAngle(90);

$graph->SetMargin(80,40,30,130);

// Coloring axises
//$graph->xaxis->SetColor('#СС0000'); 
//$graph->yaxis->SetColor('#СС0000'); 

// // Setting the LinePlot width 
// $lineplot->SetWeight(3); 

//  $lineplot->mark->SetType(MARK_FILLEDCIRCLE); 


// Adding a shadow
// $graph->SetShadow(4); 

/* 
Showing image in browser. If, when creating an graph object, the last parameter is false, the image would be saved in cache and not showed in browser.
*/  
  
$graph->Stroke(); 


?>