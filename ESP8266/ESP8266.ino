/*
   NodeMCU ESP8266/ESP12E    RFID MFRC522 / RC522
           D2       <---------->   SDA/SS
           D5       <---------->   SCK
           D7       <---------->   MOSI
           D6       <---------->   MISO
           GND      <---------->   GND
           D1       <---------->   RST
           3V/3V3   <---------->   3.3V
                             LCD 1602
           D4       <---------->   SDA
           D3       <---------->   SCL
           VU       <---------->   Vin
           G        <---------->   GND
                             Motor serve
           D8       <---------->   Signal
           3V3      <---------->   VCC
           G        <---------->   GND

*/
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>                 //Thư viện giao tiếp I2C
#include <LiquidCrystal_I2C.h>    //Thư viện LCD
#include <Servo.h>

#define SS_PIN D2  //--> SDA / SS is connected to pinout D2
#define RST_PIN D1  //--> RST is connected to pinout D1
MFRC522 mfrc522(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.

LiquidCrystal_I2C lcd(0x27, 16, 2); //Thiết lập địa chỉ và loại LCD

Servo myservo;

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router
//----------------------------------------SSID and Password of your WiFi router-------------------------------------------------------------------------------------------------------------//
const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80);  //--> Server on port 80

int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;
bool check = false;
int totalWords = 0;

//-----------------------------------------------------------------------------------------------SETUP--------------------------------------------------------------------------------------//
void setup() {
  Serial.begin(115200); //--> Initialize serial communications with the PC
  SPI.begin();      //--> Init SPI bus
  Wire.begin(2, 0);
  myservo.attach(15);
  mfrc522.PCD_Init(); //--> Init MFRC522 card
  delay(500);
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");
  lcd.init();                 //Bắt đầu màn hình
  lcd.backlight();            // Bật đèn nền
  lcd.setCursor(0, 0); lcd.print("    Welcome     ");
  lcd.setCursor(0, 1); lcd.print("Quet The Tai Day");
  lcd.display();              // Hiển thị lên màn hình.
  lcd.blink();                // Nhấp nháy con trỏ ở vị trí cuối cùng
  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  }
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  //----------------------------------------If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Please tag a card or keychain to see the UID !");
  Serial.println("");
  delay(1000);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void loop() {
  if (!mfrc522.PICC_IsNewCardPresent())
    return;
  if (mfrc522.PICC_ReadCardSerial()) {
    Serial.print("THE UID OF THE SCANNED CARD IS : ");
    for (int i = 0; i < 4; i++) {
      readcard[i] = mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
      array_to_string(readcard, 4, str);
      StrUID = str;
    }
    //Serial.println(StrUID);
    digitalWrite(ON_Board_LED, LOW);
    HTTPClient http;    //Declare object of class HTTPClient
    WiFiClient wclient;

    String UIDresultSend, postData;
    UIDresultSend = StrUID;
    Serial.println(UIDresultSend);

    //Post Data
    postData = "UIDresult=" + UIDresultSend;

    http.begin(wclient, "http://192.168.*.*/train_ticketing/dbread.php");  //Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

    int httpCode = http.POST(postData);   //Send the request
    String payload = http.getString();    //Get the response payload
    payload.remove(0, 2);
    String* array = split_string(payload,  totalWords);
    int n = sizeof(array);
    check = false;
    for (int i = 0; i < n; i++) {
      if (array[i] == StrUID) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(" ID Cua Ban La: ");
        lcd.setCursor(4, 1); lcd.print(StrUID);
        delay(1500);
        lcd.clear();
        StrUID.remove(2, 6);
        if (StrUID == "2A") {
          lcd.setCursor(0, 0); lcd.print(" The Nguoi Lon ");
          lcd.setCursor(0, 1); lcd.print("Da Thanh Toan");
          check = true;
          break;
        }
        if (StrUID == "2C") {
          lcd.setCursor(0, 0); lcd.print(" The Nguoi Lon ");
          lcd.setCursor(0, 1); lcd.print("Chua Thanh Toan");
          check = false;
          break;
        }
        if (StrUID == "EA") {
          lcd.setCursor(0, 0); lcd.print(" The Hoc Sinh  ");
          lcd.setCursor(0, 1); lcd.print("Da Thanh Toan");
          check = true;
          break;
        }
        if (StrUID == "EC") {
          lcd.setCursor(0, 0); lcd.print(" The Hoc Sinh  ");
          lcd.setCursor(0, 1); lcd.print("Chua Thanh Toan");
          check = false;
          break;
        }
        if (StrUID == "AA" || StrUID == "AC") {
          lcd.setCursor(0, 0); lcd.print(" The Tre Em ");
          lcd.setCursor(0, 1); lcd.print(" Ve Mien Phi ");
          check = true;
          break;
        }
      }
    }
    if (check) {
      myservo.write(90); // Mở
      delay(3000);
      myservo.write(0); // Đóng
    }
    else {
      delay(1500);
      lcd.clear(); lcd.print("ID Chua Duyet!!!");
      lcd.setCursor(0, 1); lcd.print("Hay Kiem Tra Lai");
      delay(1500);
    }
    lcd.clear();
    http.end();  //Close connection
    delay(1000);
    digitalWrite(ON_Board_LED, HIGH);
    mfrc522.PICC_HaltA();
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------Procedure for reading and obtaining a UID from a card or keychain---------------------------------------------------------------------------------//
void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}
String* split_string(String str, int & sizeWords) {
  int length = str.length();

  int totalWords = length % 8 == 0 ? length / 8 : length / 8 + 1;
  sizeWords = totalWords;

  String* arrResult = new String[totalWords];

  String item = "";
  for (int i = 0; i < length; i++) {
    item += str[i];

    if (i % 8 == 7) {
      arrResult[i / 8] = item;
      item = "";
    } else if (i == length - 1) {
      arrResult[i / 8] = item;
    }
  }
  return arrResult;
}
