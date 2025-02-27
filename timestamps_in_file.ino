#include <SPIFFS.h>
#include <WiFi.h>
#include <StreamLib.h>

// Set ESP32 wifi server credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

int year = 0;
int month = 0;
int day = 0;
int hour = 0;
int minute = 0;
int second = 0;

const int MEASUREMENT_INTERVAL = 5000;
String timestring = "";
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

// input button pin
int button = 0;

// Set web server port number to 80
WiFiServer server(80);
File file;

void setup() {

  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
  Serial.println("Error mounting SPIFFS");
  } else {
    begin_file();
  }

  pinMode(button, INPUT_PULLUP);
  digitalWrite(button, HIGH);  
  attachInterrupt(digitalPinToInterrupt(button), isr, RISING);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.begin();
  
}

void loop() {
  delay(MEASUREMENT_INTERVAL);
  fake_measurement();
   WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String request = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        currentLine += c;
        request += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 2) {
            Serial.println("Received request");
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
           
            // The HTTP response ends with another blank line
            client.println();
            Serial.print(request);
            Serial.println("is the request");
            if (request.indexOf("GET /download") >= 0) {
              Serial.println("Download Requested");
              File dataFile = SPIFFS.open("/data.csv", FILE_READ);
              int filesize = dataFile.size();

              char buff[1024];

              ChunkedPrint bp(client, buff, sizeof(buff));
              bp.println(F("HTTP/1.1 200 OK"));
              bp.println(F("Connection: close"));
              bp.print(F("Content-Length: "));
              bp.println(dataFile.size());
              bp.println(F("Content-Type: text/plain"));
              bp.println(F("Content-Disposition: attachment; filename=\"data.csv\""));
              bp.println();
              uint16_t c = 0;
              while (dataFile.available()) {
                bp.write(dataFile.read());
              }
              dataFile.close();
              bp.flush();

               client.println();
             // output = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><script type=\"text/javascript\" src=\"jzip.js\"></script><script>var filelist = [";
            } else if (request.indexOf("GET /delete") >= 0) {
              begin_file();
              client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Send your "Hello World" HTML response
            client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
            client.println("<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a><a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p></body><p>Data deleted</p></html>");
            //client.println("<body><p><a href=\"test.txt\">Download!</a></p></body></html>");
              } else if (request.indexOf("GET /timestamp") >= 0) { 
              timestring = request.substring(request.indexOf("GET /timestamp")+15);

                            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
             //client.println(normal_page);
             year = timestring.substring(timestring.indexOf("year=")+5, timestring.indexOf("&month")).toInt();
            month = timestring.substring(timestring.indexOf("month=")+6, timestring.indexOf("&day")).toInt();
             day = timestring.substring(timestring.indexOf("day=")+4, timestring.indexOf("&hour")).toInt();
             hour = timestring.substring(timestring.indexOf("hour=")+5, timestring.indexOf("&minute")).toInt();
             minute = timestring.substring(timestring.indexOf("minute=")+7, timestring.indexOf("&second")).toInt();
             second = timestring.substring(timestring.indexOf("second=")+7).toInt();

                // timestring = timestring.substring(0, timestring.indexOf(" "));
                // append_data_to_file(timestring+"&measurement_interval="+MEASUREMENT_INTERVAL);
                Serial.print("The timestring is ");
                Serial.println(timestring);
                Serial.println(year);
                Serial.println(month);
  Serial.println(day);
                Serial.println(hour);  
                Serial.println(minute);
                Serial.println(second);
            // Send your "Hello World" HTML response
            // client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
            // client.println("<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a><a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p></body></html>");
           
} else { client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Send your "Hello World" HTML response
            // client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
            // client.println("<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a><a href=\"/delete\"><button class=\"button button2\">Delete</button></a></p></body></html>");
            client.println(normal_page);
}
            // Break out of the while loop
            request = "";
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  } 
  // delay(5000);
  // Serial.println(1);
}


void isr() // if wifi on, turn off. if wifi off, turn on
{
       Serial.println(50);  

}

void increment_time(){
  int days_in_february = ((year % 400 == 0) || ((year % 4 == 0) && ((year % 100 != 0)))) ? 29 : 28;
  int days_in_curr_month = (month == 1) ? 31 : (month == 2) ? days_in_february : (month == 3) ? 31 :
                            (month == 4) ? 30 : (month == 5) ? 31 : (month == 6) ? 30 :
                            (month == 7) ? 31 : (month == 8) ? 31 : (month == 9) ? 30 :
                            (month == 10) ? 31 : (month == 11) ? 30 : 31;
  int seconds_added = MEASUREMENT_INTERVAL / 1000;
  int minutes_added = int((second + seconds_added) / 60);
  int hours_added = int((minute + minutes_added) / 60);
  int days_added = int((hour + hours_added) / 24);
  int months_added = int((day + days_added) / days_in_curr_month);
  int years_added = int((month + months_added) / 12);

  second = int((second + seconds_added) % 60);
  minute = int((minute + minutes_added) % 60);
  hour = int((hour + hours_added) % 24);
  day = int((day + days_added) % 30);
  month = int((month + months_added) % 12);
  year = int((year + years_added));
}

void fake_measurement(){
  // String fake_data = "";
  // if (year){
  //   fake_data = String(year) + "-" + String(month) + "-" + String(day) + "T" + String(hour) + ":" + String(minute) + ":" + String(second) + ", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
  // } else {
  //   fake_data = ", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
  // }
  increment_time();
  String fake_data = String(year) + "-" + String(month) + "-" + String(day) + "T" + String(hour) + ":" + String(minute) + ":" + String(second) + ", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14";
  append_data_to_file(fake_data);
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

  String dataFields = "Time, SoilHumidity, TempSoil, Conductivity, PH, Nitrogen, Phosphorus, Potassium, Salinity, TotalDissolvedSolids, AirHum, Airpress, Airtemp, Co2, light";

  if(file.println(dataFields))  // Write column labels to csv file
  {
    Serial.println("Data fields written to file");
  }

  file.close();

}
