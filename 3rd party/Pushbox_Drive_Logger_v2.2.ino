
/*
 strftime Clock + DHT11 + 15 min/post + 4 Column Google Drive Sheet Chart Datalogger + 3rd party pushbox post

 API LINK:             http://api.pushingbox.com/pushingbox?devid=<PushboxAPI>&Temperature=28&Humidity=80

 SHEET SCRIPT LINK:    https://script.google.com/macros/s/<GAS_ID>/exec?Temperature=28&Humidity=70

*/

#include <SPI.h>
#include <Wire.h>
#include <dht11.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     2 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

dht11 DHT11; 
#define dhtpin  2   //D4

const char *ssid     = "Arduino Wifi";
const char *password = "101010101";

const char *host = "api.pushingbox.com";  

String GAS_ID = "GAS_ID";    //Get when published as web app from Gdrive tools *choose anyone even anony logger

const char* ntpServer 		    = "pool.ntp.org";
const long  gmtOffset_sec 	   	= 28800;              //UTC+8 Region
const int   daylightOffset_sec 	= 0;

char buffer[100];

extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

WiFiClient client;

int T;   
int H;  

void setup() {	

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);display.display(); delay(500);
  Serial.begin(115200);WiFi.mode(WIFI_STA);WiFi.begin(ssid, password); 
  while ( WiFi.status() != WL_CONNECTED ) {delay(200);Serial.print( "." );}
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}

void printLocalTime()
{
  DHT11.read(dhtpin);
  H = DHT11.humidity;T = DHT11.temperature;;
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,80,"%A, %e.%m.%C, %I:%M%p, ",timeinfo);
  Serial.print(buffer);
  Serial.print("Temp:"); Serial.print(T);Serial.print("°c,"); Serial.print(" Humidity:");Serial.print(H); Serial.println("%.");            
  delay(1000); 
}

void printLocalDay()
{ 
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,80,"%A",timeinfo);
  display.clearDisplay();display.setTextSize(2);display.setTextColor(WHITE); 
  display.setCursor(15,25); display.print(buffer);display.display();
  delay(500); 
}

void printLocalDate()
{
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,80,"%e.%m.%C",timeinfo);
  display.clearDisplay();display.setTextSize(2);display.setTextColor(WHITE); 
  display.setCursor(15,25); display.print(buffer);display.display();
  delay(500); 
}

void printTime()
{
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,80,"%I:%M%p",timeinfo);//logTime=buffer;
  display.clearDisplay();display.setTextSize(3);display.setTextColor(WHITE); 
  display.setCursor(2,23); display.print(buffer);display.display();
  delay(500); 
}

void printAmbience()
{
  DHT11.read(dhtpin);
  H = DHT11.humidity;T = DHT11.temperature;
  display.setTextSize(2);
  display.setCursor(0,25);display.print(" T:");display.print(T);display.print(" H:");display.println(H);display.display(); delay(500);   
}
void loop(void)
{
  for (int i = 0; i < 90; i++) {sequence(); }	//90*10secSeq=900/60sec=15 min 28sec
  sendData(T,H);	 // logdata to Google drive sheet every 15 min (96/Day). Limit <= 1000 API requests / day.
}

void sequence(){
printLocalTime();                                   //Serial print once per sequence
display.clearDisplay();delay(1000);printTime();  
display.clearDisplay();delay(1000);printLocalDay();
display.clearDisplay();delay(1000);printTime();
display.clearDisplay();delay(1000);printLocalDate();
display.clearDisplay();delay(1000);printTime();
display.clearDisplay();delay(1000);printAmbience(); //10 sec/loop i2c Oled custom sequence
}

void sendData( int T, int H)
{
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/pushingbox?";
  url += "devid=";
  url += "<PushboxAPI>&Temperature=";
  url +=  T ;
  url += "&Humidity=";
  url +=  H ;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 4000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
    Serial.print("Data Sent!");
  }
  
  Serial.println();
  Serial.println("closing connection");
  Serial.println("----------------------------------------------");
  display.clearDisplay();display.setCursor(0,25);display.setTextSize(2);display.print("Data Sent!");display.display();
  delay(1000);
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
