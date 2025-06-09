#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <RTClib.h>
#include <DHTesp.h>

// Wi-Fi credentials
#define WIFI_SSID "pekkunu" 
#define WIFI_PASSWORD "44444444" 
// Firebase credentials
#define FIREBASE_HOST "project1-a91b7-default-rtdb.firebaseio.com" 
#define FIREBASE_AUTH "KFUe1WGnvXkhKQv4PvLDBq1TMRTxw8ByKugTzcDL" 

#define DHTPIN D0
DHTesp dht;

LiquidCrystal_PCF8574 lcd(0x27);
RTC_DS1307 rtc;
DateTime t;

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

// Button and buzzer pins
int len = D4; int gtlen;
int xuong = D3; int gtxuong;
int menu = D5; int gtmenu;
int tat_baothuc = D6; int gttat_baothuc;
int macdinh = 1;
int coi = D7;

int contro = 0; int contro_bt = 6; int hang = 0;
int congtru_tong = 0; int congtru_menubaothuc = 0;
int demtong = 0; int dembaothuc = 0;

// Variables for RTC time
int ngay = 1; int thang = 1; int namng = 0; int namtr = 0; int namch = 0; int namdv = 0; int namtong = 0;
int gio = 0; int phut = 0; int giay = 0;
// Temporary variables for editing date/time
int temp_ngay = 1; int temp_thang = 1; int temp_namng = 0; int temp_namtr = 0; int temp_namch = 0; int temp_namdv = 0; int temp_namtong = 0;
int temp_gio = 0; int temp_phut = 0; int temp_giay = 0;
// Alarm variables
int gio1 = 0; int phut1 = 0; int ngay1 = 1; int thang1 = 1; bool enabled1 = false;
int gio2 = 0; int phut2 = 0; int ngay2 = 1; int thang2 = 1; bool enabled2 = false;
int gio3 = 0; int phut3 = 0; int ngay3 = 1; int thang3 = 1; bool enabled3 = false;
int gio4 = 0; int phut4 = 0; int ngay4 = 1; int thang4 = 1; bool enabled4 = false;
int gio5 = 0; int phut5 = 0; int ngay5 = 1; int thang5 = 1; bool enabled5 = false;

float lastTemp = -999; 
float lastHumi = -999; 
unsigned long lastDHTRead = 0; 
const unsigned long DHT_INTERVAL = 5000; 
unsigned long lastFirebaseSync = 0; 
const unsigned long FIREBASE_SYNC_INTERVAL = 30000; // Reduced to 30s for more frequent sync
unsigned long lastWiFiCheck = 0; 
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Reduced to 10s for faster reconnection
int lastSecond = -1; 
bool mainScreenInitialized = false; 
bool isEditingAlarm = false; 
bool alarmTriggered[5] = {false, false, false, false, false};
unsigned long lastRTCCheck = 0;
const unsigned long RTC_CHECK_INTERVAL = 1000; 

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retries = 0;
  const int maxRetries = 20; // Reduced retries for faster response
  
  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed. Will retry later.");
  }
}

void reconnectFirebase() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  
  if (WiFi.status() == WL_CONNECTED && !Firebase.ready()) {
    Serial.println("Reconnecting to Firebase...");
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    delay(500); // Small delay to stabilize connection
  }
}

