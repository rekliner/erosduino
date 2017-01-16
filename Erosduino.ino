#define notnetEnabled
#define notwifiClient
#define notamica

#ifdef netEnabled
#ifdef amica
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <DNSServer.h>
  #include <WiFiUdp.h>
  WiFiUDP Udp;

  #define MAX_SRV_CLIENTS 1
  WiFiServer telnetServer(23);
  WiFiClient serverClients[MAX_SRV_CLIENTS];


  

#endif

#ifdef breakout8266
  #include <WiFiEsp.h>
  #include <WiFiEspClient.h>
  #include <WiFiEspServer.h>
  #include <WiFiEspUdp.h>
  WiFiEspUDP Udp;
#endif

  IPAddress apIP(192, 168, 1, 1);
 
#ifdef wifiServer
  const byte DNS_PORT = 53;
  DNSServer dnsServer;
  ESP8266WebServer webServer(80);
  char ssid[] = "StimFu";          // your network SSID (name)
  char pass[] = "";                    // your network password
  int status = WL_IDLE_STATUS;     // the Wifi radio's status
void handleRoot() {
  String response = "<h1>You are connected!</h1>";
  webServer.send(200, "text/html", "<h1>You are connected!</h1>");
}
#endif
#ifdef wifiClient
  char ssid[] = "ssid";          // your network SSID (name)
  char pass[] = "pass";                    // your network password
#endif

  // A UDP instance to let us send and receive packets over UDP
  const unsigned int outPort = 9999;          // remote port (not needed for receive)
  const unsigned int localPort = 8888;        // local port to listen for UDP packets (here's where we send the packets)

#endif

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <EEPROM.h>

#include <Venerate.h>

#include <SoftwareSerial.h>





OSCErrorCode error;
//unsigned int ledState = LOW;              // LOW means led is *on*
#ifndef amica
const int BUILTIN_LED = 13;
#endif

int ledPin =  BUILTIN_LED;       //pin 13 on Arduino Uno. Pin 6 on a Teensy++2
int ledState = LOW;

unsigned long nextUpdate = 0;

unsigned long starttime;
//SoftwareSerial bugSerial(5,4);//(14, 12, false, 256); 
SoftwareSerial erosSerial(5,4);//(14, 12, false, 256); 
//HardwareSerial & erosSerial = Serial;
HardwareSerial & bugSerial = Serial;
//WiFiClient & bugSerial = serverClients[0];
Venerate EBOX = Venerate(0);


#include <FastLED.h>  //  from https://github.com/FastLED/FastLED/
    #define NUM_LEDS 60
    #define DATA_PIN 6
const long numleds = 144;
CRGB LEDA[numleds];

void clearleds() {
  FastLED.clear();
}

void setup() {//<NEOPIXEL, ledapin>
  //FastLED.addLeds(LEDA, numleds);
  //FastLED.addLeds<NEOPIXEL, ledapin>(LEDA, numleds);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(LEDA, numleds);
  clearleds();
  
  erosSerial.begin(19200);
  pinMode(BUILTIN_LED, OUTPUT);
  bugSerial.begin(115200);
  //bugSerial.begin(9600);
  pinMode(4,OUTPUT);
  //  pinMode(7,OUTPUT);
  //  digitalWrite(8, LOW);
  
 #ifdef netEnabled
  // Connect to WiFi network

  #ifdef amica
    EEPROM.begin(16);

    #ifdef wifiClient
      WiFi.begin(ssid, pass);
      bugSerial.print("\nConnecting to "); bugSerial.println(ssid);
      uint8_t i = 0;
      while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
      if(i == 21){
        bugSerial.print("Could not connect to"); bugSerial.println(ssid);
        while(1) delay(500);
      }
      bugSerial.println("WiFi connected");
      bugSerial.println("IP address: ");
      bugSerial.println(WiFi.localIP());
    
    #endif

    #ifdef wifiServer
      bugSerial.print("Configuring access point...");
      /* You can remove the password parameter if you want the AP to be open. */
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP(ssid);
    
      dnsServer.setTTL(300);
      dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
      dnsServer.start(DNS_PORT, "*", apIP);

      IPAddress myIP = WiFi.softAPIP();
      bugSerial.print("AP IP address: ");
      bugSerial.println(myIP);
      //webServer.on("/", handleRoot);
      String responseHTML = "";
      responseHTML +=  "<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body><h1>Hello World!</h1><p>This is a captive portal example. All requests will be redirected here.</p></body></html>";    
      webServer.onNotFound([]() {
        webServer.send(200, "text/html", "<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body><h1>Hello World!</h1><p>This is a captive portal example. All requests will be redirected here.</p></body></html>");    
        
      });      
      webServer.begin();
      bugSerial.println("HTTP server started");

    #endif
  #endif

  #ifdef breakout8266
    SoftwareSerial SS3(11,7);//(14, 12, false, 256); 
    SS3.begin(9600);
    // check for the presence of the shield
    WiFi.init(&SS3);
    if (WiFi.status() == WL_NO_SHIELD) {
      bugSerial.println("WiFi shield not present");
      // don't continue
      while (true);
    }
    
     // attempt to connect to WiFi network
    while ( status != WL_CONNECTED) {
      bugSerial.print("Attempting to connect to WPA SSID: ");
      bugSerial.println(ssid);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(ssid, pass);
    }

  #endif



  bugSerial.println("Starting UDP");
  Udp.begin(localPort);
  bugSerial.print("Local port: ");
  bugSerial.println(localPort);

  if (!MDNS.begin("osc")) {
    bugSerial.println("Error setting up MDNS responder!");
  } else {
    bugSerial.println("mDNS responder started");
    MDNS.addService("osc", "udp", 8888);
    MDNS.addService("telnet", "tcp", 23);
    MDNS.addService("http", "tcp", 80);
  }

    telnetServer.begin();
    telnetServer.setNoDelay(true);

 #endif

  starttime = millis();
  EBOX.begin(erosSerial);
//  bugSerial.print("Stored MOD=");bugSerial.println(EEPROM.read(0),HEX);
  //EBOX.setmod(EEPROM.read(0));
  EBOX.setdebug(bugSerial, 0);
    digitalWrite(BUILTIN_LED, ledState);    // turn *on* led

}


