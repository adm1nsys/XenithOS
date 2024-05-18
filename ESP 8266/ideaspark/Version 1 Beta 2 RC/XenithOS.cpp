#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TOTP.h>
#include <Base32-Decode.h>
#include <U8g2lib.h>

// GO ON LINE 28 TO READ INSTRUCTIONS.

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);

struct ServiceKey {
  const char* service;
  const char* key;
};

typedef void (*MenuFunction)();

struct MenuItem {
  const char* name;
  MenuFunction function;
};

/*
===============================================
||                                           ||
|| 1. Wi-Fi Networks                         ||
||    - Paste the Wi-Fi name in the first    ||
||      area `""`.                           ||
||    - Paste the Wi-Fi password in the      ||
||      second area `""`.                    ||
||    - You can add and remove networks,     ||
||      using one or multiple networks.      ||
||    - Do not lose the `","` after          ||
||      `{"Network Name", "Password"}`.      ||
||      The correct format is                ||
||      `{"Network Name", "Password"},`.     ||
||                                           ||
|| 2. Time Servers                           ||
||    - Uncomment one of the following lines ||
||      to use it for time synchronization.  ||
||    - Remove or add `//` before the line   ||
||      to select the time server.           ||
||                                           ||
|| 3. TOTP Keys and Titles                   ||
||    - The first area `""` is the           ||
||      title/name of the service in the     ||
||      menu.                                ||
||    - You can name it as you wish: MyMeta, ||
||      Instagram5, @_name_, etc.            ||
||    - Do not use `\`, `"`, or `'` in the   ||
||      title unless you properly escape     ||
||      them.                                ||
||    - It is recommended to use letters     ||
||      (Aa-Zz) and numbers (0-9) in the     ||
||      title.                               ||
||    - When you obtain your TOTP Key for    ||
||      generation, it may have a format     ||
||      like this: X00X 00XX 0XXX XXXX.      ||
||      - Remove spaces to get the correct   ||
||        format: X00X00XX0XXXXXXX. After    ||
||        that, paste the code into the      ||
||        second area `""`.                  ||
||    - Similar to Wi-Fi Networks, do not    ||
||      forget the `","`.                    ||
||                                           ||
|| 4. Apps in Menu                           ||
||    - You can add or edit the app          ||
||      names/list/content.                  ||
||                                           ||
|| 4.1 Edit App List                         ||
||    - To add an app to the list, modify    ||
||      the array similar to Wi-Fi or TOTP.  ||
||      Add the app in the format            ||
||      `{"App name", AppView},`.            ||
||    - After that, add `void AppView();`    ||
||      before the array.                    ||
||    - Then, develop or add the app.        ||
||                                           ||
===============================================
*/

// Code to Edit

//Wi-Fi Networks
const char* wifiNetworks[][2] = {
  {"Network1", "Password1"},
  {"Network2", "Password2"},
  {"Network3", "Password3"},
  //Edit this massive
};
const int numWiFiNetworks = sizeof(wifiNetworks) / sizeof(wifiNetworks[0]);
WiFiUDP ntpUDP;

// Time servers. Uncomment 1 of them
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
// NTPClient timeClient(ntpUDP, "time.nist.gov", 0, 60000);

//TOTP Keys and Title
ServiceKey serviceKeys[] = {
  {"Service1", "X00X00XX0XXXXXXX"},
  {"Service2", "A00B00CD0EFGH000"},
  {"Service3", "Z00Y00XX0QRST000"},
  //Edit this massive
};

// Apps in menu
void DiagnosticsView();
void aboutinfoView();
void RebootView();
void NoWarView();

const MenuItem additionalMenuItems[] PROGMEM = {
  {"Diagnostic", DiagnosticsView},
  {"System", aboutinfoView},
  {"Reboot", RebootView},
  {"NO WAR", NoWarView},
};

/*
===============================================
||                                           ||
||     Code editing section ends here.       ||
||       Do not modify below this line.      ||
||                                           ||
===============================================
*/





//Code XenOS. If you are a regular user - do not touch it.

int additionalMenuItemsCount = sizeof(additionalMenuItems) / sizeof(additionalMenuItems[0]);

const char** menuItems;
int menuItemsCount;
int numServices;
int currentMenuIndex = 0;
int currentView = 0;
int firstVisibleIndex = 0;
const int maxVisibleItems = 3;
uint8_t hmacKey[20];
TOTP* totp;

