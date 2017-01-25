window.onload = downScripts;
var currentData = 0;

function downScripts() {
	$('#ledSlider').slider();
	startWebSockets();
	$('#ledSlider').on("slide", function(slideEvt) {
	  if (currentData == slideEvt.value)
	    return;
	  else{
	    currentData = slideEvt.value;
			sendData(slideEvt.value);
	  }
	});
}

function startWebSockets() {
    var wsUri = "ws://" + location.host + "/";
    websocket = new WebSocket(wsUri);
    websocket.onopen = function(evt) {
        onOpen(evt)
    };
    websocket.onclose = function(evt) {
        onClose(evt)
    };
    websocket.onmessage = function(evt) {
        onMessage(evt)
    };
    websocket.onerror = function(evt) {
        onError(evt)
    };
}

function onOpen(evt) {
  // console.log('Conectado');
}

function onClose(evt) {
  console.log('Cerrado');
}

function onMessage(evt) {
  json = JSON.parse(evt.data);
  console.log(json);
}

function onError(evt) {
  console.log(evt.data);
}

function sendData(message) {
	var json = {};
	if(message == 0)
		message = 1000;
	json['brightness'] = message;
	websocket.send(JSON.stringify(json));
	console.log(JSON.stringify(json));
}
