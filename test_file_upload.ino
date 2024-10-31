#include <SPIFFS.h>
#include <WiFi.h>
#include <StreamLib.h>

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
           
            // The HTTP response ends with another blank line
            client.println();
            Serial.print(request);
            Serial.println("is the request");
            if (request.indexOf("GET /download") >= 0) {
              Serial.println("Download Requested");
              File dataFile = SPIFFS.open("/test.csv", FILE_READ);
              int filesize = dataFile.size();
              // Serial.println("Content-Length:"+String(filesize));
              // client.println("HTTP/1.1 200 OK");
              // client.println("Content-Type: text/plain");
              // client.println("Content-Length:"+String(filesize));
              // client.println("Connection: close");
              // client.println();
              // client.write(dataFile);

              char buff[1024];

              ChunkedPrint bp(client, buff, sizeof(buff));
              bp.println(F("HTTP/1.1 200 OK"));
              bp.println(F("Connection: close"));
              bp.print(F("Content-Length: "));
              bp.println(dataFile.size());
              bp.print(F("Content-Type: text/plain"));
              bp.println(F("Content-Disposition: attachment; filename: \"test.txt\""));
              bp.println();
              uint16_t c = 0;
              while (dataFile.available()) {
                bp.write(dataFile.read());
              }
              dataFile.close();
              bp.flush();
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
              //dataFile.close();
               client.println();
             // output = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><script type=\"text/javascript\" src=\"jzip.js\"></script><script>var filelist = [";
            } else { client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Send your "Hello World" HTML response
            client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
            client.println("<body><p><a href=\"/download\"><button class=\"button button2\">Download</button></a></p></body></html>");
            //client.println("<body><p><a href=\"test.txt\">Download!</a></p></body></html>");
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
    file = SPIFFS.open("/test.csv", FILE_WRITE);
  if(!file){
    Serial.println("Error opening the file in WRITE mode");
    return;
  }
  else
  {
    Serial.println("Open file success");
  }

  String testline1= "a, b, c, d, e, f, g";
  String testline2 = "1, 2, 3, 4, 5, 6, 7";

  if(file.println(testline1))
  {
    Serial.println("Line 1 written");
  }

  if(file.println(testline2))
  {
    Serial.println("Line 2 Written");
  }

  file.close();
}