void setupMenu() {
    numServices = sizeof(serviceKeys) / sizeof(serviceKeys[0]);
    menuItemsCount = numServices + additionalMenuItemsCount;
    menuItems = new const char*[menuItemsCount];

    for (int i = 0; i < numServices; ++i) {
        menuItems[i] = reinterpret_cast<const char*>(pgm_read_ptr(&serviceKeys[i].service));
    }

    for (int i = 0; i < additionalMenuItemsCount; ++i) {
        menuItems[numServices + i] = additionalMenuItems[i].name;
    }
}

void executeMenuItem() {
    if (currentMenuIndex < numServices) {
        generateAndDisplayTOTP(currentMenuIndex);
    } else {
        int additionalMenuIndex = currentMenuIndex - numServices;
        if (additionalMenuIndex < additionalMenuItemsCount) {
            MenuFunction func = reinterpret_cast<MenuFunction>(pgm_read_ptr(&additionalMenuItems[additionalMenuIndex].function));
            if (func) {
                func();
            }
        }
    }
}

void setup() {
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_squeezed_b6_tr);
    u8g2.drawStr(0, 10, "Beta 2 RC");
    u8g2.drawStr(60, 10, "xBoot 1.1");
    u8g2.drawStr(0, 60, "By adm1nsys");
    u8g2.setFont(u8g2_font_ncenB14_tr);
    // XenithOS
    u8g2.drawStr(30, 35, "XenOS");
    u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
    u8g2.drawStr(30, 50, "Booting...");
    u8g2.sendBuffer();
    delay(500);
    showBootScreen();
}

void setupProgressBar(int progress) {
    u8g2.setFont(u8g2_font_squeezed_b6_tr);
    String progressBar = "-";
    for (int i = 0; i < progress; ++i) {
        progressBar += "-";
    }
    u8g2.drawStr(33, 50, progressBar.c_str());
    u8g2.sendBuffer();
}

void showBootScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(30, 35, "XenOS");
    Serial.begin(115200);

    setupProgressBar(1);

    connectWiFi();
    setupProgressBar(2);

    timeClient.begin();
    setupProgressBar(3);

    pinMode(D1, INPUT_PULLUP);  // Button 1 (Menu Top)
    setupProgressBar(4);

    pinMode(D2, INPUT_PULLUP);  // Button 2 (Menu Bottom)
    setupProgressBar(5);

    pinMode(D3, INPUT_PULLUP);  // Button 3 (Sleep) (Flash on board)
    pinMode(D4, INPUT_PULLUP);  // Button 4 (Home)
    pinMode(D7, INPUT_PULLUP);  // Button 4 (Apply)
    setupProgressBar(6);

    int keyLength = base32decode(serviceKeys[0].key, hmacKey, sizeof(hmacKey));
    setupProgressBar(7);

    totp = new TOTP(hmacKey, keyLength);
    setupProgressBar(8);
    setupProgressBar(9);
    setupProgressBar(10);
    setupProgressBar(11);

    synchronizeTime();
    setupProgressBar(12);
    setupProgressBar(13);

    setupMenu();
    setupProgressBar(14);
    setupProgressBar(15);

    displayMenu();
}


void loop() {
  handleMenuNavigation();
  handleBackButtonPress();
}

void handleMenuNavigation() {
    if (currentView == 0) {
        if (digitalRead(D1) == LOW) {  // Menu Top button
            currentMenuIndex--;
            if (currentMenuIndex < 0) {
                currentMenuIndex = menuItemsCount - 1;
                firstVisibleIndex = menuItemsCount - maxVisibleItems;
                if (firstVisibleIndex < 0) {
                    firstVisibleIndex = 0;
                }
            } else if (currentMenuIndex < firstVisibleIndex) {
                firstVisibleIndex = currentMenuIndex;
            }
            displayMenu();
            delay(200);
        }

        if (digitalRead(D2) == LOW) {  // Menu Bottom button
            currentMenuIndex++;
            if (currentMenuIndex >= menuItemsCount) {
                currentMenuIndex = 0;
                firstVisibleIndex = 0;
            } else if (currentMenuIndex >= firstVisibleIndex + maxVisibleItems) {
                firstVisibleIndex = currentMenuIndex - maxVisibleItems + 1;
            }
            displayMenu();
            delay(200);
        }

        if (digitalRead(D7) == LOW) {  // Apply button
            executeMenuItem();
            delay(200);
        }
    }

    if (digitalRead(D4) == LOW) {  // Home Button
        if (currentView != 2) {
            currentView = 0;
            displayMenu();
            delay(200);
        }
    }

    if (digitalRead(D3) == LOW) {  // Sleep Button
        if (currentView != 2) {
            currentView = 0;
            displayMenu();
            delay(100);
            currentView = 2;
            SleepView();
            delay(200);
        } else {
            Wake_up_View();
            delay(200);
        }
    }
}


