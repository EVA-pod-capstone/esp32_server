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
const int MEASUREMENT_INTERVAL = 5000;
int year = 0;
int month = 0;
int day = 0;
int hour = 0;
int minute = 0;
int second = 0;

float latitude = 999.9;
float longitude = 999.9;

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// We will use wifi
#include <SPIFFS.h>
#include <WiFi.h>
#include <StreamLib.h>

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

File file;

String normal_page = " \
<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head> \
<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a> \
    <a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p> \
    <button id=\"sendData\" onclick=\"sendData()\">Send time and location</button> \
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
        } };\
        function sendData() { \
            var deviceClock = new Date();\
            var hour = deviceClock.getHours(); \
            var minute = deviceClock.getMinutes(); \
            var second = deviceClock.getSeconds(); \
            var day = deviceClock.getDate(); \
            var month = deviceClock.getMonth() + 1; \
            var year = deviceClock.getFullYear(); \
             fetch(window.location.href + \"send_data?year=\" + year + \"&month=\" + month + \"&day=\" + day \
                                      + \"&hour=\" + hour + \"&minute=\" + minute + \"&second=\" + second \
                                      + \"&latitude=\" + latitude + \"&longitude=\" + longitude, { \
            method: \"GET\", \
            headers: { \
                \"Accept\": \"application/json\", \
                \"Content-type\": \"application/json\" \
            } \
}); \
        }; \ 
    </script> \
</html> \
";

String deleted_page = " \
<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head> \
<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a> \
    <a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p> \
    <button id=\"sendData\" onclick=\"sendData()\">Send time and location</button> \
    <p id=\"deleted-status\">Data deleted</p> \
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
        } };\
        function sendData() { \
            var deviceClock = new Date();\
            var hour = deviceClock.getHours(); \
            var minute = deviceClock.getMinutes(); \
            var second = deviceClock.getSeconds(); \
            var day = deviceClock.getDate(); \
            var month = deviceClock.getMonth() + 1; \
            var year = deviceClock.getFullYear(); \
             fetch(window.location.href + \"send_data?year=\" + year + \"&month=\" + month + \"&day=\" + day \
                                      + \"&hour=\" + hour + \"&minute=\" + minute + \"&second=\" + second \
                                      + \"&latitude=\" + latitude + \"&longitude=\" + longitude, { \
            method: \"GET\", \
            headers: { \
                \"Accept\": \"application/json\", \
                \"Content-type\": \"application/json\" \
            } \
}); \
        }; \ 
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
void begin_file();
void increment_time();
void fake_measurement();
void append_data_to_file(String data_string);

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
String fake_data = "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
void loop() {
  // This call will let the server do its work
  secureServer.loop();

  // Other code would go here...
  delay(MEASUREMENT_INTERVAL);
  fake_measurement();
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
  begin_file();
  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(deleted_page);

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
    res->write(buffer, l);
  }
  res->println("");
}


void handleSend(HTTPRequest * req, HTTPResponse * res) {
  ResourceParameters * params = req->getParams();
  std::string paramVal;
  if (params->getQueryParameter("year", paramVal)){
    year = String(paramVal.c_str()).toInt();
    Serial.println(year);
  }
  if (params->getQueryParameter("month", paramVal)){
    month = String(paramVal.c_str()).toInt();
    Serial.println(month);
  }
  if (params->getQueryParameter("day", paramVal)){
day = String(paramVal.c_str()).toInt();
    Serial.println(day);  }
  if (params->getQueryParameter("hour", paramVal)){
hour = String(paramVal.c_str()).toInt();
    Serial.println(hour);  }
  if (params->getQueryParameter("minute", paramVal)){
minute = String(paramVal.c_str()).toInt();
    Serial.println(minute);  }
  if (params->getQueryParameter("second", paramVal)){
second = String(paramVal.c_str()).toInt();
    Serial.println(year);  }
  if (params->getQueryParameter("latitude", paramVal)){
    if (String(paramVal.c_str()).toFloat() != 999){
  latitude = String(paramVal.c_str()).toFloat();
    }
    Serial.println(latitude);  }
  if (params->getQueryParameter("longitude", paramVal)){
if (String(paramVal.c_str()).toFloat() != 999){
  longitude = String(paramVal.c_str()).toFloat();
    }    Serial.println(longitude);    // Serial.print(req->getRequestString());

  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(normal_page);

}
}


void fake_measurement(){
  // String fake_data = "";
  // if (year){
  //   fake_data = String(year) + "-" + String(month) + "-" + String(day) + "T" + String(hour) + ":" + String(minute) + ":" + String(second) + ", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
  // } else {
  //   fake_data = ", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
  // }
  increment_time();
  // month, day, hour, minute, second all need 0 padding
  String month_pad = (month < 10) ? "0" : "";
  String day_pad = (day < 10) ? "0" : "";
  String hour_pad = (hour < 10) ? "0" : "";
  String minute_pad = (minute < 10) ? "0" : "";
  String second_pad = (second < 10) ? "0" : "";

  String fake_data = String(year) + "-" + month_pad + String(month) + "-" + day_pad + String(day) + " " + hour_pad + String(hour) + ":" + minute_pad + String(minute) + ":" + second_pad + String(second) + ", " + String(latitude) + ", " + String(longitude) + ", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
  append_data_to_file(fake_data);
}


void increment_time(){
  int days_in_february = ((year % 400 == 0) || ((year % 4 == 0) && ((year % 100 != 0)))) ? 29 : 28;
  int days_in_curr_month = (month == 1) ? 31 : (month == 2) ? days_in_february : (month == 3) ? 31 :
                            (month == 4) ? 30 : (month == 5) ? 31 : (month == 6) ? 30 :
                            (month == 7) ? 31 : (month == 8) ? 31 : (month == 9) ? 30 :
                            (month == 10) ? 31 : (month == 11) ? 30 : 31;
  int seconds_added = MEASUREMENT_INTERVAL / 1000;
 // int seconds_added = 607000; // For testing correct rollover over long intervals
  int minutes_added = int((second + seconds_added) / 60);
  int hours_added = int((minute + minutes_added) / 60);
  int days_added = int((hour + hours_added) / 24);
  int months_added = int((day + days_added) / days_in_curr_month);
  int years_added = int((month + months_added) / 12);

  second = int((second + seconds_added) % 60);
  minute = int((minute + minutes_added) % 60);
  hour = int((hour + hours_added) % 24);
  day = int((day + days_added) % days_in_curr_month);
  month = int((month + months_added) % 12);
  year = int((year + years_added));
}

void begin_file(){
    file = SPIFFS.open("/data.csv", FILE_WRITE);
  if(!file){
    Serial.println("Error opening the file in WRITE mode");
    return;
  }
  else
  {
    Serial.println("File successfully opened in WRITE mode");
  }

  String dataFields = "Time, Latitude, Longitude, SoilHumidity, TempSoil, Conductivity, PH, Nitrogen, Phosphorus, Potassium, Salinity, TotalDissolvedSolids, AirHum, Airpress, Airtemp, Co2, light";

  if(file.println(dataFields))  // Write column labels to csv file
  {
    Serial.println("Data fields written to file");
  }

  file.close();

}

void append_data_to_file(String data_string){
    file = SPIFFS.open("/data.csv", FILE_APPEND);
  if(!file){
    Serial.println("Error opening the file in APPEND mode");
    return;
  }
  else
  {
  //  Serial.println("File successfully opened in APPEND mode");
  }

  if(file.println(data_string))  // Add new row to data file
  {
   // Serial.println("Data added to file");
  }

  file.close();

}
