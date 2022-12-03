<?php


//time params
$hours = $_GET["h"];
if($hours > 240){
    $hours = 240;
}
$timeHistory = $hours*3600;//in hours converted to seconds
$window = 600;


include 'dataprocessor.php';


echo '<table cellpadding="4" cellspacing="1" border="1" class="db-table">';

echo '<tr>
<th>Index</th>
<th>Time</th>
<th><a href="./graph.php?h='.$hours.'&param=Windspeedmph">Avg</a></th>
<th><a href="./graph.php?h='.$hours.'&param=WindspeedmphMax">Max</a></th>
<th><a href="./graph.php?h='.$hours.'&param=WindspeedmphMin">Min</a></th>
<th><a href="./graph.php?h='.$hours.'&param=Winddir">Dir</a></th>
<th><a href="./graph.php?h='.$hours.'&param=QNH">Pressure</a></th>
<th><a href="./graph.php?h='.$hours.'&param=realQNH">QNH</a></th>
<th><a href="./graph.php?h='.$hours.'&param=Tempc">Temp</a></th>
<th><a href="./graph.php?h='.$hours.'&param=Humidity">Humidity</a></th>
<th><a href="./graph.php?h='.$hours.'&param=DewPoint">Dew Point</a></th>
<th><a href="./graph.php?h='.$hours.'&param=CloudbaseAGLft">Cloudbase AMSL</a></th>
<th><a href="./graph.php?h='.$hours.'&param=Battery">Battery</a></th>
<th><a href="./graph.php?h='.$hours.'&param=CSQ">CSQ</a></th>
</tr>';

for($i = count($windArray) - 1; $i >= 0; $i--){
    $row = $windArray[$i];
    /* Get the data from the query result */
    $index   = $row["PowerOnIndex"];
    $time   = $row["TimeMillis"];
    $windspeed = $row["Windspeedmph"];
    $windspeedMax = $row["WindspeedmphMax"];
    $windspeedMin = $row["WindspeedmphMin"];
    $winddir    = $row["Winddir"];
    $pressure    = $row["QNH"];
    $temp    = $row["Tempc"];
    $humidity    = $row["Humidity"];
    $battery    = $row["Battery"];
    $csq    = $row["CSQ"];
    $realQNH    = $row["realQNH"];
    $dewpoint   = round($row['DewPoint'], 1);
    $cloudbaseft = round($row['CloudbaseAGLft']) + $siteAltitude;
    $cloudbasem = round($cloudbaseft * 0.3048);

    echo '<tr>';
    echo '<td>',$index,'</td>';
    echo '<td>',date('H:i:s', $time/1000),'</td>';
    echo '<td>',$windspeed,'</td>';
    echo '<td>',$windspeedMax,'</td>';
    echo '<td>',$windspeedMin,'</td>';
    echo '<td>',getStringDir($winddir). " (" . $winddir . ")",'</td>';

    echo '<td>',$pressure,'</td>';
    echo '<td>',$realQNH,'</td>';
    echo '<td>',$temp,'</td>';
    echo '<td>',$humidity,'</td>';
    echo '<td>',$dewpoint,'</td>';
    echo '<td>',$cloudbaseft . 'ft ('. $cloudbasem .'m)','</td>';
    echo '<td>',$battery,'</td>';
    echo '<td>',$csq,'</td>';

    echo '</tr>';

}

echo '</table><br />';

?>

