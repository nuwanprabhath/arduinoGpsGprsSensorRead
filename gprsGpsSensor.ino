#include <SoftwareSerial.h>
#include <string.h>

SoftwareSerial gprs(10, 11); // TX in SIM808, RX in SIM808
String host = "34.210.162.220:8080";
String devId = "3";
const int analogInPin = A0;
String response;
String lat;
String lng;

void setupDevice(){
  Serial.println("Starting device setup");
  gprs.begin(9600);
  gprs.print("\r");
  delay(1000);
}

void setupGprs() {
  sendAT("AT");
  sendAT("AT+CREG?");
  delay(2000);
  sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  delay(1000);
  sendAT("AT+SAPBR=3,1,\"APN\",\"mobitel3g\"");
  delay(1000);
  sendAT("AT+SAPBR=1,1");
  Serial.println("GPRS setup complete");

}

void sendAT(String command) {
  command+="\r";
  Serial.println("\nTrying command:" + command);

  gprs.print(command);
  delay(2000);

  while(!gprs.available()) {
    //Serial.println("Waiting after sending AT reply");
  }
  Serial.println("After sending AT reply:");
  response="";
  while(gprs.available()) {
    char c = gprs.read();
    response+=String(c);
  }
  Serial.println(response);
  Serial.println("AT receive complete");

}

void sendGET(String url) {
  Serial.println("Trying GET on URL:"+url);
  sendAT("AT+HTTPINIT");
  sendAT("AT+HTTPPARA=\"CID\",1");
  sendAT("AT+HTTPPARA=\"URL\",\""+url+"\"");
  sendAT("AT+HTTPSSL=0");
  sendAT("AT+HTTPACTION=0");
  sendAT("AT+HTTPREAD");
  Serial.println("Send GET finished");
}

//34.210.162.220:8080/fuel/addData?lat=4.34&lng=82&deviceId=3&percentage=50
String createDataUrl(String lat, String lng, String pr){
  return host+"/fuel/addData?lat="+lat+"&lng="+lng+"&deviceId="+devId+"&percentage="+pr;
}

void setupGps(){
  Serial.println("Powering on GPS");
  sendAT("AT+CGNSPWR=1");
  delay(5000);
  Serial.println("Powering on GPS complete");
}

void getGpsLocation(){
  sendAT("AT+CGPSINF=0");
  int firstComma = response.indexOf(',');
  int secondComma = response.indexOf(',',firstComma+1);
  int thirdComma = response.indexOf(',',secondComma+1);
  lat = response.substring(firstComma+1,secondComma);
  lng = response.substring(secondComma+1,thirdComma);

} 

String doubleToString(double input,int decimalPlaces){
  if(decimalPlaces!=0){
    String string = String((int)(input*pow(10,decimalPlaces)));
    if(abs(input)<1){
      if(input>0)
        string = "0"+string;
      else if(input<0)
        string = string.substring(0,1)+"0"+string.substring(1);
    }
    return string.substring(0,string.length()-decimalPlaces)+"."+string.substring(string.length()-decimalPlaces);
  }
  else {
    return String((int)input);
  }
}


String readSensorValue(){
  int sensorValue = analogRead(analogInPin);
  double precentage = (sensorValue-59)/883.0;
  String value = doubleToString(precentage, 2);
  Serial.println("Sensor value:"+value);
  return value;
}

void setup() {
  Serial.begin(9600);
  setupDevice();
  setupGprs();
  setupGps();
}

void loop() {
  getGpsLocation();
  String pre = readSensorValue();
  String url = createDataUrl(lat,lng,pre);
  if(lat.length()>0 && lng.length()>0 && pre.length()>0){
    Serial.println("Valid data available. Making GET call");
    Serial.println("Final URL:"+url);
    sendGET(url);
  }
  
  delay(5000);
}










