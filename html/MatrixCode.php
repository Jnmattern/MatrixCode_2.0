<!DOCTYPE html>
<html>
        <head>
                <title>MatrixCode</title>
                <meta charset='utf-8'>
                <meta name='viewport' content='width=device-width, initial-scale=1'>
                <link rel='stylesheet' href='http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.css' />
                <script src='http://code.jquery.com/jquery-1.9.1.min.js'></script>
                <script src='http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.js'></script>
                <style>
                        .ui-header .ui-title { margin-left: 1em; margin-right: 1em; text-overflow: clip; }
                </style>
        </head>
		<body>
<div data-role="page" id="page1">
    <div data-theme="a" data-role="header" data-position="fixed">
        <h3>MatrixCode Configuration</h3>
    </div>
    <div data-role="content">
        <div class="ui-grid-a">
            <div class="ui-block-a">
                <input id="cancel" value="Cancel" type="submit">
            </div>
            <div class="ui-block-b">
                <input id="save" data-theme="a" value="Save" type="submit">
            </div>
        </div>
    	<div data-role="fieldcontain">
            <label for="straight">
                Straight Digits
            </label>
            <select name="straight" id="straight" data-theme="" data-role="slider">
<?php
	if (!isset($_GET['straight'])) {
		$straight = 0;
	} else {
		$straight = $_GET['straight'];
	}
	
	if ($straight == 0) {
		echo '<option value="0" selected>Off</option><option value="1">On</option>';
	} else {
		echo '<option value="0">Off</option><option value="1" selected>On</option>';
	}
?>
			</select>
        </div>
        
        <div data-role="fieldcontain">
            <label for="semicolon">
                Blinking Semicolon
            </label>
            <select name="semicolon" id="semicolon" data-theme="" data-role="slider">
<?php
	if (!isset($_GET['semicolon'])) {
		$semicolon = 0;
	} else {
		$semicolon = $_GET['semicolon'];
	}
	
	if ($semicolon == 0) {
		echo '<option value="0" selected>Off</option><option value="1">On</option>';
	} else {
		echo '<option value="0">Off</option><option value="1" selected>On</option>';
	}
	?>
            </select>
        </div>
	</div>
</div>

    <script>
      function saveOptions() {
        var options = {
          'straight': parseInt($("#straight").val(), 10),
          'semicolon': parseInt($("#semicolon").val(), 10),
        }
        return options;
      }

      $().ready(function() {
        $("#cancel").click(function() {
          console.log("Cancel");
          document.location = "pebblejs://close#";
        });

        $("#save").click(function() {
          console.log("Submit");
          
          var location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(saveOptions()));
          console.log("Close: " + location);
          console.log(location);
          document.location = location;
        });

      });
    </script>
</body>
</html>
