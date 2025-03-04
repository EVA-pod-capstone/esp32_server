/**
 * Example for the ESP32 HTTP(S) Webserver
 *
 * IMPORTANT NOTE:
 * To run this script, your need to
 *  1) Enter your WiFi SSID and PSK below this comment
 *  2) Make sure to have certificate data available. You will find a
 *     shell script and instructions to do so in the library folder
 *     under extras/
 *
 * This script will install an HTTPS Server on your ESP32 with the following
 * functionalities:
 *  - Show simple page on web server root
 *  - 404 for everything else
 */

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// Binary data for the favicon
#include "favicon.h"

// We will use wifi
#include <SPIFFS.h>
#include <WiFi.h>
#include <StreamLib.h>

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

String normal_page = " \
<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head> \
<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a> \
    <a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p> \
    <a href=\"/send_data\"><button class=\"button button2\">Send timestamp and location</button></a></p> \
    <p id=\"error-code\">Error code will display here</p></body> \
    <script> \
        var latitude = 999; \
        var longitude = 999; \
        window.onload = function() { \
        if (navigator.geolocation) { \
            navigator.geolocation.getCurrentPosition((position) => {  \
              latitude = position.coords.latitude; \
              longitude = position.coords.longitude; \
              console.log(latitude); \
              console.log(longitude); \
              }, (error) => {console.log(error); document.getElementById(\"error-code\").innerText = error.message; \
switch(error.code) { \
    case error.PERMISSION_DENIED: \
      console.log(\"Permission denied\"); \
      break; \
    case error.POSITION_UNAVAILABLE: \
      console.log(\"Position unavailable\"); \
      break; \
    case error.TIMEOUT: \
      console.log(\"Location timeout\"); \
      break; \
    case error.UNKNOWN_ERROR: \
      console.log(\"Unknown error\"); \
      break; \
  } \
  }); \
        } else { \
            console.log(\"Geolocation is not supported by this browser. Setting both to 999...\"); \
            latitude = 999; \
            longitude = 999; \
        } \
            var deviceClock = new Date();\
            var hour = deviceClock.getHours(); \
            var minute = deviceClock.getMinutes(); \
            var second = deviceClock.getSeconds(); \
            var day = deviceClock.getDate(); \
            var month = deviceClock.getMonth() + 1; \
            var year = deviceClock.getFullYear(); \
        } \ 
    </script> \
</html> \
";

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

// Create an SSL-enabled server that uses the certificate
// The contstructor takes some more parameters, but we go for default values here.
HTTPSServer secureServer = HTTPSServer(&cert);

// Declare some handler functions for the various URLs on the server
// The signature is always the same for those functions. They get two parameters,
// which are pointers to the request data (read request body, headers, ...) and
// to the response data (write response, set status code, ...)
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleDownload(HTTPRequest * req, HTTPResponse * res);
void handleDelete(HTTPRequest * req, HTTPResponse * res);
void handleTimestamp(HTTPRequest * req, HTTPResponse * res);
void handleLocation(HTTPRequest * req, HTTPResponse * res);

void setup() {
  // For logging
  Serial.begin(115200);

  if(!SPIFFS.begin(true)){
  Serial.println("Error mounting SPIFFS");
  } else {
    begin_file();
  }
  
  WiFi.softAP(ssid, password);

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeDownload = new ResourceNode("/download", "GET", &handleDownload);
  ResourceNode * nodeDelete = new ResourceNode("/delete", "GET", &handleDelete);
  ResourceNode * nodeSend = new ResourceNode("/send_data", "GET", &handleSend);

  // Add the nodes to the server
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeDownload);
  secureServer.registerNode(nodeDelete);
  secureServer.registerNode(nodeSend);

  Serial.println("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");
  }
}

void loop() {
  // This call will let the server do its work
  secureServer.loop();

  // Other code would go here...
  delay(1);
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}

void handleDelete(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}

void handleDownload(HTTPRequest * req, HTTPResponse * res) {
  Serial.print("got download request");
  //Serial.print(req->getRequestString());
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/plain");
  res->setHeader("Content-Disposition", "attachment");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  // res->println(normal_page);
  File dataFile = SPIFFS.open("/data.csv", FILE_READ);
  int filesize = dataFile.size();

  byte buffer[256];
  // HTTPRequest::requestComplete can be used to check whether the
  // body has been parsed completely.
  int l;
  while(dataFile.available()) {
    l = dataFile.readBytes((char*)buffer, sizeof(buffer));
    // buffer[l] = 0;
    // Serial.println(buffer);
    // // HTTPRequest::readBytes provides access to the request body.
    // // It requires a buffer, the max buffer length and it will return
    // // the amount of bytes that have been written to the buffer.
    // size_t s = req->readBytes(buffer, 256);

    // The response does not only implement the Print interface to
    // write character data to the response but also the write function
    // to write binary data to the response.
    res->write(buffer, l);
  }
  res->println("");

}

void handleSend(HTTPRequest * req, HTTPResponse * res) {
    // Serial.print(req->getRequestString());

  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}


void begin_file(){
    File file = SPIFFS.open("/data.csv", FILE_WRITE);
  if(!file){
    Serial.println("Error opening the file in WRITE mode");
    return;
  }
  else
  {
    Serial.println("File successfully opened in WRITE mode");
  }

  String dataFields = "Time, SoilHumidity, TempSoil, Conductivity, PH, Nitrogen, Phosphorus, Potassium, Salinity, TotalDissolvedSolids, AirHum, Airpress, Airtemp, Co2, light";

  if(file.println(dataFields))  // Write column labels to csv file
  {
    Serial.println("Data fields written to file");
  }

  file.close();

}
