Pebble.addEventListener("ready", function(e) {
	console.log("ready");
});

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL("http://www.foodev.mobi/settings.html");
});

Pebble.addEventListener("webviewclosed", function(e) {
	console.log(e.response);
	var responseFromWebView = decodeURIComponent(e.response);
	var settings = JSON.parse(responseFromWebView);
	Pebble.sendAppMessage(settings);
});

