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

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

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

File file;

String page_part1 = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head> \
<style> \
  .button{ \
    font-family:'Times New Roman', Times, serif; \
    font-size: 20px; \
    margin-left: 20px; \
  } \
  #download-button { \
      background-color: aquamarine; \
      border: 1px solid black; \
  } \
  #download-button:hover { \
      background-color: rgb(106, 233, 224); \
  } \
  #download-button:active { \
      background-color: rgb(40, 115, 189); \
  } \
  #delete-button { \
      background-color: rgb(255, 61, 61); \
      border: 1px solid black; \
      color: white; \
  } \
  #delete-button:hover { \
      background-color: rgb(213, 40, 40); \
  } \
  #delete-button:active { \
      background-color: rgb(233, 111, 24); \
  } \
  #sendLocation, #sendTime{ \
      background-color: rgb(255, 251, 195); \
      border: 1px solid black;\
      color: rgb(0, 0, 0);\
  } \
  #sendLocation:hover, #sendTime:hover{ \
      background-color: rgb(250, 235, 134); \
  } \
  #sendLocation:active, #sendTime:active{ \
      background-color: rgb(195, 233, 24); \
  } \
  .status-message{ \
    font-family:'Times New Roman', Times, serif; \
    font-size: 18px; \
    margin-left: 20px; \
  } \
  h1{ \
    background-color: rgb(164, 255, 154); \
    margin: -8px; \
  } \
</style> \
<body> \
<h1>EVA Pod Control</h1> \
<p> <button id=\"sendTime\" onclick=\"sendTime()\" class=\"button\">Send Time</button> \
  <button id=\"sendLocation\" onclick=\"sendLocation()\" class=\"button\">Send Location</button> \
<p><a href=\"/download\"><button id=\"download-button\" class=\"button\">Download Data</button></a> \
    <a href=\"/delete\"><button id=\"delete-button\" class=\"button\">Delete Data</button></a> </p>";

String page_status_message = "<p class=\"status-message\" id=\"status-message\">Status Message:</p>";

