//Azure IoT Hub + DHT11 + NodeMCU ESP8266 Experiment Done By Prasenjit Saha
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_ST7735.h>   // include Adafruit ST7735 TFT library

// WiFi settings
const char* ssid = "Familia TM";
const char* password = "fichopreple";
const char * headerKeys[] = {"ETag"};
const size_t numberOfHeaders = 1;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "south-america.pool.ntp.org", -14400, 60000);

#define TFT_RST   -1      // TFT RST pin is connected to arduino pin 8
#define TFT_CS    15      // TFT CS  pin is connected to arduino pin 9
#define TFT_DC    2     // TFT DC  pin is connected to arduino pin 10
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define DHTPIN 5
// Digital pin connected to the DHT sensor

//Azure IoT Hub
const String AzureIoTHubURI="https://IoTHub-TecnoUpsa.azure-devices.net/devices/esp-02/messages/events?api-version=2020-03-13";
const String AzureIoTHubURIGet="https://IoTHub-TecnoUpsa.azure-devices.net/devices/esp-02/messages/deviceBound?api-version=2020-03-13";
const String AzureIoTHubURIDel="https://IoTHub-TecnoUpsa.azure-devices.net/devices/esp-02/messages/deviceBound/{etag}?api-version=2020-03-13";
//openssl s_client -servername myioteventhub.azure-devices.net -connect myioteventhub.azure-devices.net:443 | openssl x509 -fingerprint -noout //
const String AzureIoTHubFingerPrint="9C:46:08:5D:55:5C:F7:83:79:0A:03:BD:DE:F4:54:F2:C9:E6:FF:9C"; 
//az iot hub generate-sas-token --device-id {YourIoTDeviceId} --hub-name {YourIoTHubName} 
const String AzureIoTHubAuth="SharedAccessSignature sr=IoTHub-TecnoUpsa.azure-devices.net%2Fdevices%2Fesp-02&sig=XWLCBzP6ZOn%2B2nAsR%2Fjsnhei17q5F9yrdXdqIf6ubRA%3D&se=1623479220";

#define DHTTYPE DHT11  

DHT dht(DHTPIN, DHTTYPE);

unsigned long contAux;
unsigned long cont;
String banderaBomba="false";
String banderaBuzzer="false";
String banderaAux="false";
#define BA 4
#define BU 12

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();
  // Init serial line
 // Serial.begin(115200);
  Serial.println("ESP8266 starting in normal mode");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Print the IP address
  Serial.println(WiFi.localIP());

  tft.initR(INITR_BLACKTAB);     // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);  // fill screen with black color
  tft.drawFastHLine(0, 50,  tft.width(), ST7735_BLUE);   // draw horizontal blue line at position (0, 50)
  tft.drawFastHLine(0, 102,  tft.width(), ST7735_BLUE);  // draw horizontal blue line at position (0, 102)

  tft.setTextColor(ST7735_GREEN, ST7735_BLACK);     // set text color to green and black background
  tft.setCursor(25, 8);              // move cursor to position (25, 61) pixel
  tft.print("TEMPERATURA =");
  tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);  // set text color to yellow and black background
  tft.setCursor(34, 61);              // move cursor to position (34, 113) pixel
  tft.print("HUMEDAD =");
  tft.setTextSize(2);                 // text size = 2

  timeClient.begin();

  pinMode(BA, OUTPUT);
  digitalWrite(BA, LOW);
  pinMode(BU, OUTPUT);
  digitalWrite(BU, LOW);
}

char _buffer[7];

