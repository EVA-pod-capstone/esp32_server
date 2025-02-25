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
#include <WiFi.h>

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

String normal_page = " \
<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head> \
<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a> \
    <a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p> \
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
            fetch(window.location.href + \"timestamp?year=\" + year + \"&month=\" + month + \"&day=\" + day \
                                      + \"&hour=\" + hour + \"&minute=\" + minute + \"&second=\" + second \
                                      + \"&latitude=\" + latitude + \"&longitude=\" + longitude, { \
            method: \"GET\", \
            headers: { \
                \"Accept\": \"application/json\", \
                \"Content-type\": \"application/json\" \
            } \
}); \
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
  
  WiFi.softAP(ssid, password);

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeDownload = new ResourceNode("/download", "GET", &handleDownload);
  ResourceNode * nodeDelete = new ResourceNode("/delete", "GET", &handleDelete);
  ResourceNode * nodeTimestamp = new ResourceNode("/timestamp", "GET", &handleTimestamp);
  ResourceNode * nodeLocation = new ResourceNode("/location", "GET", &handleLocation);

  // Add the nodes to the server
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeDownload);
  secureServer.registerNode(nodeDelete);
  secureServer.registerNode(nodeTimestamp);
  secureServer.registerNode(nodeLocation);

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
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}

void handleTimestamp(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}

void handleLocation(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}