void syncAlarmsFromFirebase() {
  if (WiFi.status() != WL_CONNECTED || !Firebase.ready()) {
    Serial.println("Cannot sync alarms: No Wi-Fi or Firebase connection");
    return;
  }

  Serial.println("Syncing alarms from Firebase...");
  String path = "/esp8266_data/alarms";
  if (Firebase.getJSON(fbdo, path)) {
    FirebaseJson &json = fbdo.jsonObject();
    FirebaseJsonData result;

    struct Alarm {
      int &gio, &phut, &ngay, &thang;
      bool &enabled;
    };
    Alarm alarms[] = {
      {gio1, phut1, ngay1, thang1, enabled1},
      {gio2, phut2, ngay2, thang2, enabled2},
      {gio3, phut3, ngay3, thang3, enabled3},
      {gio4, phut4, ngay4, thang4, enabled4},
      {gio5, phut5, ngay5, thang5, enabled5}
    };

    bool updated = false;
    for (int i = 0; i < 5; i++) {
      String alarmPath = "alarm" + String(i + 1);
      int temp_gio = alarms[i].gio;
      int temp_phut = alarms[i].phut;
      int temp_ngay = alarms[i].ngay;
      int temp_thang = alarms[i].thang;
      bool temp_enabled = alarms[i].enabled;

      if (json.get(result, alarmPath + "/hour") && result.success && result.intValue >= 0 && result.intValue <= 23) {
        alarms[i].gio = result.intValue;
      }
      if (json.get(result, alarmPath + "/minute") && result.success && result.intValue >= 0 && result.intValue <= 59) {
        alarms[i].phut = result.intValue;
      }
      if (json.get(result, alarmPath + "/day") && result.success && result.intValue >= 1 && result.intValue <= 31) {
        alarms[i].ngay = result.intValue;
      }
      if (json.get(result, alarmPath + "/month") && result.success && result.intValue >= 1 && result.intValue <= 12) {
        alarms[i].thang = result.intValue;
      }
      if (json.get(result, alarmPath + "/enabled") && result.success) {
        alarms[i].enabled = result.boolValue;
      }

      // Check if alarm data changed
      if (temp_gio != alarms[i].gio || temp_phut != alarms[i].phut || temp_ngay != alarms[i].ngay || 
          temp_thang != alarms[i].thang || temp_enabled != alarms[i].enabled) {
        updated = true;
      }

      Serial.printf("Alarm %d: %02d:%02d %02d/%02d Enabled: %d\n", i + 1, 
                    alarms[i].gio, alarms[i].phut, alarms[i].ngay, alarms[i].thang, alarms[i].enabled);
    }

    if (updated) {
      Serial.println("Alarms updated from Firebase");
    } else {
      Serial.println("No changes in alarm data");
    }
  } else {
    Serial.println("Failed to sync alarms: " + fbdo.errorReason());
  }
}

void uploadSensorData(float temp, float humi, unsigned long timestamp) {
  if (WiFi.status() != WL_CONNECTED || !Firebase.ready()) {
    Serial.println("Cannot upload sensor data: No Wi-Fi or Firebase connection");
    return;
  }

  FirebaseJson sensorData;
  sensorData.add("temperature", temp);
  sensorData.add("humidity", humi);
  sensorData.add("timestamp", timestamp);

  if (Firebase.setJSON(fbdo, "/esp8266_data/sensors", sensorData)) {
    Serial.println("Sensor data uploaded to Firebase");
  } else {
    Serial.println("Failed to upload sensor data: " + fbdo.errorReason());
  }
}

void updateAlarmInFirebase(int alarmNum, int day, int month, int hour, int minute, bool enabled) {
  if (WiFi.status() != WL_CONNECTED || !Firebase.ready()) {
    Serial.println("Cannot update alarm: No Wi-Fi or Firebase connection");
    return;
  }

  FirebaseJson alarmData;
  alarmData.add("day", day);
  alarmData.add("month", month);
  alarmData.add("hour", hour);
  alarmData.add("minute", minute);
  alarmData.add("enabled", enabled);

  String path = "/esp8266_data/alarms/alarm" + String(alarmNum);
  if (Firebase.setJSON(fbdo, path, alarmData)) {
    Serial.println("Alarm " + String(alarmNum) + " updated in Firebase");
    // Force immediate sync to ensure consistency
    lastFirebaseSync = 0;
  } else {
    Serial.println("Failed to update alarm " + String(alarmNum) + ": " + fbdo.errorReason());
  }
}

