<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>FreeFlight Wx</title>

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>
    <!-- Include all compiled plugins (below), or include individual files as needed -->
    <script src="../js/bootstrap.min.js"></script>
    <script src="../bootstrap-datepicker/js/bootstrap-datepicker.min.js"></script>
    <script src="https://gitcdn.github.io/bootstrap-toggle/2.1.0/js/bootstrap-toggle.min.js"></script>

    <!-- Bootstrap -->
    <link href="../css/bootstrap.min.css" rel="stylesheet">
    <link href="https://gitcdn.github.io/bootstrap-toggle/2.1.0/css/bootstrap-toggle.min.css" rel="stylesheet">
    <link id="bsdp-css" href="../bootstrap-datepicker/css/bootstrap-datepicker3.min.css" rel="stylesheet">

    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
    <script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"></script>
    <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
    <![endif]-->
</head>
<body>




<div class="container">
<!-- Static navbar -->


<nav class="navbar navbar-default navbar-static-top">
    <div class="container">
        <div class="navbar-header">
            <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">
                <span class="sr-only">Toggle navigation</span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
            </button>
            <a class="navbar-brand" href="../">Home</a>
        </div>
        <div id="navbar" class="navbar-collapse collapse">
            <ul class="nav navbar-nav">
                <li id="li_index"><a href="index.php">Current</a></li>
                <!--<li id="li_15mins"><a href="15mins.php">15 Mins</a></li>-->
                <li id="li_1hour"><a href="1hour.php">1 Hour</a></li>
                <li id="li_4hours"><a href="4hours.php">4 Hours</a></li>
                <li id="li_12hours"><a href="12hours.php">12 Hours</a></li>
                <li id="li_day"><a href="day.php">Day</a></li>
                <li id="li_table"><a href="table.php?h=24">Table</a></li>
            </ul>
        </div><!--/.nav-collapse -->
    </div><!--/.container-fluid -->
</nav>

<script>
    function GetCurrentPageName() {
        //method to get Current page name from url.
        var PageURL = document.location.href;
        var PageName = PageURL.substring(PageURL.lastIndexOf('/') + 1);

        return PageName.toLowerCase() ;
    }



    $(document).ready(function(){


        var CurrPage = GetCurrentPageName();

        switch(CurrPage){
            case 'index.php':
                $('#li_index').addClass('active') ;
                break;
            case '15mins.php':
                $('#li_15mins').addClass('active') ;
                break;
            case '1hour.php':
                $('#li_1hour').addClass('active') ;
                break;
            case '4hours.php':
                $('#li_4hours').addClass('active') ;
                break;
            case '12hours.php':
                $('#li_12hours').addClass('active') ;
                break;
            case 'day.php':
                $('#li_day').addClass('active') ;
                break;
            case 'table.php?h=1':
                $('#table').addClass('active') ;
                break;
        }


    });


</script>


<h3><?php
include 'sitedetails.php';
echo $site_name;
?> Weather Station</h3>
</div>

