#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>


const char ssid[] = "ucqy"; //SSID
const char pass[] = ""; //Password
const char* host = "iot";

const char *ap_ssid = "ESPap";
const char *ap_password = "12345678";

ESP8266WebServer server(80);

//实现了简单的HTTP服务器
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void setup( ) {

  Serial.begin(115200);
  SPIFFS.begin();
  Serial.print("welcome\r\n");
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);
  //WiFi.begin(ssid, pass);

  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  Serial.println("");
  //  Serial.print("Connected to ");
  //  Serial.println(ssid);
  //  Serial.print("IP address: ");
  //  Serial.println(WiFi.localIP());
  //  IPAddress ip = WiFi.localIP();

  if (!MDNS.begin(host, myIP)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }

  Serial.println("");
  //  Serial.print("Connected! IP address: ");
  //  Serial.println(WiFi.localIP());

  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  server.on ("/gpio02", []() {
    String state = server.arg("state");
    if (state == "SET") {
      digitalWrite(2, HIGH);
    } else if (state == "RESET") {
      digitalWrite(2, LOW);
    }
  });
  server.on ("/gpio16", []() {
    String state = server.arg("state");
    if (state == "SET") {
      digitalWrite(16, HIGH);
    } else if (state == "RESET") {
      digitalWrite(16, LOW);
    }
  });
  server.on ("/gpio14", []() {
    String state = server.arg("state");
    if (state == "SET") {
      digitalWrite(14, HIGH);
    } else if (state == "RESET") {
      digitalWrite(14, LOW);
    }
  });
  server.on ("/gpio12", []() {
    String state = server.arg("state");
    if (state == "SET") {
      digitalWrite(12, HIGH);
    } else if (state == "RESET") {
      digitalWrite(12, LOW);
    }
  });
  server.on ("/gpio13", []() {
    String state = server.arg("state");
    if (state == "SET") {
      digitalWrite(13, HIGH);
    } else if (state == "RESET") {
      digitalWrite(13, LOW);
    }
  });
  server.on ("/gpio09", []() {
    String state = server.arg("state");
    if (state == "SET") {
      digitalWrite(9, HIGH);
    } else if (state == "RESET") {
      digitalWrite(9, LOW);
    }
  });
  server.on ("/state", []() {
    int state = WiFi.status() == WL_CONNECTED ? 1 : 0 ;
    StaticJsonBuffer<2000> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["state"] = state ;
    root["ssid"] = state == 1 ? WiFi.SSID() : "" ;
    root["rssi"] = state == 1 ? WiFi.RSSI() : 0 ;
    IPAddress ip(192, 168, 0, 2);
    root["ip"] = state == 1 ? IpAddress2String(WiFi.localIP()) : "" ;
    root["title"] = "wifi-state";
    String msg = "" ;
    root.printTo(msg);
    server.send(200, "application/json", msg);
  });
  server.on("/con", []() {
    String ssid = server.arg("ssid");
    String pwd = server.arg("pwd");
    WiFi.begin(ssid.c_str(), pwd.c_str());
  });
  server.on ("/wifis", []() {
    String state = server.arg("state");
    String msg = "" ;
    StaticJsonBuffer<2000> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["title"] = "wifi-list";

    // 添加一个data数组
    JsonArray& data = root.createNestedArray("data");
    //data.add(double_with_n_digits(48.756080, 6));
    Serial.println("1------------1");
    int n = WiFi.scanNetworks();
    n = n > 10 ? 10 : n;
    root["size"] = n ;
    Serial.println(n);

    for (int i = 0; i < n; ++i)
    {
      JsonObject& wifi = jsonBuffer.createObject();
      wifi["index"] = i ;
      wifi["ssid"] = WiFi.SSID(i);
      wifi["rssi"] = WiFi.RSSI(i);
      Serial.println(WiFi.SSID(i));
      data.add(wifi);
      delay(10);
    }
    root.printTo(msg);
    server.send(200, "application/json", msg);
  });


  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404 Not Found.");
  });

  server.begin();
  Serial.println("HTTP server started");

  MDNS.setInstanceName("IOT605 WiFiBoard");
  MDNS.addService("http", "tcp", 80);
  Serial.println("end");
}

void loop() {
  server.handleClient();
}