bool isValidDate(int year, int month, int day) {
  if (month < 1 || month > 12 || day < 1 || year < 2000 || year > 2099) return false;
  int daysInMonth[] = {31, (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return day <= daysInMonth[month - 1];
}

void updateRTC() {
  temp_namtong = (temp_namng * 1000) + (temp_namtr * 100) + (temp_namch * 10) + temp_namdv;
  if (isValidDate(temp_namtong, temp_thang, temp_ngay) && temp_gio >= 0 && temp_gio <= 23 && temp_phut >= 0 && temp_phut <= 59 && temp_giay >= 0 && temp_giay <= 59) {
    rtc.adjust(DateTime(temp_namtong, temp_thang, temp_ngay, temp_gio, temp_phut, temp_giay));
    Serial.println("RTC updated");
    ngay = temp_ngay; thang = temp_thang; namng = temp_namng; namtr = temp_namtr; namch = temp_namch; namdv = temp_namdv; namtong = temp_namtong;
    gio = temp_gio; phut = temp_phut; giay = temp_giay;
  } else {
    Serial.println("Invalid date/time, RTC not updated");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Date/Time");
    delay(2000);
  }
}
void doc_capnhatDHT22()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastDHTRead >= DHT_INTERVAL) {
        float temp = dht.getTemperature();
        float humi = dht.getHumidity();

        lcd.setCursor(8, 2);
        if (dht.getStatus() != 0) {
            lcd.print("Error   ");
            Serial.println("DHT22 Temperature Error: " + String(dht.getStatusString()));
        }
        else {
            lcd.print(temp, 1);
            lcd.print(" C ");
            Serial.print("Temperature: ");
            Serial.print(temp);
            Serial.println(" C");
            lastTemp = temp;
        }

        lcd.setCursor(8, 3);
        if (dht.getStatus() != 0) {
            lcd.print("Error   ");
            Serial.println("DHT22 Humidity Error: " + String(dht.getStatusString()));
        }
        else {
            lcd.print(humi, 1);
            lcd.print(" % ");
            Serial.print("Humidity: ");
            Serial.print(humi);
            Serial.println(" %");
            lastHumi = humi;
        }

        if (WiFi.status() == WL_CONNECTED && dht.getStatus() == 0) {
            uploadSensorData(temp, humi, t.unixtime());
        }

        lastDHTRead = currentMillis;
    }
    else {
        lcd.setCursor(8, 2);
        if (lastTemp == -999) {
            lcd.print("Error   ");
        }
        else {
            lcd.print(lastTemp, 1);
            lcd.print(" C ");
        }

        lcd.setCursor(8, 3);
        if (lastHumi == -999) {
            lcd.print("Error   ");
        }
        else {
            lcd.print(lastHumi, 1);
            lcd.print(" % ");
        }
    }
}

void manhinh() {
  if (!mainScreenInitialized) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("DATE: ");
    lcd.setCursor(2, 1);
    lcd.print("TIME: ");
    lcd.setCursor(2, 2);
    lcd.print("TEMP: ");
    lcd.setCursor(2, 3);
    lcd.print("HUMI: ");
    mainScreenInitialized = true;
  }

  if (t.second() != lastSecond) {
    lcd.setCursor(8, 0);
    char dateStr[11];
    sprintf(dateStr, "%02d/%02d/%04d", t.day(), t.month(), t.year());
    lcd.print(dateStr);

    lcd.setCursor(8, 1);
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
    lcd.print(timeStr);
    lastSecond = t.second();
  }
  doc_capnhatDHT22();
}

void menu_tong() {
  mainScreenInitialized = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(congtru_tong == 0 ? ">BACK" : " BACK");
  lcd.setCursor(0, 1);
  lcd.print(congtru_tong == 1 ? ">DATE" : " DATE");
  lcd.setCursor(0, 2);
  lcd.print(congtru_tong == 2 ? ">TIME" : " TIME");
  lcd.setCursor(0, 3);
  lcd.print(congtru_tong == 3 ? ">ALARM" : " ALARM");
}

void chonmenu_tong() {
  mainScreenInitialized = false;
  switch (congtru_tong) {
    case 0:
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("SETUP DATE");
      lcd.setCursor(14, 1);
      lcd.print("SAVE");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("SETUP TIME");
      lcd.setCursor(14, 1);
      lcd.print("SAVE");
      break;
    case 3:
      break;
  }
}