String page_part2 = "<hr> \
  <h4>Help</h4> \
  <p>Send Time: When setting up the EVA Pod, be sure to connect an external device and hit the 'Send Time' button. \
     When powered and running, the EVA Pod can keep time, but it needs to be provided a starting point. \
     When you hit this button, the EVA Pod will grab your device's current time and count from there to assign date and time \
     to all future measurements. Note that if it loses power or resets, it will lose time, so this step should be done again \
     after changing the battery. \
  </p> \
  <p>Send Location: Optionally hit this button when setting up the EVA Pod to store latitude and longitude with the data. \
    The EVA Pod has no internal GPS, but it can grab your device's GPS location if you allow it.  \
    This will allow the EVA website to later grab your EVA Pod's location automatically from your data file. \
    If the website cannot find location data in the file, it will ask you to enter it manually. \
    (This step does not need to be done again until you decide to move your EVA Pod to a new location.) \
 </p> \
 <p>Download Data: Press this button to download the csv file storing the EVA Pod's data. This file can be uploaded \
  directly to the EVA website or parsed and analyzed manually. \
</p> \
<p>Delete Data: Press this button to clear the contents of the csv file on the EVA Pod. This action cannot be undone. This does not clear the \
  stored time or location. If you wish to reset these data, press the button to update the corresponding value. \
</p> \
  </body> \
<script> \
        var latitude = 999; \
        var longitude = 999; \
        var err_message = \"\"; \
        window.onload = function() { \
        if (navigator.geolocation) {  \
            navigator.geolocation.getCurrentPosition((position) => {  \
              latitude = position.coords.latitude; \
              longitude = position.coords.longitude; \ 
              console.log(latitude); \
              console.log(longitude); \
          }, (error) => {console.log(error); err_message = error.message; }); \ 
        } else { \
            console.log(\"Geolocation is not supported by this browser.\"); \
            latitude = 999; \
            longitude = 999; \
        }}; \
        function sendLocation() { \
                  if (err_message == \"\"){ \
          document.getElementById(\"status-message\").innerText = \"Status Message: \" + err_message; \
          } else { \
            document.getElementById(\"status-message\").innerText = \"Status Message: Sending Location\"; \
          } \
             fetch(window.location.origin + \"/send_location?latitude=\" + latitude + \"&longitude=\" + longitude, { \
            method: \"GET\", \ 
            headers: { \
                \"Accept\": \"application/json\", \
                \"Content-type\": \"application/json\" \
            } \
}); \
        }; \
        function sendTime() { \
            var deviceClock = new Date();\
            var hour = deviceClock.getUTCHours(); \
            var minute = deviceClock.getUTCMinutes(); \
            var second = deviceClock.getUTCSeconds(); \
            var day = deviceClock.getUTCDate(); \
            var month = deviceClock.getUTCMonth() + 1; \
            var year = deviceClock.getUTCFullYear(); \
            fetch(window.location.origin + \"/send_time?year=\" + year + \"&month=\" + month + \"&day=\" + day \
                                      + \"&hour=\" + hour + \"&minute=\" + minute +\"&second=\" + second, { \
            method: \"GET\", \
            headers: { \
                \"Accept\": \"application/json\", \
                \"Content-type\": \"application/json\" \
            } \
}); \
        }; \
    </script> \ 
</html>";

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
  ResourceNode * nodeTimestamp = new ResourceNode("/send_time", "GET", &handleTimestamp);
  ResourceNode * nodeLocation = new ResourceNode("/send_location", "GET", &handleLocation);


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
String fake_data = "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
void loop() {
  // This call will let the server do its work
  secureServer.loop();

  // Other code would go here...
  delay(MEASUREMENT_INTERVAL);
  fake_measurement();
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  String month_pad = (month < 10) ? "0" : "";
  String day_pad = (day < 10) ? "0" : "";
  String hour_pad = (hour < 10) ? "0" : "";
  String minute_pad = (minute < 10) ? "0" : "";
  String second_pad = (second < 10) ? "0" : "";

  String timestring = String(year) + "-" + month_pad + String(month) + "-" + day_pad + String(day) + " " + hour_pad + String(hour) + ":" + minute_pad + String(minute) + ":" + second_pad + String(second);

  String page_last_meas_time = "<p class=\"status-message\" id=\"last_meas_time\">Last Measurement: " + timestring + "</p>";
  String page_saved_location = "<p class=\"status-message\" id=\"status-message\">Latitude: " + latitude + ", Longitude: " + longitude + "</p>";
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(page_part1);
  res->println(page_status_message);
  res->println(page_last_meas_time);
  res->println(page_saved_location);
  res->println(page_part2);

}

void handleDelete(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");
  begin_file();
  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(page_part1);
  res->println("<p class=\"status-message\" id=\"status-message\">Status Message: Data deleted</p>");
  res->println(page_part2);

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

void handleTimestamp(HTTPRequest * req, HTTPResponse * res){
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
    Serial.println(second);  
  }

  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(page_part1);
  res->print("<p class=\"status-message\" id=\"status-message\">Status Message: Received timestamp </p>");
  res->print(year);
  res->print("-");
  res->print(month);
  res->print("-");
  res->print(day);
  res->print("T");
  res->print(hour);
  res->print(":");
  res->print(minute);
  res->print(":");
  res->print(second);
  res->println("</p>");
  res->println(page_part2);
}


void handleLocation(HTTPRequest * req, HTTPResponse * res) {
  ResourceParameters * params = req->getParams();
  std::string paramVal;
  
  if (params->getQueryParameter("latitude", paramVal)){
    if (String(paramVal.c_str()).toFloat() != 999){
      latitude = String(paramVal.c_str()).toFloat();
    }
    Serial.println(latitude);  
  }
  if (params->getQueryParameter("longitude", paramVal)){
    if (String(paramVal.c_str()).toFloat() != 999){
     longitude = String(paramVal.c_str()).toFloat();
    }
  Serial.println(longitude);   
  }

  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(page_part1);
  res->print("<p class=\"status-message\" id=\"status-message\">Status Message: Received location </p>");
  res->print(latitude);
  res->print(", ");
  res->print(longitude);
  res->println("</p>");
  res->println(page_part2);

}


void fake_measurement(){
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
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  if ((fs_info.totalBytes - fs_info.usedBytes) > 255){ // each line should only take 83 bytes but we leave extra space
      file = SPIFFS.open("/data.csv", FILE_APPEND);
    if (!file) {
      Serial.println("Error opening the file in APPEND mode");
      return;
    } else {
      Serial.println("File successfully opened in APPEND mode");
    }

    if(file.println(data_string))  // Add new row to data file
    {
      Serial.println("Data added to file");
    }
    file.close();
    } else {
      Serial.println('Cannot append to data file');
    }
}
