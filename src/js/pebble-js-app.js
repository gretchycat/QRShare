Pebble.addEventListener("ready", function(e) {
	console.log("ready");
});

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL("http://www.foodev.mobi/settings.html");
});

Pebble.addEventListener("webviewclosed", function(e) {
	console.log(e.response);
	var responseFromWebView = decodeURIComponent(e.response);
//	var settings = JSON.parse(responseFromWebView);
	var settings2={
		"QRContent1":"Content 1!!!"
	};
	settings2['QRContent1']="Content 1!!!";
	settings2['QRContent2']="Content 2!!!";
	settings2['QRContent3']="Content 3!!!";
	settings2['QRContent4']="Content 4!!!";
	settings2['QRDesc1']="Desc 1!!!";
	settings2['QRDesc2']="Desc 2!!!";
	settings2['QRDesc3']="Desc 3!!!";
	settings2['QRDesc4']="Desc 4!!!";

	Pebble.sendAppMessage(settings2);
});