void menu_baothuc() {
  mainScreenInitialized = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(congtru_menubaothuc == 0 ? ">BACK" : " BACK");
  lcd.setCursor(0, 1);
  lcd.print(congtru_menubaothuc == 1 ? ">ALARM 1" : " ALARM 1");
  lcd.setCursor(0, 2);
  lcd.print(congtru_menubaothuc == 2 ? ">ALARM 2" : " ALARM 2");
  lcd.setCursor(0, 3);
  lcd.print(congtru_menubaothuc == 3 ? ">ALARM 3" : " ALARM 3");
  if (congtru_menubaothuc == 4 || congtru_menubaothuc == 5) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ALARM 3");
    lcd.setCursor(0, 1);
    lcd.print(congtru_menubaothuc == 4 ? ">ALARM 4" : " ALARM 4");
    lcd.setCursor(0, 2);
    lcd.print(congtru_menubaothuc == 5 ? ">ALARM 5" : " ALARM 5");
    lcd.setCursor(0, 3);
    lcd.print(" BACK");
  }
}

void chonmenu_baothuc() {
  mainScreenInitialized = false;
  switch (congtru_menubaothuc) {
    case 0:
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("SET ALARM ");
      lcd.print(congtru_menubaothuc);
      lcd.setCursor(2, 1);
      lcd.print("DATE: ");
      lcd.setCursor(2, 2);
      lcd.print("TIME: ");
      lcd.setCursor(14, 2);
      lcd.print("SAVE");
      break;
  }
}

void tb_baothuc() {
  mainScreenInitialized = false;
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("WAKE UP!");
  lcd.setCursor(3, 2);
  lcd.print("RISE AND SHINE");
}

void tru_ngay_gio(int &ngay, int &gio) {
  if (hang == 0) {
    ngay--;
    if (ngay < 1)
      ngay = 31;
  } else if (hang == 1) {
    gio--;
    if (gio < 0)
      gio = 23;
  }
}

void tru_thang_phut(int &thang, int &phut) {
  if (hang == 0) {
    thang--;
    if (thang < 1)
      thang = 12;
  } else if (hang == 1) {
    phut--;
    if (phut < 0)
      phut = 59;
  }
}

void cong_ngay_gio(int &ngay, int &gio) {
  if (hang == 0) {
    ngay++;
    if (ngay > 31)
      ngay = 1;
  } else if (hang == 1) {
    gio++;
    if (gio > 23)
      gio = 0;
  }
}

void cong_thang_phut(int &thang, int &phut) {
  if (hang == 0) {
    thang++;
    if (thang > 12)
      thang = 1;
  } else if (hang == 1) {
    phut++;
    if (phut > 59)
      phut = 0;
  }
}

void hien_thi_bt(int &ngay, int &thang, int &gio, int &phut, bool &enabled) {
  if (ngay < 10) {
    lcd.setCursor(8, 1); lcd.print("0");
    lcd.setCursor(9, 1); lcd.print(ngay);
  } else {
    lcd.setCursor(8, 1); lcd.print(ngay);
  }
  lcd.setCursor(10, 1); lcd.print("/");
  if (thang < 10) {
    lcd.setCursor(11, 1); lcd.print("0");
    lcd.setCursor(12, 1); lcd.print(thang);
  } else {
    lcd.setCursor(11, 1); lcd.print(thang);
  }
  if (gio < 10) {
    lcd.setCursor(8, 2); lcd.print("0");
    lcd.setCursor(9, 2); lcd.print(gio);
  } else {
    lcd.setCursor(8, 2); lcd.print(gio);
  }
  lcd.setCursor(10, 2); lcd.print(":");
  if (phut < 10) {
    lcd.setCursor(11, 2); lcd.print("0");
    lcd.setCursor(12, 2); lcd.print(phut);
  } else {
    lcd.setCursor(11, 2); lcd.print(phut);
  }
  lcd.setCursor(contro_bt + 2, hang + 1);
  lcd.cursor();
  delay(50);
}

