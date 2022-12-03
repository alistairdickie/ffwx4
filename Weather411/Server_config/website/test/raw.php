<?php

    include 'sitedetails.php';

    $mysqli = new mysqli($server, $username, $password, $database);

    if ($mysqli->connect_errno) {
        echo "Failed to connect to MySQL: " . $mysqli->connect_error;
    }


    $maxRecords = $_GET["r"];

    $sql = "SELECT * FROM `$data_table` ORDER BY id DESC LIMIT " . $maxRecords ;

	$result= $mysqli->query($sql);

if (!$result) die('Couldn\'t fetch records');
//$num_fields = mysql_num_fields($result);
//$headers = array();
//for ($i = 0; $i < $num_fields; $i++) {
//    $headers[] = mysql_field_name($result , $i);
//}
$fp = fopen('php://output', 'w');
if ($fp && $result) {
    header('Content-Type: text/csv');
    header('Content-Disposition: attachment; filename="export.csv"');
    header('Pragma: no-cache');
    header('Expires: 0');
    fputcsv($fp, array('id', 'timestamp',  'Station', 'PowerOnIndex', 'Windspeedmph', 'WindspeedmphMax', 'WindspeedmphMin','Winddir', 'QNH', 'Tempc', 'Humidity', 'Battery','TimeMillis', 'CSQ'));
    while ($row = $result->fetch_array(MYSQLI_NUM)) {
        fputcsv($fp, array_values($row));
    }
    die;
}



?>