void SleepView() {
  currentView = 2;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_streamline_weather_t);
  u8g2.drawGlyph(53, 38, 0x0031);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(35, 53, "Sleeping");
  u8g2.sendBuffer();
  delay(1000);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void Wake_up_View() {
  currentView = 2;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t);
  u8g2.drawGlyph(53, 38, 0x0039);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(45, 53, "Hello!");
  u8g2.sendBuffer();
  delay(1000);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  currentView = 0;
  displayMenu();
}

void RebootView() {
    currentView = 2;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_streamline_music_audio_t);
    u8g2.drawGlyph(53, 38, 0x0033);
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(35, 53, "Rebooting");
    u8g2.sendBuffer();

    delay(1000);
    ESP.restart();
}

void DiagnosticsView() {
    currentView = 1;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t);
    u8g2.drawGlyph(53, 38, 0x0043);
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(30, 53, "Diagnostic");
    u8g2.sendBuffer();
    delay(500);

    wificonnectView();
}


void handleBackButtonPress() {
    if (digitalRead(D4) == LOW) {  // New Home Button
        if (currentView != 2) {
            currentView = 0;
            displayMenu();
            delay(200);  // Debounce delay
        }
    }
}

char buffer1[11];
char buffer[11];

void displayMenu() {
    u8g2.clearBuffer();
    drawStatusBar();
  
    u8g2.setFont(u8g2_font_sirclivethebold_tr);
    u8g2.drawStr(15, 10, "Menu");

    u8g2.setFont(u8g2_font_7x14B_tr);
    for (int i = 0; i < maxVisibleItems; ++i) {
        int itemIndex = firstVisibleIndex + i;
        if (itemIndex >= menuItemsCount) {
            break;
        }

        limitString(menuItems[itemIndex]);

        if (itemIndex == currentMenuIndex) {
            u8g2.setFont(u8g2_font_siji_t_6x10);
            u8g2.drawGlyph(0, (i + 1) * 17 + 10, 0xe1a9);
            u8g2.setFont(u8g2_font_7x14B_tr);
            u8g2.drawStr(15, (i + 1) * 17 + 10, buffer);
        } else {
            u8g2.setFont(u8g2_font_scrum_te);
            u8g2.drawStr(23, (i + 1) * 17 + 10, buffer);
        }
    }
    u8g2.sendBuffer();
}

void limitString(const char* input) {
    size_t len = strlen(input);
    if (len <= 10) {
        strcpy(buffer, input);
    } else {
        strncpy(buffer, input, 10);
        buffer[10] = '\0';
        strcat(buffer, "...");
    }
}

void drawStatusBar() {
    // u8g2.setFont(u8g2_font_streamline_interface_essential_key_lock_t);
    // u8g2.drawStr(85, 22, "0");
    // u8g2.setFont(u8g2_font_7x14B_tr);
    // u8g2.drawStr(15, 15, "Menu");
    // char keysCountStr[4];
    // snprintf(keysCountStr, sizeof(keysCountStr), "%d", numServices);
    // u8g2.drawStr(25, 15, keysCountStr);

    // u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFont(u8g2_font_7x14B_tr);

    if (WiFi.status() == WL_CONNECTED) {
        // u8g2.setFont(u8g2_font_streamline_interface_essential_wifi_t);
        // u8g2.drawStr(105, 21, "0");
    } else {
        u8g2.setFont(u8g2_font_twelvedings_t_all);
        u8g2.drawGlyph(110, 14, 0x0021);
    }

    if (timeClient.isTimeSet()) {
        // u8g2.drawStr(100, 10, "T");
    } else {
        u8g2.setFont(u8g2_font_twelvedings_t_all);
        u8g2.drawGlyph(110, 14, 0x0020);
    }
}

