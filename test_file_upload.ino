#include <SPIFFS.h>
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Set web server port number to 80
WiFiServer server(80);
File file;

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
  Serial.println("Error mounting SPIFFS");
  } else {
    write_file();
  }

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
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Send your "Hello World" HTML response
            client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
            client.println("<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a></p></body></html>");
            //client.println("<body><p><a href=\"test.txt\">Download!</a></p></body></html>");

            // The HTTP response ends with another blank line
            client.println();
            Serial.print(request);
            Serial.println("is the request");
            if (request.indexOf("GET /download") >= 0) {
              Serial.println("Download Requested");
              File dataFile = SPIFFS.open("/test.txt", FILE_READ);
              int filesize = dataFile.size();
              Serial.println("Content-Length:"+String(filesize));
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/plain");
              client.println("Content-Length:"+String(filesize));
              client.println("Connection: close");
              client.println();
              client.write(dataFile);
              // uint8_t buf[256];
              // while (dataFile.available())
              // {
              //   int n = dataFile.read(buf, sizeof(buf));
              //   for (int i = 0; i < n; i++)
              //   {
              //     Serial.print(buf[i]);
              //     Serial.println('end');
              //     client.print(buf[i]);
              //     //res->write(buf[i]);
              //   }
              // }
              dataFile.close();
               client.println();
             // output = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><script type=\"text/javascript\" src=\"jzip.js\"></script><script>var filelist = [";
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

void write_file(){
    file = SPIFFS.open("/test.txt", FILE_WRITE);
  if(!file){
    Serial.println("Error opening the file in WRITE mode");
    return;
  }
  else
  {
    Serial.println("Open file success");
  }

  String testssid= "MY_WIFI";
  String testpass = "MY_WIFI_PASSWORD";

  if(file.println(testssid))
  {
    Serial.println("SSID Written");
  }

  if(file.println(testpass))
  {
    Serial.println("PASSWORD Written");
  }

  file.close();
}
