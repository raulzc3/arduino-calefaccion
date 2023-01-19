#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

const char* ssid = "NASA";             //Set your wifi network name(ssid)
const char* password = "A4bofoli0";    //Set your router password

int RelayPin = D1;          //Relay is controlled by pin D1
int relayStatus = LOW;      //Initial status of the Relay
int defaultTimeLimit = 10800;  //Time before automatic shut down (seconds)
int timeLimit = defaultTimeLimit;
int timer = 0;



WiFiServer server(80);


void setup() {
  Serial.begin(74880); //change according to your com port baud rate
  delay(10);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RelayPin, OUTPUT);

  
  digitalWrite(RelayPin, relayStatus);
  digitalWrite(LED_BUILTIN, HIGH);

  //WiFi configuration
  IPAddress ip(192, 168, 1, 100);
  IPAddress dns(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, dns, gateway, subnet);



  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(LED_BUILTIN, LOW);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}



void loop() {
 
 // Serial.print("Free Heap: ");
 // Serial.println(ESP.getFreeHeap());
 // Serial.print("Fragmentation: ");
 // Serial.println(ESP.getHeapFragmentation());

  // Check for an active client
  WiFiClient client = server.available();
  if (!client) {
    if(relayStatus == HIGH){
      delay(950);
      timer++;
      Serial.println(timeLimit);
      if(timer == timeLimit){
        relayStatus = LOW;
        digitalWrite(RelayPin, LOW);
        timer = 0;
        }
      }
    return;
  }

  // Wait until client responds
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  // Read client request
  String request = client.readStringUntil('\r');

  Serial.println(request);
  client.flush();

  // Match the request

  if (request.indexOf("/ON") != -1) {
    if (request.indexOf("/timer=") != -1) {
      int newTimer = request.substring(request.indexOf("/timer=") + 7).substring(0, 5).toInt();
      if(newTimer){
        timeLimit = newTimer;
      }else{
        timeLimit = defaultTimeLimit;
        }
   }else {
      timeLimit = defaultTimeLimit;
     }
   
     timer = 0;
     digitalWrite(RelayPin, HIGH);
     relayStatus = HIGH;
  }
  
  if (request.indexOf("/OFF") != -1){
    digitalWrite(RelayPin, LOW);
    timer = 0;
    relayStatus = LOW;
  }
  
 
client.print("<!DOCTYPE html> <html lang='es'> <head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>Calefacción</title> <style> * { box-sizing: border-box; margin: 0; padding: 0; font-family: Arial, Helvetica, sans-serif; color: white; text-align: center; } main { width: 100vw; height: 100vh; margin: 0; padding: 0; border: 2rem solid darkgray; display: flex; align-items: center; justify-content: center; gap: 1.5rem; flex-direction: column; background-color: rgb(37, 37, 37); font-size: 2rem; } #status { font-size: 3rem; } #statusSpan { font-size: .7em; text-align: center; margin-top: -2rem; } #actions { display: flex; flex-direction: column; gap: .5rem; } #actions span { margin-bottom: 1rem; margin-top: 1rem; } a { text-decoration: none; padding: .5rem; border: 10px solid white; transition: .3s; } a:hover { background-color: white; color: rgb(37, 37, 37); border: 10px solid black; } .on { border-image: linear-gradient(white, yellow, orange, rgb(255, 72, 0), red) 3; } </style> </head> <body> <main id=\"main\"> <span id=\"statusSpan\">Calefacción <br /> <b id='status'>Apagada</b> </span> </p> <span id=\"info\"> Encender: </span> <div id=\"actions\"> <span> <a href='/ON/timer=00900'>15min</a> <a href='/ON/timer=01800'>30min</a> </span> <a href='/ON/timer=03600'>1h</a> <a href='/ON/timer=05400'>1h 30min</a> <a href='/ON/timer=07200'>2h</a> </div> </main> <script>if(window.location.pathname !== '/'){window.location.replace('http://192.168.1.100/')} const relayStatus = ");
client.print(relayStatus);
client.print("; let timeRemaining = ");
client.print(timeLimit - timer);
client.print("; const info = document.getElementById(\"info\"); const status = document.getElementById('status'); const actionContainer = document.querySelector('#actions'); function setTime(timer) { let acc = timer; let hours = Math.floor(timer / 3600).toString().padStart(2, \"0\"); acc = acc - hours * 3600; let minutes = Math.floor(acc / 60).toString().padStart(2, \"0\"); acc = acc - minutes * 60; let seconds = acc.toString().padStart(2, \"0\"); return `${hours}:${minutes}:${seconds}`; } if (relayStatus) { const interval = setInterval(() => { if (timeRemaining > 0) { timeRemaining--; document.getElementById(\"time\").innerText = setTime(timeRemaining); } else {location.reload(); clearInterval(interval); } }, 1000); status.innerText = 'Encendida'; info.innerHTML = `Se apagará en: <br> <b id=\"time\">${setTime(timeRemaining)}</b>`; document.getElementById('main').classList.add('on'); actionContainer.innerHTML = \"<a href='/OFF'>Apagar</a>\"; } </script> </body> </html>");
 

  delay(50);
  
  Serial.println("Client disconnected");
  Serial.println("");

}
