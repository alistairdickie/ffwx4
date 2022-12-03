<?php include 'header.php'; ?>

<div class="container">
    <div class="checkbox">
        <label>
            <input id="toggle" type="checkbox" checked data-toggle="toggle" data-size="mini" data-onstyle="info" data-offstyle="warning">
            Auto refresh (every 3 seconds)
        </label>
    </div>
</div>

<div class="container">
    <img SRC='windgraph.php?
    time=600&
    window=600&
    width=780&
    height=500&
    max=0&
    min=0&
    markers=1&
    error=1&
    average=0&
    point=3&
    timeDisplay=true&
    rand=<?=Date('U')?>'
         id="graph">'
         
</div>


<script type="text/javascript">

    var myVar;

    var count;

    function stopRefresh() {
        clearInterval(myVar);
    }

    function startRefresh() {
        myVar = setInterval(function(){ updateImage() }, 1000);
        count = 60;
    }

    function updateImage() {
        var image = document.getElementById("graph");
        image.src = './windgraph.php?time=600&window=600&width=780&height=500&max=0&min=0&bol=0&markers=1&error=1&average=0&point=3&timeDisplay=true&rand=' + Math.random();
        count--;
        if(count <= 0){
            toggleOff();

        }
    }

    function toggleOff() {
        $('#toggle').prop('checked', false).change()
    }

    window.onload = function() {
        startRefresh();

    }

    $(function() {
        $('#toggle').change(function(){

            if($('#toggle').is(':checked')) {
                updateImage();
                startRefresh();
            }
            else {
                stopRefresh();
            }


        });
    })

</script>


<?php include 'footer.php'; ?>

