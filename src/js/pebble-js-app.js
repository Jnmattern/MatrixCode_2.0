var straight = localStorage.getItem("straight");
if (!straight) {
	straight = 0;
}

var semicolon = localStorage.getItem("semicolon");
if (!semicolon) {
	semicolon = 0;
}

Pebble.addEventListener("ready", function() {
	console.log("Ready Event");
	console.log("	straight: " + straight);
	console.log("	semicolon: " + semicolon);
	initialized = true;
});

Pebble.addEventListener("showConfiguration", function(e) {
	console.log("showCOnfiguration Event");
	console.log("	straight: " + straight);
	console.log("	semicolon: " + semicolon);
	Pebble.openURL("http://www.famillemattern.com/jnm/pebble/MatrixCode/MatrixCode.php?straight=" + straight + "&semicolon=" + semicolon);
});

Pebble.addEventListener("webviewclosed", function(e) {
	console.log("Configuration window closed");
	console.log(e.type);
	console.log(e.response);

	var configuration = JSON.parse(e.response);
	Pebble.sendAppMessage(configuration);
	
	straight = configuration["straight"];
	localStorage.setItem("straight", straight);
	
	semicolon = configuration["semicolon"];
	localStorage.setItem("semicolon", semicolon);
});
