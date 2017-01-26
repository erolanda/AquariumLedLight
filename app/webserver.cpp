#include <configuration.h>
#include <Services/ArduinoJson/ArduinoJson.h>

HttpServer server;
int totalActiveSockets = 0;

int getActiveSockets(){
	return totalActiveSockets;
}

void onIndex(HttpRequest &request, HttpResponse &response){
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest &request, HttpResponse &response){
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void wsSendData(String json){
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString(json);
}

void wsConnected(WebSocket& socket){
	totalActiveSockets++;
}

void wsMessageReceived(WebSocket& socket, const String& message){
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(message);
	if (!root.success()){
  	Serial.println("parseObject() failed");
  	return;
	}
	if(root["brightness"]){
		int brightness = root["brightness"];
		if(brightness == 1000){
			brightness = 0;
		}
		setCurrentBrightness(brightness);
	}else if(root["fan"]){
		int value = root["fan"];
		setMode(false);
		if(value == 1)
			controlFan(true);
		else
			controlFan(false);
	}
}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size){
	Serial.printf("Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket){
	totalActiveSockets--;
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

	// // Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}
