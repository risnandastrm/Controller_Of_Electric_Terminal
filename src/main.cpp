#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <RtcDS3231.h>

// Variable Initial
#define relayOne D0
#define relayTwo D3
bool LED1status = LOW;
bool LED2status = LOW;
//Wi-Fi things
const char* ssid = "Qwerty";
const char* password = "makerjinx";
ESP8266WebServer server(80);
//RTC things
RtcDS3231<TwoWire> rtcObject(Wire);
String switch1timeron = "NOTSET" , switch1timeroff = "NOTSET", switch2timeron = "NOTSET", switch2timeroff = "NOTSET" ,timingHour = " " ,timingMinute =" ",timingSecond=" ";

//========================WEB TEMPLATE========================
String SendHTML(uint8_t led1stat, uint8_t led2stat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"; // ratio WEB
  ptr += "<title>Smart Plug</title>\n"; // JUDUL
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +="input[type=time] {width: 100%;padding: 12px 20px;margin: 8px 0;box-sizing: border-box;border: none;background-color: #3CBC8D;color: white;}";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-apply {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 10px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr += "<h1>Smart Plug</h1>\n";
  ptr += "<h3>Internet Of Things for Smart Plug</h3>\n";
  ptr += "<p> TIME " + timingHour + ":" +timingMinute+ ":"+timingSecond+" </p>"; 
  ptr += "</h4>\n";


  if (led1stat){
    ptr += "<p>Switch 1 Status: ON</p>  <a class=\"button button-off\" href=\"/led1off\">OFF</a>    \n";}
  else{
    ptr += "<p>Switch 1 Status: OFF</p> <a class=\"button button-on\" href=\"/led1on\">ON</a>       \n";}

  if (led2stat){
    ptr += "<p>Switch 2 Status: ON</p>  <a class=\"button button-off\" href=\"/led2off\">OFF</a>    \n";}
  else{
    ptr += "<p>Switch 2 Status: OFF</p> <a class=\"button button-on\" href=\"/led2on\">ON</a>       \n";}

  ptr += "<center><table><form action=\"/action_page\">";
  ptr += "<tr><td><p>Switch 1 Timer ON : </p>    <input type=\"time\" name=\"switch1timeron\" value = " + switch1timeron + "></td>";
  ptr += "<td><p>Switch 1 Timer OFF : </p>   <input type=\"time\" name=\"switch1timeroff\" value = " + switch1timeroff + "></td></tr>";
  ptr += "<tr><td><p>Switch 2 Timer ON : </p>    <input type=\"time\" name=\"switch2timeron\" value = " + switch2timeron + " ></td>";
  ptr += "<td><p>Switch 2 Timer OFF : </p>   <input type=\"time\" name=\"switch2timeroff\" value = " + switch2timeroff + "></td></tr>";
  ptr += "<td><tr colspan = '2'><input type=\"submit\" class =\"button-apply\" Value=\"Apply\"></tr></td>";
  ptr += "</table></center></form>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";

  return ptr;
}

void showTime() {
  RtcDateTime now = rtcObject.GetDateTime();

  timingHour = now.Hour();
  timingMinute = now.Minute();
  timingSecond = now.Second();
}

void setTime() {
  RtcDateTime currentTime = RtcDateTime(16, 05, 18, 21, 20, 0); //define date and time object
  rtcObject.SetDateTime(currentTime); //configure the RTC with object
}

void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1on() {
  LED1status = HIGH;
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_led1off() {
  LED1status = LOW;
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() {
  LED2status = HIGH;
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() {
  LED2status = LOW;
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_form(){
  switch1timeron  = server.arg("switch1timeron");
  switch1timeroff = server.arg("switch1timeroff");
  switch2timeron  = server.arg("switch2timeron");
  switch2timeroff = server.arg("switch2timeroff");
  server.sendHeader("Location", "/", "true");
  server.send(302, "text/plain", "");
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found"); // untuk membawa ke page not Found
}

int returntime(){
  int me;
  me = (timingHour.toInt()*100)+timingMinute.toInt();// 9:30 . 900+30=  930
  return me;  
}

void setup(){
  //------------SETTING I/O---------------
  Serial.begin(115200);
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  
  //---------------SET RTC----------------
  rtcObject.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!rtcObject.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    rtcObject.SetDateTime(compiled);}

  RtcDateTime now = rtcObject.GetDateTime();
  rtcObject.Enable32kHzPin(false);
  rtcObject.SetSquareWavePin(DS3231SquareWavePin_ModeNone);//*/
  
  //-----------SET WI-FI--------------
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");}
    
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup server side
  server.begin();
  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.on("/action_page", handle_form);
  server.onNotFound(handle_NotFound);

  Serial.println("HTTP server started");
  
  // Show time first
  setTime();
}

//=========================MAIN PROGRAM=========================
void loop(){
  // Function for loading website page
  server.handleClient(); 

  String  me1 = switch1timeron,
          me2 = switch1timeroff,
          me3 = switch2timeron,
          me4 = switch2timeroff;

  me1.replace(":","");
  me2.replace(":","");
  me3.replace(":","");
  me4.replace(":","");

  // Show time
  showTime();

  if (me1==0||me2==0 || me3==0||me4==0){
    if(LED1status){
      digitalWrite(relayOne, HIGH);
    } else{
      digitalWrite(relayOne, LOW);
    }
  
    if(LED2status){
      digitalWrite(relayTwo, HIGH);
    } else{
      digitalWrite(relayTwo, LOW);
    }
  } else if (LED2status||me3.toInt()>=returntime()){
    digitalWrite(relayTwo, HIGH);
  } else if (!LED2status||me4.toInt()>=returntime()){
    digitalWrite(relayTwo, LOW);
  }
  
  if (LED1status || me1.toInt()>=returntime()){
    digitalWrite(relayOne, HIGH);
  } else if (!LED1status||me2.toInt()>=returntime()){
    digitalWrite(relayOne, LOW);
  }

  Serial.println(WiFi.localIP());
}