void generateAndDisplayTOTP(int serviceIndex) {
    int keyLength = base32decode(serviceKeys[serviceIndex].key, hmacKey, sizeof(hmacKey));
    totp = new TOTP(hmacKey, keyLength);
    unsigned long epoch;
    String totpCode;
    int remainingTime;

    while (true) {
        epoch = timeClient.getEpochTime();
        totpCode = totp->getCode(epoch);
        remainingTime = 30 - (epoch % 30);

        u8g2.clearBuffer();
        drawStatusBar();
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        limitString1(serviceKeys[serviceIndex].service);
        u8g2.drawStr(15, 10, buffer1);
        u8g2.setFont(u8g2_font_fur20_tn);
        u8g2.drawStr(15, 43, totpCode.c_str());
        u8g2.setFont(u8g2_font_b12_b_t_japanese2);
        u8g2.setCursor(0, 63);
        u8g2.printf("Code Active: %02d Sec", remainingTime);
        u8g2.sendBuffer();

        delay(1000);

        if (digitalRead(D3) == LOW) {  // Sleep Button
            if (currentView != 2) {
                currentView = 0;
                displayMenu();
                delay(100);
                currentView = 2;
                SleepView();
                break;
            } else {
                currentView = 0;
                displayMenu();
                delay(200);
            }
        }
        if (digitalRead(D4) == LOW) {
            break;
        }
    }
    displayMenu();
}

void limitString1(const char* input) {
    size_t len = strlen(input);
    if (len <= 10) {
        strcpy(buffer1, input);
    } else {
        strncpy(buffer1, input, 7);
        strcpy(buffer1 + 7, "...");
    }
}


void displayTOTP(const char* code) {
  u8g2.clearBuffer();
  drawStatusBar();
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
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(20, 10, "Wi-Fi");
  u8g2.setFont(u8g2_font_streamline_interface_essential_wifi_t);
  u8g2.drawGlyph(53, 38, 0x0030);
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(25, 60, "Diagnosing");
  u8g2.sendBuffer();
  setupProgressBar(1);
  setupProgressBar(2);
  setupProgressBar(3);

  if (WiFi.status() == WL_CONNECTED) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sirclivethebold_tr);
    u8g2.drawStr(20, 10, "Wi-Fi");
    u8g2.setFont(u8g2_font_streamline_business_t);
    u8g2.drawGlyph(53, 38, 0x0032);
    u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
    u8g2.drawStr(35, 60, "Connected");
    setupProgressBar(3);
    u8g2.sendBuffer();
    delay(500);
    setupProgressBar(15);
    delay(500);
    synctimeView();
  } else {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sirclivethebold_tr);
    u8g2.drawStr(20, 10, "Wi-Fi");
    u8g2.setFont(u8g2_font_streamline_interface_essential_wifi_t);
    u8g2.drawGlyph(53, 38, 0x0030);
    u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
    u8g2.drawStr(32, 60, "Connecting");
    setupProgressBar(3);
    u8g2.sendBuffer();
    delay(500);
    setupProgressBar(1);
    delay(500);

    if (connectWiFi()) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_sirclivethebold_tr);
      u8g2.drawStr(20, 10, "Wi-Fi");
      u8g2.setFont(u8g2_font_streamline_business_t);
      u8g2.drawGlyph(53, 38, 0x0032);
      u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
      u8g2.drawStr(30, 60, "Connected");
      setupProgressBar(3);
      u8g2.sendBuffer();
      delay(500);
      setupProgressBar(15);
      delay(500);
      synctimeView();
    } else {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_sirclivethebold_tr);
      u8g2.drawStr(20, 10, "Wi-Fi");
      u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t);
      u8g2.drawGlyph(53, 38, 0x0037);
      u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
      u8g2.drawStr(45, 60, "Failed");
      setupProgressBar(15);
      u8g2.sendBuffer();
      delay(1000);
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_sirclivethebold_tr);
      u8g2.drawStr(20, 10, "Wi-Fi");
      u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
      u8g2.drawStr(0, 25, "Failed Connect to");
      u8g2.drawStr(0, 35, "Wi-Fi. Without it");
      u8g2.drawStr(0, 45, "you'll can't sync");
      u8g2.drawStr(0, 55, "time. Click Home.");
      u8g2.sendBuffer();
    }
  }
}

