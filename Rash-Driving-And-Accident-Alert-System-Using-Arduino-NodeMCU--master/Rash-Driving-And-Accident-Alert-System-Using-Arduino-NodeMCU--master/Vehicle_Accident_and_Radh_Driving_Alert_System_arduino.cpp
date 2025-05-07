#include <LiquidCrystal.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#define HOSTIFTTT "maker.ifttt.com"
#define EVENTO "disturbance"
#define IFTTTKEY "p7zBBT6FHzN_7-6batwm3rg8v9T4gmrsjXSajuwaDJ1"
TinyGPSPlus gps;  // The TinyGPS++ object
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;
// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D1;
const uint8_t sda = D2;
// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;
int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
int dcount=0;
int mailcount=0;
char temp[15];
String location1;
double MaxValue = 0.35;
double MinValue = -0.35;
int count=0;
int gpscount=0;
unsigned int timer = millis();
float latitude , longitude;
int year , month , date, hour , minute , second;
static String date_str , time_str , lat_str , lng_str;
int pm;
const int RS = D0, EN = D3, d4 = D4, d5 = D5, d6 = D6, d7 = D7;   
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);
const char* ssid = "surya";   // Enter the namme of your WiFi Network.
const char* password = "123456789";  // Enter the Password of your WiFi Network.
//char server[] = "mail.smtp2go.com";   // The SMTP Server
//WiFiClient espClient;
 WiFiClient client;
void setup() {
  Serial.begin(9600);                    
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("circuit digest");
  Wire.begin(sda, scl);
  MPU6050_Init();
  delay(1500);
  lcd.clear();  
}
void loop() {
  double Ax, Ay, Az;
  double xvalue,yvalue,zvalue;
  double xvalue1,yvalue1,zvalue1;
  double xvalue2,yvalue2,zvalue2;
  double dx,dy,dz;
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;
  xvalue = Ax -1.03;
  yvalue = Ay +0.06;
  zvalue = Az -0.07;
if (dcount%2 ==0)
{
  xvalue1 = xvalue;
  yvalue1 = yvalue;
  zvalue1 = zvalue;
  dcount++;
}
else
{
 xvalue2 = xvalue;
 yvalue2 = yvalue;
 zvalue2 = zvalue;
 dcount++;
 dx = xvalue2-xvalue1;
 dy = yvalue2-yvalue1;
 dz = zvalue2-zvalue1;
 lcd.clear();
lcd.setCursor(0,0);
lcd.print("Normal Driving");
delay(1100);
  lcd.setCursor(0,1);
  lcd.print(dx);
  lcd.setCursor(6,1);
  lcd.print(dy);
  lcd.setCursor(11,1);
  lcd.print(dz);
  delay(500);
 if(((dx < MinValue) || (dx > MaxValue)  || (dy < MinValue) || (dy > MaxValue)  || (dz < MinValue) || (dz > MaxValue)) && (timer+millis()>12000))
 {count++;
  if(count>10)
  {if(mailcount<2)
  {
  delay(500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("High Disturbance");
  wifi_123();
  gps_123(); 
  mail(); 
  //byte ret = sendEmail();
  }
    mailcount++; 
 }
}
}
}
void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}
// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}
//configure MPU6050
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
void wifi_123()
{WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }
  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void gps_123()
{
while ((Serial.available() > 0)&&(gpscount<2))
{
    gpscount++;
    if (gps.encode(Serial.read()))
    {
      if (gps.location.isValid())
      {
        gpscount++;
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
      }

      if (gps.date.isValid())
      {
        date_str = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
        if (date < 10)
          date_str = '0';
        date_str += String(date);
        date_str += " / ";
        if (month < 10)
          date_str += '0';
        date_str += String(month);
        date_str += " / ";
        if (year < 10)
          date_str += '0';
        date_str += String(year);
      }
      if (gps.time.isValid())
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        minute = (minute + 30);
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }
        hour = (hour + 5) ;
        if (hour > 23)
          hour = hour - 24;
        if (hour >= 12)
          pm = 1;
        else
          pm = 0;
        hour = hour % 12;
        if (hour < 10)
          time_str = '0';
        time_str += String(hour);
        time_str += " : ";
        if (minute < 10)
          time_str += '0';
        time_str += String(minute);
        time_str += " : ";
        if (second < 10)
          time_str += '0';
        time_str += String(second);
        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";
         Serial.print("Date= ");
         Serial.println(date_str);
         Serial.print("Time= ");
         Serial.println(time_str);
         Serial.print("Lat= ");
         Serial.println(lat_str);
         Serial.print("Long= ");
         Serial.println(lng_str); 
         delay(500);
         lcd.clear();
         lcd.setCursor(0,0);
         lcd.print(lat_str);
         lcd.setCursor(8,0);
         lcd.print(lng_str);         
      }
    }
}
}
void mail()
{
   if (client.connected())
  {
    client.stop();
  }

  client.flush();
  if (client.connect(HOSTIFTTT,80)) {
    Serial.println("Connected");
    // build the HTTP request
    String toSend = "GET /trigger/";
    toSend += EVENTO;
    toSend += "/with/key/";
    toSend += IFTTTKEY;
    toSend += "?value1=";
    toSend += lat_str;
    toSend += "&value2=";
    toSend += lng_str;
    toSend += " HTTP/1.1\r\n";
    toSend += "Host: ";
    toSend += HOSTIFTTT;
    toSend += "\r\n";
    toSend += "Connection: close\r\n\r\n";
    client.print(toSend);
    delay(250);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Email Sent");
  delay(1000);
  }
  client.flush();
  client.stop();  
  }