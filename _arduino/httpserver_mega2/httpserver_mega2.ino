/*
 * WiFlyHQ Example httpserver.ino
 *
 * This sketch implements a simple Web server that waits for requests
 * and serves up a small form asking for a username, then when the
 * client posts that form the server sends a greeting page with the
 * user's name and an analog reading.
 *
 * This sketch is released to the public domain.
 *
 */

/* Notes:
 * Uses chunked message bodies to work around a problem where
 * the WiFly will not handle the close() of a client initiated
 * TCP connection. It fails to send the FIN to the client.
 * (WiFly RN-XV Firmware version 2.32).
 */

/* Work around a bug with PROGMEM and PSTR where the compiler always
 * generates warnings.
 */
#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 
#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 

#include <WiFlyHQ.h>

#include <SoftwareSerial.h>
//SoftwareSerial wifiSerial(6,7);

//#include <AltSoftSerial.h>
//AltSoftSerial wifiSerial(8,9);

WiFly wifly;

/* Change these to match your WiFi network */
const char mySSID[] = "myssid";
const char myPassword[] = "my-wpa-password";

void sendIndex();
void sendGreeting(char *name);
void send404();

char buf[80];

int mode;
unsigned long modeStartTime;


void setup()
{
  Serial.begin(115200);
  Serial.println(F("Starting"));
  Serial.print(F("Free memory: "));
  Serial.println(wifly.getFreeMemory(),DEC);


  Serial1.begin(9600);

  //wifiSerial.begin(9600);
  if (!wifly.begin(&Serial1, &Serial)) {
    Serial.println(F("Failed to start wifly"));
    wifly.terminal();
  }

  /* Join wifi network if not already associated */
  if (!wifly.isAssociated()) {
    /* Setup the WiFly to connect to a wifi network */
    Serial.println(F("Joining network"));
    wifly.setSSID(("yesyesno"));
    wifly.setPassphrase(("chinatown"));
    wifly.enableDHCP();
    wifly.save();

    if (wifly.join()) {
      Serial.println(F("Joined wifi network"));
    } 
    else {
      Serial.println(F("Failed to join wifi network"));
      wifly.terminal();
    }
  } 
  else {
    Serial.println(F("Already joined network"));
  }

  wifly.setBroadcastInterval(0);	// Turn off UPD broadcast

  //wifly.terminal();

  Serial.print(F("MAC: "));
  Serial.println(wifly.getMAC(buf, sizeof(buf)));
  Serial.print(F("IP: "));
  Serial.println(wifly.getIP(buf, sizeof(buf)));

  wifly.setDeviceID("Wifly-WebServer");

  if (wifly.isConnected()) {
    Serial.println(F("Old connection active. Closing"));
    wifly.close();
  }

  wifly.setProtocol(WIFLY_PROTOCOL_TCP);
  if (wifly.getPort() != 80) {
    wifly.setPort(80);
    /* local port does not take effect until the WiFly has rebooted (2.32) */
    wifly.save();
    Serial.println(F("Set port to 80, rebooting to make it work"));
    wifly.reboot();
    delay(3000);
  }


  pinMode(2, OUTPUT);  
  pinMode(3, OUTPUT); 
  pinMode(4, OUTPUT); 
  pinMode(5, OUTPUT); 
  mode = 1;
  modeStartTime = millis();
  digitalWrite(2,HIGH);
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);


  Serial.println(("Ready"));



}

void loop(){

  //----------------------------------- mode logic



  if (millis() - modeStartTime > 4000){
    mode = -1;
  }


  //Serial.println(mode);

  switch(mode){
  case -1:
    for (int i = 0; i < 4; i++){
      digitalWrite(i+2,LOW);
    }
    break;
  case 0:
  case 1: 
  case 2:
  case 3:
    for (int i = 0; i < 4; i++){
      if (i == mode) digitalWrite(i+2,HIGH);
      else digitalWrite(i+2,LOW);
    }
    break;
  }

  //----------------------------------- 

//Serial.println("a");

  if (wifly.available() > 0) {
    if (wifly.gets(buf, sizeof(buf))) {
      if (strncmp_P(buf, PSTR("GET / "), 6) == 0) {
        /* GET request */

        Serial.println(F("Got GET request"));
        while (wifly.gets(buf, sizeof(buf), 2000) > 0) {
          /* Skip rest of request */
        }
        sendIndex();
        Serial.println(F("Sent index page"));
      } 
      else {

        
        
        if (strncmp_P(buf, PSTR("GET /1.1"), 8) == 0) {
          mode = 0;
          modeStartTime = millis();
          Serial.print(F("Unexpected: "));
          Serial.println(buf);
          wifly.flushRx();		// discard rest of input
          Serial.println(F("Sending 404"));
          sendGreeting("y");
          return;
        } 
        else if (strncmp_P(buf, PSTR("GET /2.2"), 8) == 0) {
          Serial.println(F("right"));
          mode = 1;
          modeStartTime = millis();
          while (wifly.gets(buf, sizeof(buf)) > 0) {
            /* Skip rest of request */
          }
        sendGreeting("y");
          //sendIndex();
          return;
        } 
        else if (strncmp_P(buf, PSTR("GET /3.3"), 8) == 0) {
          Serial.println(F("up"));
          mode = 2;
          modeStartTime = millis();
          while (wifly.gets(buf, sizeof(buf)) > 0) {
            /* Skip rest of request */
          }
           sendGreeting("y");
          //sendIndex();
          return;
        } 
        else if (strncmp_P(buf, PSTR("GET /4.4"), 8) == 0) {
          Serial.println(F("down"));
          mode = 3;
          modeStartTime = millis();
          while (wifly.gets(buf, sizeof(buf)) > 0) {
            /* Skip rest of request */
          }
          sendGreeting("y");
          //sendIndex();
          return;
        } 
        else if (strncmp_P(buf, PSTR("GET /fav"), 8) == 0) {
          //Serial.println("favicon");
          while (wifly.gets(buf, sizeof(buf)) > 0) {
            /* Skip rest of request */
          }		// discard rest of input

          //Serial.println(F("Sending 404"));
          send404();
          return;
        }

        /* Unexpected request */
        Serial.print(F("Unexpected: "));
        Serial.println(buf);
        wifly.flushRx();		// discard rest of input
        Serial.println(F("Sending 404"));
        send404();
      }
    }
  }
}


