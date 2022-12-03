<script>

    $(document).ready(function(){


        $('#dp1').datepicker({
            format: "d M yyyy",
            todayBtn: "linked",
            autoclose: true,
            todayHighlight: true

        });

        $('#dp1').datepicker()
            .on("changeDate", function(e) {
                updateDP1Image(e.date);
            });

    });

    function updateDP1Image(adate) {
        var image = document.getElementById("imagedp1");


        var bd = pad(adate.getDate() +"");
        var bm = pad(adate.getMonth() + 1 + "");
        var begin = adate.getFullYear() + "" + bm + bd;

        var endms = adate.getTime() + 86400000;
        var d = new Date(endms);
        var dd = pad(d.getDate() +"");
        var dm = pad(d.getMonth() + 1 + "" )

        var end = d.getFullYear() + "" + dm + dd;


        image.src = 'windgraph2.php?begin=' +
             begin +
            '&end=' +
            end +
            '&window=600&width=1200&height=500&max=1&min=1&markers=0&error=0&average=1&point=1.0&timeDisplay=true&rand=' + Math.random();

    }

    function pad(str) {
        if (str.length < 2){
            str = "0" + str;
        }
        return str;
    }


</script>

<div class="container">
    <div class="row">
        <div class="col-md-2">

            <div class="input-group date" data-provide="datepicker" id="dp1">
                <input type="text" class="form-control" value="Today">

                <div class="input-group-addon">
                    <span class="glyphicon glyphicon-th"></span>
                </div>
            </div>
        </div>

    </div>

</div>

<div class="container">
    <img SRC='windgraph2.php?
    begin=today&
    end=tomorrow&
    window=600&
    width=1200&
    height=500&
    max=1&
    min=1&
    markers=0&
    error=0&
    average=1&
    point=1.0&
    timeDisplay=true&
    rand=<?=Date('U')?>'
         id="imagedp1">

</div> <!-- /container -->