void loop() {
  
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  contAux=epochTime;
  funcionamiento(epochTime);

  int monthDay = ptm->tm_mday;
  String m;
  int currentMonth = ptm->tm_mon+1;
  if(currentMonth<10)
  {m="0"+String(currentMonth);}
  else
  {m=String(currentMonth);}

  int currentYear = ptm->tm_year+1900;
  
  String currentDate = String(currentYear) + "-" + m + "-" + String(monthDay)+"T"+formattedTime;
  
  // Wait a few seconds between measurements.
  delay(1000);
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  byte humi = dht.readHumidity();
  // read temperature
  byte temp = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  sprintf(_buffer, "%02u.0", temp);
  tft.setTextColor(ST7735_RED, ST7735_BLACK);  // set text color to red and black background
  tft.setCursor(29, 25);
  tft.print(_buffer);
  tft.drawCircle(83, 27, 2, ST7735_RED);  // print degree symbol ( ° )
  tft.setCursor(89, 25);
  tft.print("C");

  // print humidity (in %)
  sprintf(_buffer, "%02u.0 %%", humi);
  tft.setTextColor(ST7735_CYAN, ST7735_BLACK);  // set text color to cyan and black background
  tft.setCursor(29, 78);
  tft.print(_buffer);

  String PostData="{\"NameDevice\":\"ESP-02\",\"EventDateTime\":'"+String(currentDate)+"',\"Temperatura\":'"+String(t)+"',\"Humedad\":'"+String(h)+"',\"BombaAgua\":'"+String(banderaBomba)+"',\"Buzzer\":'"+String(banderaBuzzer)+"'}";
      Serial.println(PostData);
      // Send data to cloud
      int returnCode=RestPostData(AzureIoTHubURI,AzureIoTHubFingerPrint,AzureIoTHubAuth,PostData);
      Serial.println(returnCode);
      String mensaje=RestGetData(AzureIoTHubURIGet,AzureIoTHubFingerPrint,AzureIoTHubAuth);
      Serial.println(mensaje);

} //Fin Loop

// Functions
int RestPostData(String URI, String fingerPrint, String Authorization, String PostData)
{
    HTTPClient http;
    http.begin(URI,fingerPrint);
    http.addHeader("Authorization",Authorization);
    http.addHeader("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    int returnCode=http.POST(PostData);
    if(returnCode<0) 
  {
    Serial.println("RestPostData: Error sending data: "+String(http.errorToString(returnCode).c_str()));
  }
    http.end();
  return returnCode;
}

String RestGetData(String URI, String fingerPrint, String Authorization)
{
    HTTPClient http;
    http.begin(URI,fingerPrint);
    http.addHeader("Authorization",Authorization);
    http.addHeader("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    http.collectHeaders(headerKeys, numberOfHeaders);
    int httpCode = http.GET();
    String payload;
    if(httpCode<0) 
  {
    Serial.println("RestGetData: Error getting data: "+String(http.errorToString(httpCode).c_str()));
  }
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String eTag = http.header("ETag");
        String newEtag=trimETag(eTag);
        payload = http.getString();
        controlador(payload,contAux);
        String urlNew=AzureIoTHubURIDel;
        urlNew.replace("etag",newEtag);
        int httpdel=RestDelData(urlNew,AzureIoTHubFingerPrint,AzureIoTHubAuth);
        Serial.println(httpdel);
    }
    else{
    payload=String(httpCode);}
    http.end();
  return payload;
}

int RestDelData(String URI, String fingerPrint, String Authorization)
{
    HTTPClient http;
    http.begin(URI,fingerPrint);
    http.addHeader("Authorization",Authorization);
    http.addHeader("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    int httpCode = http.sendRequest("DELETE");;
    if(httpCode<0) 
  {
    Serial.println("RestDelData: Error getting data: "+String(http.errorToString(httpCode).c_str()));
  }
    http.end();
  return httpCode;
}

String trimETag(String value)
{
    String retVal=value;

    if(value.startsWith("\""))
      retVal=value.substring(1);

    if(value.endsWith("\""))
      retVal=retVal.substring(0,retVal.length()-1);

    return retVal;     
}

void controlador(String flag,unsigned long t){
  if(flag=="on"){
        banderaBomba="true";
        banderaBuzzer="true";
        cont=t+8;
    }
  else if(flag=="off"){
        banderaBomba="false";
        banderaBuzzer="false";
    }
  }

void funcionamiento(unsigned long t){
    if(banderaBomba=="true"){
    digitalWrite(BA, HIGH);
    banderaBuzzer="true";
    digitalWrite(BU, HIGH);
      if(t>cont){
        digitalWrite(BU, LOW);
        banderaBuzzer="false";}
      }
  else if(banderaBomba=="false"){
    digitalWrite(BA, LOW);
    }
  }

  