void hien_thi_date() {
  if (temp_ngay < 10) {
    lcd.setCursor(0, 1); lcd.print("0");
    lcd.setCursor(1, 1); lcd.print(temp_ngay);
  } else {
    lcd.setCursor(0, 1); lcd.print(temp_ngay);
  }
  lcd.setCursor(2, 1); lcd.print("/");
  if (temp_thang < 10) {
    lcd.setCursor(3, 1); lcd.print("0");
    lcd.setCursor(4, 1); lcd.print(temp_thang);
  } else {
    lcd.setCursor(3, 1); lcd.print(temp_thang);
  }
  lcd.setCursor(5, 1); lcd.print("/");
  lcd.setCursor(6, 1); lcd.print(temp_namng); lcd.setCursor(7, 1); lcd.print(temp_namtr);
  lcd.setCursor(8, 1); lcd.print(temp_namch); lcd.setCursor(9, 1); lcd.print(temp_namdv);
  lcd.setCursor(contro, 1);
  lcd.cursor();
  delay(50);
}

void hien_thi_time() {
  if (temp_gio < 10) {
    lcd.setCursor(0, 1); lcd.print("0");
    lcd.setCursor(1, 1); lcd.print(temp_gio);
  } else {
    lcd.setCursor(0, 1); lcd.print(temp_gio);
  }
  lcd.setCursor(2, 1); lcd.print(":");
  if (temp_phut < 10) {
    lcd.setCursor(3, 1); lcd.print("0");
    lcd.setCursor(4, 1); lcd.print(temp_phut);
  } else {
    lcd.setCursor(3, 1); lcd.print(temp_phut);
  }
  lcd.setCursor(5, 1); lcd.print(":");
  if (temp_giay < 10) {
    lcd.setCursor(6, 1); lcd.print("0");
    lcd.setCursor(7, 1); lcd.print(temp_giay);
  } else {
    lcd.setCursor(6, 1); lcd.print(temp_giay);
  }
  lcd.setCursor(contro, 1);
  lcd.cursor();
  delay(50);
}

void xuli_nutxuong()
{
    if (gtxuong != macdinh) {
        if (gtxuong == 0) {
            if (demtong == 1) {
                if (congtru_tong >= 3)
                    congtru_tong = 0;
                else
                    congtru_tong++;
                menu_tong();
            }
            else if (demtong == 2 && (congtru_tong == 1 || congtru_tong == 2)) {
                contro++;
                if (contro > 15)
                    contro = 0;
            }
            else if (demtong == 3) {
                if (congtru_tong == 1) {
                    if (contro == 0 || contro == 1) {
                        temp_ngay--;
                        if (temp_ngay < 1)
                            temp_ngay = 31;
                    }
                    else if (contro == 3 || contro == 4) {
                        temp_thang--;
                        if (temp_thang < 1)
                            temp_thang = 12;
                    }
                    else if (contro == 6) {
                        temp_namng--;
                        if (temp_namng < 0)
                            temp_namng = 9;
                    }
                    else if (contro == 7) {
                        temp_namtr--;
                        if (temp_namtr < 0)
                            temp_namtr = 9;
                    }
                    else if (contro == 8) {
                        temp_namch--;
                        if (temp_namch < 0)
                            temp_namch = 9;
                    }
                    else if (contro == 9) {
                        temp_namdv--;
                        if (temp_namdv < 0)
                            temp_namdv = 9;
                    }
                }
                else if (congtru_tong == 2) {
                    if (contro == 0 || contro == 1) {
                        temp_gio--;
                        if (temp_gio < 0)
                            temp_gio = 23;
                    }
                    else if (contro == 3 || contro == 4) {
                        temp_phut--;
                        if (temp_phut < 0)
                            temp_phut = 59;
                    }
                    else if (contro == 6 || contro == 7) {
                        temp_giay--;
                        if (temp_giay < 0)
                            temp_giay = 59;
                    }
                }
                else if (congtru_tong == 3) {
                    contro_bt++;
                    if (hang == 0) {
                        if (contro_bt > 10) {
                            contro_bt = 6;
                            hang = 1;
                        }
                    }
                    else {
                        if (contro_bt > 15) {
                            contro_bt = 6;
                            hang = 0;
                        }
                    }
                }
            }
            else if (demtong == 2 && congtru_tong == 3) {
                congtru_menubaothuc++;
                if (congtru_menubaothuc > 5) {
                    congtru_menubaothuc = 0;
                }
                menu_baothuc();
            }
            else if (demtong == 4 && congtru_tong == 3) {
                if (contro_bt == 6 || contro_bt == 7) {
                    if (congtru_menubaothuc == 1)
                        tru_ngay_gio(ngay1, gio1);
                    else if (congtru_menubaothuc == 2)
                        tru_ngay_gio(ngay2, gio2);
                    else if (congtru_menubaothuc == 3)
                        tru_ngay_gio(ngay3, gio3);
                    else if (congtru_menubaothuc == 4)
                        tru_ngay_gio(ngay4, gio4);
                    else if (congtru_menubaothuc == 5)
                        tru_ngay_gio(ngay5, gio5);
                }
                else if (contro_bt == 9 || contro_bt == 10) {
                    if (congtru_menubaothuc == 1)
                        tru_thang_phut(thang1, phut1);
                    else if (congtru_menubaothuc == 2)
                        tru_thang_phut(thang2, phut2);
                    else if (congtru_menubaothuc == 3)
                        tru_thang_phut(thang3, phut3);
                    else if (congtru_menubaothuc == 4)
                        tru_thang_phut(thang4, phut4);
                    else if (congtru_menubaothuc == 5)
                        tru_thang_phut(thang5, phut5);
                }
            }
            delay(200);
        }
        macdinh = gtxuong;
    }
}