void OSCsend(const char* destination,int value) {
/*
  char* _dest;
  if (destination[0] != '/') 
      sprintf(_dest,"/%s",destination);
    else
      sprintf(_dest,"%s",destination);
  OSCMessage msgOUT(_dest);
*/
  OSCMessage msgOUT(destination);
  msgOUT.add(value);
  //const IPAddress blankIP(0,0,0,0);
  
#ifdef netEnabled
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255;

  Udp.beginPacket(broadcastIP, outPort);
 // serverClients[0].print(_dest);
 // serverClients[0].print(" = ");
 // serverClients[0].print(value);
 // serverClients[0].print("\n");

  msgOUT.send(Udp); // send the bytes
  bugSerial.print(destination);bugSerial.print("\t");bugSerial.println(value);
  Udp.endPacket(); // mark the end of the OSC Packet
  msgOUT.empty(); // free space occupied by message  
#endif
}


int heartbeatValue = 0;
#ifdef netEnabled
IPAddress broadcastIP = apIP;
void sendHeartbeat() {
      OSCMessage heartbeat = "/heartbeat";
      heartbeat.add(heartbeatValue);
      broadcastIP[3] = 255;
      Udp.beginPacket(broadcastIP, outPort);
      heartbeat.send(Udp);
      Udp.endPacket();
      bugSerial.print(millis());bugSerial.print(" /heartbeat ");bugSerial.println(heartbeatValue);
      heartbeat.empty();
     if (heartbeatValue) {
        heartbeatValue = 0;
     } else {
        heartbeatValue = 1;
     }
}
#endif

void OSCMsgReceive(){
  OSCMessage msgIN;
  int size;
  #ifdef netEnabled
  if((size = Udp.parsePacket())>0){
    while(size--)
      msgIN.fill(Udp.read());
    if(!msgIN.hasError()){
      //ignore brodcasts here...check ip
      if (Udp.remoteIP()[3] == 255) return; //ignore broadcasts
       for (int i =0;i < (sizeof(erosParamIdx)/sizeof(int)); i++) {
         if(msgIN.match(erosParamName[i]) > 0) {
            bugSerial.print("Incoming! (");bugSerial.print(erosParamIdx[i],HEX);
            bugSerial.print(") ");
            bugSerial.print(erosParamName[i]);
            bugSerial.print(" ");
            bugSerial.println(int(msgIN.getFloat(0)));
            EBOX.setbyte(erosParamIdx[i],int(msgIN.getFloat(0)));
         }

       }
       if (msgIN.match("/heartbeat")) sendHeartbeat();
      /*
      msgIN.route("/OnOff/toggle1",toggleOnOff);
      msgIN.route("/Fader/Value",funcValue);
      */
    }
  }
  #endif
}



