
// Direct logging after access is approved by Google and Confimed by User of scripting request via esp8266


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define ON_Board_LED 2 

const char* ssid = "Arduino Wifi"; 
const char* password = "101010101";

const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client; 

String GAS_ID = "GAS_ID";


void setup() {
  
  Serial.begin(115200);
  delay(500);

 
  delay(500);
  
  WiFi.begin(ssid, password); 
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); 
  digitalWrite(ON_Board_LED, HIGH);

 
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
   
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
   
  }
 
  digitalWrite(ON_Board_LED, HIGH);
 
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
 

  client.setInsecure();
}

void loop() {
 
  int h = 60;    // dummy value for sheet log test
 
  float t = 28;  // dummy value for sheet log test
  
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";
  Serial.println(Temp);
  Serial.println(Humi);
  
  sendData(t, h); 
}

void sendData(float tem, int hum) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  
  String string_temperature =  String(tem);
  String string_humidity =  String(hum, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?Temperature=" + string_temperature + "&Humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("Check esp8266 post in Google Sheet");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  
} 


/*
GOOGLE DRIVE GAS SCRIPT

https://script.google.com/macros/s/<GAS_ID>/exec?Temperature=28&Humidity=70


function doGet(e) { 
  Logger.log( JSON.stringify(e) );  // view parameters
  var result = '...DataLogged'; // assume success
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id = 'Spreadsheet ID in Google drive'; 		            // Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();		// get Active sheet
    var newRow = sheet.getLastRow() + 1;						
    var rowData = [];
	var Curr_Date = new Date(); 
    rowData[0] = Curr_Date; 											// Date in column A
	var Curr_Time = Utilities.formatDate(Curr_Date, "GMT+8", 'HH:mm:ss');
	rowData[1] = Curr_Time; 											// Time in column B
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'Temperature': //Parameter
          rowData[2] = value; //Value in column C
          break;
        case 'Humidity': //Parameter
          rowData[3] = value; //Value in column D
          break;  
        default:
          result = "unsupported parameter";
      }
    }
    Logger.log(JSON.stringify(rowData));
    // Write new row below
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  // Return result of operation
  return ContentService.createTextOutput(result);
}

//Remove leading and trailing single or double quotes

function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}

//-----------------------------------------------
// End of file
//-----------------------------------------------

*/