void xuli_nutlen()
{
    if (gtlen != macdinh) {
        if (gtlen == 0) {
            if (demtong == 1) {
                if (congtru_tong <= 0) {
                    congtru_tong = 3;
                }
                else {
                    congtru_tong--;
                }
                menu_tong();
            }
            else if (demtong == 2 && (congtru_tong == 1 || congtru_tong == 2)) {
                contro--;
                if (contro < 0) {
                    contro = 15;
                }
            }
            else if (demtong == 3) {
                if (congtru_tong == 1) {
                    if (contro == 0 || contro == 1) {
                        temp_ngay++;
                        if (temp_ngay > 31)
                            temp_ngay = 1;
                    }
                    else if (contro == 3 || contro == 4) {
                        temp_thang++;
                        if (temp_thang > 12)
                            temp_thang = 1;
                    }
                    else if (contro == 6) {
                        temp_namng++;
                        if (temp_namng > 9)
                            temp_namng = 0;
                    }
                    else if (contro == 7) {
                        temp_namtr++;
                        if (temp_namtr > 9)
                            temp_namtr = 0;
                    }
                    else if (contro == 8) {
                        temp_namch++;
                        if (temp_namch > 9)
                            temp_namch = 0;
                    }
                    else if (contro == 9) {
                        temp_namdv++;
                        if (temp_namdv > 9)
                            temp_namdv = 0;
                    }
                }
                else if (congtru_tong == 2) {
                    if (contro == 0 || contro == 1) {
                        temp_gio++;
                        if (temp_gio > 23)
                            temp_gio = 0;
                    }
                    else if (contro == 3 || contro == 4) {
                        temp_phut++;
                        if (temp_phut > 59)
                            temp_phut = 0;
                    }
                    else if (contro == 6 || contro == 7) {
                        temp_giay++;
                        if (temp_giay > 59)
                            temp_giay = 0;
                    }
                }
                else if (congtru_tong == 3) {
                    contro_bt--;
                    if (hang == 0) {
                        if (contro_bt < 6) {
                            contro_bt = 15;
                            hang = 1;
                        }
                    }
                    else {
                        if (contro_bt < 6) {
                            contro_bt = 10;
                            hang = 0;
                        }
                    }
                }
            }
            else if (demtong == 2 && congtru_tong == 3) {
                congtru_menubaothuc--;
                if (congtru_menubaothuc < 0) {
                    congtru_menubaothuc = 5;
                }
                menu_baothuc();
            }
            else if (demtong == 4 && congtru_tong == 3) {
                if (contro_bt == 6 || contro_bt == 7) {
                    if (congtru_menubaothuc == 1)
                        cong_ngay_gio(ngay1, gio1);
                    else if (congtru_menubaothuc == 2)
                        cong_ngay_gio(ngay2, gio2);
                    else if (congtru_menubaothuc == 3)
                        cong_ngay_gio(ngay3, gio3);
                    else if (congtru_menubaothuc == 4)
                        cong_ngay_gio(ngay4, gio4);
                    else if (congtru_menubaothuc == 5)
                        cong_ngay_gio(ngay5, gio5);
                }
                else if (contro_bt == 9 || contro_bt == 10) {
                    if (congtru_menubaothuc == 1)
                        cong_thang_phut(thang1, phut1);
                    else if (congtru_menubaothuc == 2)
                        cong_thang_phut(thang2, phut2);
                    else if (congtru_menubaothuc == 3)
                        cong_thang_phut(thang3, phut3);
                    else if (congtru_menubaothuc == 4)
                        cong_thang_phut(thang4, phut4);
                    else if (congtru_menubaothuc == 5)
                        cong_thang_phut(thang5, phut5);
                }
            }
            delay(200);
        }
        macdinh = gtlen;
    }
}

