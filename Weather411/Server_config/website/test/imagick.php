<?php
system("/usr/bin/convert -version");
?>

<?php
$IM_version=shell_exec("/usr/bin/convert -version");
echo $IM_version
?>

<?php
exec("/usr/bin/convert -version",$out,$returnval);
print_r($out[0]);
?>