#ifdef netEnabled
void telnetReceive() {
    //check if there are any new clients
    int i;
  if (telnetServer.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = telnetServer.available();
        bugSerial.print("New client: "); bugSerial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = telnetServer.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) bugSerial.write(serverClients[i].read());
      }
    }
  }
}
#endif
int erosParamIdx[] = {ETMEM_freqa,ETMEM_freqb,ETMEM_widtha,ETMEM_widthb,ETMEM_mode,ETMEM_knoba,ETMEM_knobb,ETMEM_knobma,ETMEM_gatea,ETMEM_gateb,ETMEM_levela,ETMEM_levelb,ETMEM_powerlevel};
const char *erosParamName[] = {"/freqa","/freqb","/widtha","/widthb","/mode","/knoba","/knobb","/knobma","/gatea","/gateb","/levela","/levelb","/powerlevel"};
int erosParamVal[] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
int erosModeIdx[] = {ETMODE_waves,ETMODE_stroke,ETMODE_climb,ETMODE_combo,ETMODE_intense,ETMODE_rhythm,ETMODE_audio1,ETMODE_audio2,ETMODE_audio3,ETMODE_split,ETMODE_random1,ETMODE_random2,ETMODE_toggle,ETMODE_orgasm,ETMODE_torment,ETMODE_phase1,ETMODE_phase2,ETMODE_phase3,ETMODE_user1,ETMODE_user2,ETMODE_user3,ETMODE_user4,ETMODE_user5};
const char *erosModeName[] = {"waves","stroke","climb","combo","intense","rhythm","audio1","audio2","audio3","split","random1","random2","toggle","orgasm","torment","phase1","phase2","phase3","user1","user2","user3","user4","user5","no mode"};



void loop() {

      //OSCMsgReceive();
      
  int i;
  #ifdef amica
      telnetReceive();

  #endif
  #ifdef webServer
    dnsServer.processNextRequest();
  webServer.handleClient();
  #endif
  if (EBOX.isconnected()) {

//    levela = max(0,levela-128) + max(0,widtha-128);
//    levelb = max(0,levelb-128) + max(0,widthb-128);
//    int intensitya = max(20,255-freqa); //max(0,freqa-64)+64;
//    int intensityb = max(20,255-freqb);  //max(0,freqb-64)+64;


    for (i =0;i < (sizeof(erosParamIdx)/sizeof(int)); i++) 
        erosParamVal[i] = EBOX.getbyte(erosParamIdx[i]);
        
    OSCBundle bndl;

    //bugSerial.print('{');
    //if (nextUpdate < millis()) {
      for (i =0;i < (sizeof(erosParamIdx)/sizeof(int)); i++) {  
      
    

/*
        for(int ii = 0; ii < MAX_SRV_CLIENTS; ii++){
          if (serverClients[ii] && serverClients[ii].connected()){
            serverClients[ii].print(erosParamName[i]);
            serverClients[ii].print("\t=\t");
            serverClients[ii].print(erosParamVal[i]);
            serverClients[ii].print("\n");
           //delay(1);
          }
        }
 */     
        //bugSerial.print(i,HEX);bugSerial.print("/");bugSerial.print(sizeof(erosParamIdx)/sizeof(int));bugSerial.print(": ");
        if (erosParamIdx[i] == ETMEM_mode) {
          if (erosParamVal[i] != 0) 
            //erosParamVal[i] = 23; //mode is 0 at intro screen, mark this as no mode
          //else
            erosParamVal[i] -= 118; //otherwise subtract 0x78 to line up mode with name array
          
          //bugSerial.print("(");bugSerial.print(erosModeName[erosParamVal[i]]);bugSerial.print(") ");
          //bndl.add(erosParamName[i]).add(erosParamVal[i]]);
          const char * modeName = "/modeName";
          //bndl.add(const_cast<char*>(modeName)).add(erosModeName[erosParamVal[i]]);
        } else {
         
        }
        //bndl.add(const_cast<char*>(erosParamName[i])).add(erosParamVal[i]);
        //bugSerial.print(erosParamName[i]);bugSerial.print("=");bugSerial.print(erosParamVal[i]);
       // bugSerial.print(i,HEX);
        bugSerial.print(erosParamVal[i],HEX);
        if (i < (sizeof(erosParamIdx)/sizeof(int)) - 1) bugSerial.print(",");
        //bugSerial.println(" ");
    }
    //bugSerial.println("EOP");
    bugSerial.print('|');
    
    
    #ifdef netEnabled
    Udp.beginPacket(broadcastIP, outPort);

    bndl.send(Udp);
    Udp.endPacket();
    //delay(500);
    bndl.empty();
    #endif
    
    nextUpdate = millis() + 2000;

    //}  //end update
    
    if ((millis() - starttime) > 2000) {
      starttime = millis();
      #ifdef netEnabled
        sendHeartbeat();   
      #endif  

    }
    
    bargraph();
    
  } else {  //if the eros is not connected
    if ((millis() - starttime) > 2000) {
      starttime = millis();
//      bugSerial.println("No sync with EROS yet...");
       
      #ifdef netEnabled
        serverClients[0].println("No sync with EROS yet...");
        sendHeartbeat();   
      #endif  
      EBOX.hello();
    }
  }
  
}  