void xuli_nutmenu()
{
    if (gtmenu != macdinh) {
        if (gtmenu == 0) {
            demtong++;
            if (demtong == 1) {
                menu_tong();
            }
            else if (demtong == 2) {
                if (congtru_tong == 0) {
                    demtong = 0;
                    isEditingAlarm = false;
                    manhinh();
                }
                else if (congtru_tong == 1 || congtru_tong == 2) {
                    chonmenu_tong();
                }
                else if (congtru_tong == 3) {
                    isEditingAlarm = true;
                    menu_baothuc();
                }
            }
            else if (demtong == 4) {
                if (congtru_tong == 1) {
                    demtong = 2;
                    chonmenu_tong();
                }
                else if (congtru_tong == 2) {
                    demtong = 2;
                    chonmenu_tong();
                }
                else if (congtru_tong == 3 && (contro_bt == 12 || contro_bt == 13 || contro_bt == 14 || contro_bt == 15)) {
                    if (congtru_menubaothuc == 1) {
                        enabled1 = true;
                        updateAlarmInFirebase(1, ngay1, thang1, gio1, phut1, enabled1);
                    }
                    else if (congtru_menubaothuc == 2) {
                        enabled2 = true;
                        updateAlarmInFirebase(2, ngay2, thang2, gio2, phut2, enabled2);
                    }
                    else if (congtru_menubaothuc == 3) {
                        enabled3 = true;
                        updateAlarmInFirebase(3, ngay3, thang3, gio3, phut3, enabled3);
                    }
                    else if (congtru_menubaothuc == 4) {
                        enabled4 = true;
                        updateAlarmInFirebase(4, ngay4, thang4, gio4, phut4, enabled4);
                    }
                    else if (congtru_menubaothuc == 5) {
                        enabled5 = true;
                        updateAlarmInFirebase(5, ngay5, thang5, gio5, phut5, enabled5);
                    }
                    isEditingAlarm = false;
                    menu_baothuc();
                    demtong = 2;
                    congtru_tong = 3;
                    contro_bt = 6;
                    hang = 0;
                    lcd.noCursor();
                    lastFirebaseSync = 0; // Trigger immediate sync
                }
            }
            else if (demtong == 3 && (congtru_tong == 2 || congtru_tong == 1) && (contro == 12 || contro == 13 || contro == 14 || contro == 15)) {
                updateRTC();
                demtong = 1;
                congtru_tong = 0;
                contro = 0;
                menu_tong();
                lcd.noCursor();
            }
            else if (demtong == 3 && congtru_tong == 3) {
                if (congtru_menubaothuc == 0) {
                    demtong = 1;
                    congtru_menubaothuc = 0;
                    isEditingAlarm = false;
                    menu_tong();
                }
                else if (congtru_menubaothuc == 1 || congtru_menubaothuc == 2 || congtru_menubaothuc == 3 || congtru_menubaothuc == 4 || congtru_menubaothuc == 5) {
                    isEditingAlarm = true;
                    chonmenu_baothuc();
                }
            }
            else if (demtong == 5 && congtru_tong == 3 && (congtru_menubaothuc == 1 || congtru_menubaothuc == 2 || congtru_menubaothuc == 3 || congtru_menubaothuc == 4 || congtru_menubaothuc == 5)) {
                chonmenu_baothuc();
                demtong = 3;
            }
            delay(200);
        }
        macdinh = gtmenu;
    }
}