/** Send an index HTML page with an input box for a username */
void sendIndex()
{
  /* Send the header direclty with print */
  wifly.println(F("HTTP/1.1 200 OK"));
  wifly.println(F("Content-Type: text/html"));
  wifly.println(F("Transfer-Encoding: chunked"));
  wifly.println();

  /* Send the body using the chunked protocol so the client knows when
   * the message is finished.
   * Note: we're not simply doing a close() because in version 2.32
   * firmware the close() does not work for client TCP streams.
   */
  wifly.sendChunkln(F("<html>"));


  wifly.sendChunkln(F("<title>WiFly HTTP Server Example</title>"));

  wifly.sendChunkln(F("<head>"));

  wifly.sendChunkln(F("<script>function loadXMLDoc(page){var e;if(window.XMLHttpRequest){e=new XMLHttpRequest}else{e=new ActiveXObject(\"Microsoft.XMLHTTP\")}e.onreadystatechange=function(){if(e.readyState==4&&e.status==200){}};e.open(\"GET\", page, true);e.send()}</script>"));

  wifly.sendChunkln(F("</head>"));

  wifly.sendChunkln(F("<body>"));

  wifly.sendChunkln(F("<h1>"));
  wifly.sendChunkln(F("<p>wot??!</p>"));
  wifly.sendChunkln(F("</h1>"));

  wifly.sendChunkln(F("<button type=\"button\" onclick=\"loadXMLDoc('1.1')\">pin 2</button>"));
  wifly.sendChunkln(F("<button type=\"button\" onclick=\"loadXMLDoc('2.2')\">pin 3</button>"));
  wifly.sendChunkln(F("<button type=\"button\" onclick=\"loadXMLDoc('3.3')\">pin 4</button>"));
  wifly.sendChunkln(F("<button type=\"button\" onclick=\"loadXMLDoc('4.4')\">pin 5</button>"));

  wifly.sendChunkln(F("</body>"));
  wifly.sendChunkln(F("</html>"));
  wifly.sendChunkln();
}

/** Send a greeting HTML page with the user's name and an analog reading */
void sendGreeting(char *name)
{
  /* Send the header directly with print */
  wifly.println(F("HTTP/1.1 200 OK"));
  wifly.println(F("Content-Type: text/html"));
  wifly.println(F("Transfer-Encoding: chunked"));
  wifly.println();

  /* Send the body using the chunked protocol so the client knows when
   * the message is finished.
   */
  wifly.sendChunkln(F("<html>"));
  wifly.sendChunkln(F("<title>WiFly HTTP Server Example</title>"));
  /* No newlines on the next parts */
  wifly.sendChunk(F("<h1><p>Hello "));
  wifly.sendChunk(name);
  /* Finish the paragraph and heading */
  wifly.sendChunkln(F("</p></h1>"));

  /* Include a reading from Analog pin 0 */
  snprintf_P(buf, sizeof(buf), PSTR("<p>Analog0=%d</p>"), analogRead(A0));
  wifly.sendChunkln(buf);

  wifly.sendChunkln(F("</html>"));
  wifly.sendChunkln();
}

/** Send a 404 error */
void send404()
{
  wifly.println(F("HTTP/1.1 404 Not Found"));
  wifly.println(F("Content-Type: text/html"));
  wifly.println(F("Transfer-Encoding: chunked"));
  wifly.println();
  wifly.sendChunkln(F("<html><head>"));
  wifly.sendChunkln(F("<title>404 Not Found</title>"));
  wifly.sendChunkln(F("</head><body>"));
  wifly.sendChunkln(F("<h1>Not Found</h1>"));
  wifly.sendChunkln(F("<hr>"));
  wifly.sendChunkln(F("</body></html>"));
  wifly.sendChunkln();
}

