#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TOTP.h>
#include <Base32-Decode.h>
#include <U8g2lib.h>

// 1. Connect Wi-Fi (line 17-18).
// 2. Write your key (line 26).
// 3. Name Servise.  (line 34).
// 4. You can change time server (line 21 or 22). 
// 5. No Wifi: X on status bar. Wifi work - W. 
// 6. Error synchronize time ! or if it correct - T. 


const char* ssid = "write wifi name here";
const char* password = "write wifi password here";
WiFiUDP ntpUDP;
//time servers. Uncomment 1 of them. 
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
// NTPClient timeClient(ntpUDP, "time.nist.gov", 0, 60000);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);
// Correct example:
// const char* base32Secret1 = "BSWY3DPEHPK3PXP";
const char* base32Secret1 = "write here key without spaces example: JBSWY3DPEHPK3PXP";
uint8_t hmacKey[20];
TOTP* totp;

unsigned long lastButtonPress = 0; 

int currentMenuIndex = 0;
const char* menuItems[] = {
"TOTP Code", 
"Wi-Fi Connect", 
"Sync Time", 
"About info", 
};
int menuItemsCount = sizeof(menuItems) / sizeof(menuItems[0]);
int currentView = 0;  


void setup() {
  Serial.begin(115200);
  pinMode(D1, INPUT_PULLUP);  // Button 1 (Menu Top)
  pinMode(D2, INPUT_PULLUP);  // Button 2 (Menu Bottom)
  pinMode(D3, INPUT_PULLUP);  // Button 3 (Apply)
  u8g2.begin();
  showBootScreen();
  connectWiFi();
  timeClient.begin();
  int keyLength = base32decode(base32Secret1, hmacKey, sizeof(hmacKey));
  totp = new TOTP(hmacKey, keyLength);
  synchronizeTime();
  displayMenu();
}

void showBootScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_b6_tr);
  u8g2.drawStr(0, 10, "Beta 1");
  u8g2.drawStr(0, 60, "By adm1nsys");
  u8g2.setFont(u8g2_font_littlemissloudonbold_te);
  // XenithOS
  u8g2.drawStr(40, 25, "XenOS");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(30, 40, "Booting...");
  u8g2.sendBuffer();
}


void loop() {

  if (currentView > 0 && !digitalRead(D1) && millis() - lastButtonPress > 0) {
    currentView = 0;
    lastButtonPress = millis();
    displayMenu();
  }

  switch (currentView) {
    case 0:
      handleMenu();
      break;
    case 1:
      generateAndDisplayTOTP();
      break;
    case 2:
      wificonnectView();
      break;
    case 3:
      synctimeView();
      break;
    case 4:
      aboutinfoView();
      break;
  }
  delay(100);
}

void handleMenu() {
  if (!digitalRead(D1) && millis() - lastButtonPress > 200) {  
    currentMenuIndex--;
    if (currentMenuIndex < 0) currentMenuIndex = menuItemsCount - 1;
    lastButtonPress = millis();
    displayMenu();
  }
  if (!digitalRead(D2) && millis() - lastButtonPress > 200) {
    currentMenuIndex++;
    if (currentMenuIndex >= menuItemsCount) currentMenuIndex = 0;
    lastButtonPress = millis();
    displayMenu();
  }
  if (!digitalRead(D3) && millis() - lastButtonPress > 200) {
    currentView = currentMenuIndex + 1;
    lastButtonPress = millis();
  }
}

void handleButtonPress() {
  if (!digitalRead(D1)) {  
    currentMenuIndex--;
    if (currentMenuIndex < 0) currentMenuIndex = menuItemsCount - 1;
  }
  if (!digitalRead(D2)) { 
    currentMenuIndex++;
    if (currentMenuIndex >= menuItemsCount) currentMenuIndex = 0;
  }
  if (!digitalRead(D3)) {  
    currentView = currentMenuIndex + 1; 
  }
}

void displayMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  for (int i = 0; i < menuItemsCount; i++) {
    if (i == currentMenuIndex) {
      u8g2.drawStr(0, (i + 1) * 16, (String("> ") + menuItems[i]).c_str());
    } else {
      u8g2.drawStr(20, (i + 1) * 16, menuItems[i]);
    }
  }
  drawStatusBar();
  u8g2.sendBuffer();
}

void drawStatusBar() {
  // u8g2.setFont(u8g2_font_battery19_tn);
  // u8g2.drawStr(120, 60, "6");
  // // u8g2.drawStr(10, 60, "01234567");

  u8g2.setFont(u8g2_font_6x10_tf);  

  if (WiFi.status() == WL_CONNECTED) {
    u8g2.drawStr(110, 60, "W");  
  } else {
    u8g2.drawStr(110, 60, "X");  
  }

  if (timeClient.isTimeSet()) {
    u8g2.drawStr(100, 60, "T"); 
  } else {
    u8g2.drawStr(100, 60, "!");
  }

  u8g2.sendBuffer();
}

void generateAndDisplayTOTP() {
  char code[7];
  strcpy(code, totp->getCode(now()));
  displayTOTP(code);
  delay(1000); 
}

void displayTOTP(const char* code) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(0, 10, "TOTP Code:");
  u8g2.drawStr(10, 30, code);
  int remainingTime = 30 - (now() % 30);
  u8g2.setCursor(10, 50);
  u8g2.printf("Active: %02d Sec", remainingTime);
  u8g2.sendBuffer();
}

void wificonnectView() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_littlemissloudonbold_te);
  u8g2.drawStr(25, 30, "Please Wait");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(30, 50, "Connecting");
  u8g2.sendBuffer();
  connectWiFi();
  delay(100);
  currentMenuIndex = 1;
  currentView = 0;  
  displayMenu();
}

void synctimeView() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_littlemissloudonbold_te);
  u8g2.drawStr(25, 30, "Please Wait");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(30, 50, "Processing");
  u8g2.sendBuffer();
  synchronizeTime();
  delay(100);
  currentMenuIndex = 2;
  currentView = 0;  
  displayMenu();
}

void aboutinfoView() {
  u8g2.clearBuffer();
  // u8g2.setFont(u8g2_font_squeezed_b6_tr);
  u8g2.setFont(u8g2_font_squeezed_b6_tr);
  u8g2.drawStr(0, 10, "By adm1nsys");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(0, 25, "XenOS Beta1");
  u8g2.setFont(u8g2_font_squeezed_b6_tr);
  u8g2.drawStr(0, 35, "Full OS name XenithOS");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(0, 45, "For ESP 8266");
  u8g2.drawStr(0, 55, "Key support 1");
  u8g2.sendBuffer();
}


void connectWiFi() {
  int attempts = 0; 
  while (attempts < 5) { 
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && attempts < 5) {
      delay(500);
      Serial.print(".");
      attempts++; 
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected");
      break; 
    }
  }
  if (attempts >= 5) {
    Serial.println("Failed to connect to WiFi after 5 attempts");
  }
}


void synchronizeTime() {
  if (timeClient.forceUpdate()) {
    setTime(timeClient.getEpochTime());
    Serial.println("Time synchronized");
  } else {
    Serial.println("Failed to synchronize time");
  }
}