void xuli_nuttatbt()
{
    if (gttat_baothuc != macdinh) {
        if (gttat_baothuc == 0 && dembaothuc == 1) {
            dembaothuc = 0;
            digitalWrite(coi, LOW);
            manhinh();
            delay(200);
        }
        macdinh = gttat_baothuc;
    }
}

void setup() {
  Serial.begin(9600);
  
  connectWiFi();
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(2025, 5, 28, 9, 49, 0));
  }
  
  lcd.begin(20, 4);
  lcd.setBacklight(255);
  
  dht.setup(DHTPIN, DHTesp::DHT22); 
  delay(2000);
  
  pinMode(len, INPUT_PULLUP);
  pinMode(xuong, INPUT_PULLUP);
  pinMode(menu, INPUT_PULLUP);
  pinMode(tat_baothuc, INPUT_PULLUP);
  pinMode(coi, OUTPUT);

  // Initial sync to ensure alarms are loaded
  syncAlarmsFromFirebase();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastRTCCheck >= RTC_CHECK_INTERVAL) {
    t = rtc.now();
    lastRTCCheck = currentMillis;
  }
  
  gtlen = digitalRead(len);
  gtxuong = digitalRead(xuong);
  gtmenu = digitalRead(menu);
  gttat_baothuc = digitalRead(tat_baothuc);

  // Always check Wi-Fi and Firebase connection
  if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
      reconnectFirebase();
    }
    lastWiFiCheck = currentMillis;
  }

  // Sync alarms even when in menus, unless editing an alarm
  if (currentMillis - lastFirebaseSync >= FIREBASE_SYNC_INTERVAL && !isEditingAlarm) {
    syncAlarmsFromFirebase();
    lastFirebaseSync = currentMillis;
  }
  xuli_nutlen();
  xuli_nutxuong();
  xuli_nutmenu();
  xuli_nuttatbt();

  if (demtong == 0 && congtru_tong == 0) {
    manhinh();
    lcd.noCursor();
  } else if ((demtong == 2 || demtong == 3) && congtru_tong != 3) {
    if (congtru_tong == 1) {
      hien_thi_date();
    } else if (congtru_tong == 2) {
      hien_thi_time();
    }
  } else if ((demtong == 3 || demtong == 4) && congtru_tong == 3) {
    if (congtru_menubaothuc == 1)
      hien_thi_bt(ngay1, thang1, gio1, phut1, enabled1);
    else if (congtru_menubaothuc == 2)
      hien_thi_bt(ngay2, thang2, gio2, phut2, enabled2);
    else if (congtru_menubaothuc == 3)
      hien_thi_bt(ngay3, thang3, gio3, phut3, enabled3);
    else if (congtru_menubaothuc == 4)
      hien_thi_bt(ngay4, thang4, gio4, phut4, enabled4);
    else if (congtru_menubaothuc == 5)
      hien_thi_bt(ngay5, thang5, gio5, phut5, enabled5);
  }

  // Check alarms regardless of menu state
  struct Alarm {
    int gio, phut, ngay, thang;
    bool enabled;
  };
  Alarm alarms[] = {
    {gio1, phut1, ngay1, thang1, enabled1},
    {gio2, phut2, ngay2, thang2, enabled2},
    {gio3, phut3, ngay3, thang3, enabled3},
    {gio4, phut4, ngay4, thang4, enabled4},
    {gio5, phut5, ngay5, thang5, enabled5}
  };

  for (int i = 0; i < 5; i++) {
    if (alarms[i].enabled &&
        t.day() == alarms[i].ngay && t.month() == alarms[i].thang &&
        t.hour() == alarms[i].gio && t.minute() == alarms[i].phut &&
        t.second() == 1 && !alarmTriggered[i]) {
      dembaothuc = 1;
      alarmTriggered[i] = true;
      Serial.printf("Alarm %d triggered!\n", i + 1);
    } else if (t.minute() != alarms[i].phut || t.hour() != alarms[i].gio) {
      alarmTriggered[i] = false; 
    }
  }

  if (dembaothuc == 1) {
    tb_baothuc();
    digitalWrite(coi, HIGH);
    delay(500);
    digitalWrite(coi, LOW);
    delay(50);
  }
}