CRGB h1v_to_rgb(byte inh, byte inv)
{
    CRGB rgb;
    unsigned char region, p, q, t;
    unsigned int h, s, v, remainder;

    h = inh;
    v = inv;
    region = h / 43;
    remainder = (h - (region * 43)) * 6; 
    //p = (v * (255 - s)) >> 8;
    //q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    //t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    // we know s is always full, so just cheat save some cpu
    p = 0;
    q = (v * (255 - remainder)) >> 8;
    t = (v * remainder) >> 8;
    
    switch (region)  {
        case 0:
            rgb.r = v;            rgb.g = t;            rgb.b = p;
            break;
        case 1:
            rgb.r = q;            rgb.g = v;            rgb.b = p;
            break;
        case 2:
            rgb.r = p;            rgb.g = v;            rgb.b = t;
            break;
        case 3:
            rgb.r = p;            rgb.g = q;            rgb.b = v;
            break;
        case 4:
            rgb.r = t;            rgb.g = p;            rgb.b = v;
            break;
        default:
            rgb.r = v;            rgb.g = p;            rgb.b = q;
            break;
    }
    return rgb;
}

// Save some serial comms, only grab a/b knob values every few times
// around, so let's global remember some stuff

int knobb = 0;
int knoba = 0;
int barcount = 0;

void bargraph(void) {
    int levelb,levela,widtha,freqa,widthb,freqb;

    byte gateb = EBOX.getbyte(ETMEM_gateb) & 1;
    byte gatea = EBOX.getbyte(ETMEM_gatea) & 1;

    if (gateb == 1) {
        levelb = EBOX.getbyte(ETMEM_levelb);
        widthb = EBOX.getbyte(ETMEM_widthb);
        freqb = EBOX.getbyte(ETMEM_freqb);
    } else {
        levelb = widthb = freqb = 0;
    }
    if (gatea == 1) {
        levela = EBOX.getbyte(ETMEM_levela);
        widtha = EBOX.getbyte(ETMEM_widtha);
        freqa = EBOX.getbyte(ETMEM_freqa);
    } else {
        levela = widtha = freqa = 0;
    }
    if (knoba == 0 || knobb == 0 || barcount++ > 5) {
      int lastb = knobb;
      int lasta = knoba;
      knobb = EBOX.getbyte(ETMEM_knobb);
      knoba = EBOX.getbyte(ETMEM_knoba);
      if (lasta == knoba && lastb == knobb) {
        barcount = 0;
      }
    }
    // let's try the frequency is the brightness, but the knob is the colour
    // int intensitya = max(10,(max(widtha-128,0) *4 + max(freqa-128,0)*2)/3);
    // int intensityb = max(10,(max(widthb-128,0) *4 + max(freqb-128,0)*2)/3);

    levela = max(0,levela-128) + max(0,widtha-128);
    levelb = max(0,levelb-128) + max(0,widthb-128);

    int intensitya = widtha-64;//max(0,freqa-64)+64;//max(20,255-freqa); //
    int intensityb = max(20,255-freqb);  //max(0,freqb-64)+64;

    // FastLED uses rainbow colour map with 96 as green through to 255 as red

    // we want 255 redish and 0 greenish so lets shift
    // the hue so 255 is red and knob <24 is just floor near green
    byte hue = knobb+10;  // overflow okay
    if (knobb < 62) hue = 74;
    CRGB value = h1v_to_rgb( hue, intensityb);    

    for (int i = 0; i < numleds; i++) {
      if (levelb <= i * (128/numleds)) value = CRGB::Black;
      //LEDB[numleds-1-i] = value;
    }

    hue = min(knoba+64,256); // overflow okay
    if (knoba < 62) hue = 74;
    value = h1v_to_rgb( hue, intensitya);  

    for (int i = 0; i < numleds; i++) {
      if ((intensitya)*numleds/128 <= i) value = CRGB::Black;
      LEDA[numleds-1-i] = value;
    }
    FastLED.show();
    //Serial.print("numleds=");Serial.print(numleds);Serial.print("inta=");Serial.println(intensitya);
}

byte cylon = 0;

// The main loop; if we're not already connected to a box, try to
// connect to a box.  If we're connected then grab some values and
// light up some LEDs

byte nowtime = 0;