void synctimeView() {
int attempts = 0;
int progressBarValues[5][2] = {
  {6, 7},
  {8, 9},
  {10, 11},
  {12, 13},
  {14, 15}
};

u8g2.clearBuffer();
u8g2.setFont(u8g2_font_sirclivethebold_tr);
u8g2.drawStr(20, 10, "Time");
u8g2.setFont(u8g2_font_streamline_all_t);
u8g2.drawGlyph(53, 38, 0x01f9);
u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
u8g2.drawStr(25, 60, "Diagnosting");
u8g2.sendBuffer();
setupProgressBar(1);
setupProgressBar(2);
setupProgressBar(3);

while (attempts < 5) {
    synchronizeTime();
    if (timeClient.isTimeSet()) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(20, 10, "Time");
        u8g2.setFont(u8g2_font_streamline_business_t);
        u8g2.drawGlyph(53, 38, 0x0032);
        u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
        u8g2.drawStr(37, 60, "Complete");
        setupProgressBar(3);
        u8g2.sendBuffer();
        setupProgressBar(15);
        delay(1000);
        currentView = 0;
        displayMenu();
        break;
    } else {
        setupProgressBar(progressBarValues[attempts][0]);
        setupProgressBar(progressBarValues[attempts][1]);
        attempts++;
    }
}


if (attempts >= 5 && !timeClient.isTimeSet()) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(20, 10, "Time");
  u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t);
  u8g2.drawGlyph(53, 38, 0x0037);
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(45, 60, "Failed");
  setupProgressBar(15);
  u8g2.sendBuffer();
  delay(1000);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(20, 10, "Time");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(0, 25, "Failed to sync");
  u8g2.drawStr(0, 35, "time. Try again.");
  u8g2.sendBuffer();
}

}

void NoWarView() {
   currentView = 1;
  static int currentSlide = 0;
  const int totalSlides = 9;

  while (true) {
    if (digitalRead(D4) == LOW) {
      break;
    }

    if (digitalRead(D2) == LOW) {
      currentSlide = (currentSlide + 1) % totalSlides;
      delay(200);
    }

    if (digitalRead(D1) == LOW) {
      currentSlide = (currentSlide - 1 + totalSlides) % totalSlides;
      delay(200);
    }

    u8g2.clearBuffer();

    switch (currentSlide) {
      case 0:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "Please, stop war");
        u8g2.drawStr(0, 45, "in Ukraine! You");
        u8g2.drawStr(0, 60, "can Donate.");
        break;
      case 1:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "COME BACK ALIVE");
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(0, 45, "https://savelife.in.ua/en/");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 60, "UA Fund");
        break;
      case 2:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "Prytula Foundation");
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(0, 45, "prytulafoundation.org/en/");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 60, "Prytula Fund");
        break;
      case 3:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "All links are on");
        u8g2.drawStr(0, 45, "main page of my ");
        u8g2.drawStr(0, 60, "GitHub Profile.->");
        break;
      case 4:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "Or you can find");
        u8g2.drawStr(0, 45, "this funds in");
        u8g2.drawStr(0, 60, "Google/Bing...");
        break;
      case 5:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(35, 35, "adm1nsys");
        u8g2.drawStr(30, 60, "My GitHub");
        break;
      case 6:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "Boycott on Russia");
        u8g2.drawStr(0, 45, "terrorist must be");
        u8g2.drawStr(0, 60, "punished!");
        break;
      case 7:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(25, 30, "RUSSIA IS A");
        u8g2.drawStr(10, 45, "TERRORIST STATE!");
        u8g2.drawStr(15, 60, "REMEMBER THIS!");
        break;
      case 8:
        u8g2.setFont(u8g2_font_sirclivethebold_tr);
        u8g2.drawStr(15, 10, "NO WAR");
        u8g2.setFont(u8g2_font_7x14B_tr);
        u8g2.drawStr(0, 30, "Thank you for your");
        u8g2.drawStr(0, 45, "attention! Take");
        u8g2.drawStr(0, 60, "care of yourself!");
        break;
    }

    char slideNumber[6];
    sprintf(slideNumber, "%02d/%02d", currentSlide + 1, totalSlides);
    u8g2.drawStr(90, 10, slideNumber);

    u8g2.sendBuffer();

    delay(100);
  }
}

