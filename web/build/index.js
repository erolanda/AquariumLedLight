window.onload = downScripts;

function downScripts() {
    startWebSockets();
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
  $('#fadein').text(json.fadein + " hrs");
  $('#fadeout').text(json.fadeout + " hrs");
  console.log(json.fans);
  if(json.fans)
    $('#vent').text('On');
  else
    $('#vent').text('Off');
  $('#leds').text(json.brightness);
}

function onError(evt) {
  console.log(evt.data);
}