void aboutinfoView() {
   currentView = 1;
  static int currentSlide = 0;
  const int totalSlides = 9;

  while (true) {
    if (digitalRead(D4) == LOW) {
      break;
    }

    if (digitalRead(D2) == LOW) {
      currentSlide = (currentSlide + 1) % totalSlides;
      delay(200);
    }

    if (digitalRead(D1) == LOW) {
      currentSlide = (currentSlide - 1 + totalSlides) % totalSlides;
      delay(200);
    }

    u8g2.clearBuffer();

    switch (currentSlide) {
      case 0:
        system_slides_short_os_name();
        break;
      case 1:
        system_slides_full_os_name();
        break;
      case 2:
        system_slides_os_build();
        break;
      case 3:
        system_slides_board_name();
        break;
      case 4:
        system_slides_board_edition();
        break;
      case 5:
        system_slides_board_display();
        break;
      case 6:
        system_slides_keys_available();
        break;
      case 7:
        system_slides_connection_type();
        break;
      case 8:
        system_slides_creator_github();
        break;
    }

    char slideNumber[6];
    sprintf(slideNumber, "%02d/%02d", currentSlide + 1, totalSlides);
    u8g2.drawStr(90, 10, slideNumber);

    u8g2.sendBuffer();

    delay(100);
  }
}





void system_slides_short_os_name(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Short OS Name");
  u8g2.setFont(u8g2_font_streamline_computers_devices_electronics_t);
  u8g2.drawGlyph(0, 60, 0x0034);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "XenOS");
}

void system_slides_full_os_name(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Full OS Name");
  u8g2.setFont(u8g2_font_streamline_interface_essential_text_t);
  u8g2.drawGlyph(0, 60, 0x0034);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "XenithOS");
}

void system_slides_os_build(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Version/Build");
  u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t);
  u8g2.drawGlyph(0, 60, 0x003d);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "1.0 Beta 2 RC");
}

void system_slides_board_name(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Board Name");
  u8g2.setFont(u8g2_font_streamline_computers_devices_electronics_t);
  u8g2.drawGlyph(0, 60, 0x003a);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "ESP 8266");
}

void system_slides_board_edition(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Board Edidion");
  u8g2.setFont(u8g2_font_streamline_computers_devices_electronics_t);
  u8g2.drawGlyph(0, 60, 0x0044);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "ideaspark");
}

void system_slides_board_display(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Display");
  u8g2.setFont(u8g2_font_streamline_computers_devices_electronics_t);
  u8g2.drawGlyph(0, 60, 0x0036);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "0.96 OLED");
}

void system_slides_keys_available(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Keys available");
  u8g2.setFont(u8g2_font_streamline_interface_essential_key_lock_t);
  u8g2.drawGlyph(0, 60, 0x0030);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "infinity");
}

void system_slides_connection_type(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "Connection type");
  u8g2.setFont(u8g2_font_streamline_computers_devices_electronics_t);
  u8g2.drawGlyph(0, 60, 0x0033);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "Type 2a");
}

void system_slides_creator_github(){
  u8g2.setFont(u8g2_font_sirclivethebold_tr);
  u8g2.drawStr(15, 10, "System");
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 30, "GitHub");
  u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t);
  u8g2.drawGlyph(0, 60, 0x0044);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(25, 55, "adm1nsys");
}

bool connectWiFi() {
  for (int i = 0; i < numWiFiNetworks; ++i) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifiNetworks[i][0]);

    WiFi.begin(wifiNetworks[i][0], wifiNetworks[i][1]);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("Connected to ");
      Serial.println(wifiNetworks[i][0]);
      return true;
    } else {
      Serial.println();
      Serial.print("Failed to connect to ");
      Serial.println(wifiNetworks[i][0]);
    }
  }

  Serial.println("Failed to connect to any WiFi network");
  return false;
}

void synchronizeTime() {
  if (timeClient.forceUpdate()) {
    setTime(timeClient.getEpochTime());
    Serial.println("Time synchronized");
  } else {
    Serial.println("Failed to synchronize time");
  }
}
