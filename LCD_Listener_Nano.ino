
#include "BluetoothSerial.h"
#include <HardwareSerial.h>
#include <driver/rtc_io.h>
#include "lldp_functions.h"
#include "cdp_functions.h"
#include "Packet_data.h"
#include "images.h"
#include <EtherCard.h>
#include "DHCPOptions.h"
#include "Button2.h"

#include "esp_system.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"

/////////////SCREEN////////////
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "Free_Fonts.h"
int tft_width = 135;
int tft_height = 240;
TFT_eSPI tft = TFT_eSPI(tft_width, tft_height); // Invoke custom library

#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
int btnCick = false;

DHCP_DATA DHCP_Info[256];
int DisplayedScreen = 0;
PINFO LastCapturedPacket;
int OptionCount = 0;

byte mymac[] = {  0xCA, 0xFE, 0xC0, 0xFF, 0xEE, 0x00};
byte Ethernet::buffer[1500];
bool ENCLink;
String Protocal;
bool LogicLink = false;
bool justbooted = true;
#define Serialout
float BatLvl = 0;

DHCP_DATA DHCP_info[255];


// Setup other core to handle Bluetooth Serial port

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//int queueSize = 1;
//int queue_delay = 2;
//QueueHandle_t bluetooth_stat;

static int taskCore = 0;
int BT_STAT = 0;
BluetoothSerial SerialBT;
HardwareSerial MySerial(2);
String BluetoothName = "CDP4ME";
int BluetoothSerialSpeed = 9600;
char *Bluetoothpin = "1234";
int RS323Speed = 9600;
int RXpin = 32;
int TXpin = 13;

#define SERIAL_BUFF  1278


int ENCToBluetooth = 0;
#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"

unsigned long interval = 1000;
unsigned long previousMillis = 0;

void coreTask( void * pvParameters ) {
  rtc_wdt_protect_off();
  rtc_wdt_disable();
  disableCore0WDT();
  disableLoopWDT();

  MySerial.begin(RS323Speed, SERIAL_8N1, RXpin, TXpin); //RX, TX
  MySerial.setRxBufferSize(SERIAL_BUFF);

  SerialBT.begin(BluetoothName);
  SerialBT.register_callback(bt_callback);
  SerialBT.setPin(Bluetoothpin);

  while (true) {
    if (MySerial.available()) {
      SerialBT.write(MySerial.read());
    }
    if (SerialBT.available()) {
      MySerial.write(SerialBT.read());
    }
  }
}

void setup()
{
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_2_5, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 22, 1);

  button_init();

  tft.println("Serial 9600");

  Serial.begin(9600);

  delay(1000);

  tft.print("Starting to create task on core ");
  tft.println(taskCore);
  tft.print("Current Core:");
  tft.println(xPortGetCoreID());

  xTaskCreatePinnedToCore(  coreTask, "coreTask",     10000, NULL, 1 , NULL,  0);

  tft.println("Initializing Ethernet.");
  if (ether.begin(sizeof (Ethernet::buffer), mymac, 5) == 0) {
    tft.print("Failed to access Ethernet controller");
  }
  else {
    tft.println("Ethernet Done");

    ether.dhcpAddOptionCallback( 15, DHCPOption);

  }

  ENC28J60::enablePromiscuous();

  tft.println("Ethernet Promiscuous");
  tft.setSwapBytes(true);
  title();


}

void loop()
{
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
    showVoltage();
    LinkStatus();
    previousMillis = millis();
  }

  button_loop();

  if (BT_STAT == 0) {
    tft.pushImage(tft_width - 65, 0, 12, 21, image_BT_OFF);
  }
  if (BT_STAT == 1) {
    tft.pushImage(tft_width - 65, 0, 12, 21, image_BT);
  }

  int  plen = ether.packetReceive();
  byte buffcheck[1500];
  memcpy( buffcheck, Ethernet::buffer, plen );

  if (plen >= 0) {
    unsigned int cdp_correct = cdp_check_Packet( plen, buffcheck, plen);
    if (cdp_correct > 1) {
      displayinfo(cdp_packet_handler(buffcheck, plen));

    }
    unsigned int lldp_Correct = lldp_check_Packet(plen,  buffcheck, plen);
    if (lldp_Correct > 1) {
      displayinfo(lldp_packet_handler( buffcheck,  plen));
    }
  }
}

void displayinfo(PINFO Screens) {
  DisplayedScreen = 2;
  LastCapturedPacket = Screens;
  displayLowerBar(String( Screens.Proto[1]));

  tft.fillRect(0, 21, tft_width, tft_height - 31, TFT_BLACK );
  tft.setCursor(0, 22, 1);
  tft.setTextColor(TFT_WHITE);

  drawtext(Screens.Proto);
  drawtext(Screens.ProtoVer);
  drawtext(Screens.Name);
  drawtext(Screens.ChassisID);
  drawtext(Screens.MAC);
  drawtext(Screens.Port);
  drawtext(Screens.Model);
  drawtext(Screens.VLAN);
  drawtext(Screens.IP);
  drawtext(Screens.VoiceVLAN);
  drawtext(Screens.Cap);
  drawtext(Screens.SWver);
  drawtext(Screens.PortDesc);
  drawtext(Screens.Dup);
  drawtext(Screens.VTP);
  drawtext(Screens.TTL);



}

void drawtext(String value[2]) {
  if (value[1] != "EMPTY") {
    tft.setTextColor(TFT_GREEN);
    tft.print(value[0] + ": ");
    tft.setTextColor(TFT_WHITE);
    tft.print(value[1] + '\n');
    if (ENCToBluetooth == 1) {
      SerialBT.println(value[0] + ": " + value[1]);
    }
  }
}

void title() {

  if (tft_width >= 240) {
    tft.fillRect(0, 0, tft_width, 21, TFT_WHITE );
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("MODLOG CDP/LLDP", 5, 1, 2);
  }
  else {
    tft.fillRect(0, 0, tft_width, 21, TFT_WHITE );
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(String(" MODLOG"), 2, 1, 1);
    tft.drawString(String("CDP/LLDP"), 2, 9, 1);
  }
  tft.pushImage(tft_width - 84, 2, 18, 18, no_mag_glass);
}

void showVoltage()
{
  int vref = 1100;

  digitalWrite(14, HIGH);
  delay(1);
  float measurement = (float) analogRead(34);



  if (BatLvl != measurement) {
    BatLvl = measurement;
    tft.pushImage(tft_width - 33, 2, 32, 18, image_data_bat1);
    tft.setTextColor(TFT_BLACK);

    float battery_voltage = (measurement / 4095.0) * 7.26;
    digitalWrite(14, LOW);
    if (battery_voltage > 4.2) {
      tft.pushImage(tft_width - 33, 2, 32, 18, image_Charge);
      tft.setTextColor(TFT_RED);
      // tft.drawString(String(int((battery_voltage - 3.2) * (100 - 0) / (4.0 - 3.2) + 0)), tft_width - 28, 7);
    }
    else {

      float percent = (battery_voltage - 3.1) * (100 - 0) / (4.0 - 3.1) + 0;
      //tft.drawString(String(battery_voltage), tft_width - 28, 7);
      int per = 26 * percent / 100;
      if (per >100){per=100;}
      int colbat;
      if (percent > 70) {
        colbat = TFT_BLUE;
      }
      if (percent < 70 && percent > 30) {
        colbat = TFT_YELLOW;
      }
      if (percent < 30) {
        colbat = TFT_RED;
      }
      tft.fillRect(tft_width - 31, 2 + 2,  per, 2 + 11, colbat );


    }
  }
}

void LinkStatus()
{
  bool LinkStat = ENC28J60::isLinkUp();
  if (ENCLink != LinkStat || justbooted == true) {
    justbooted = false;
    ENCLink = LinkStat;
    if (LinkStat == true ) {
      tft.setTextColor(TFT_BLACK);
      tft.pushImage(tft_width - 50, 0, 16, 21, image_up);
      DHCP();
    }
    if (LinkStat == false ) {
      tft.setTextColor(TFT_BLACK);
      tft.fillRect(0, tft_height - 10, tft_width, 10, TFT_WHITE );
      tft.pushImage(tft_width - 50, 0, 16, 21, image_down);
    }
  }
}

void DHCP() {

  // Reset Array to defaults.
  for ( int j = 0; j < 255; ++j) {
    DHCP_info[j].Option[0] = "EMPTY";
    DHCP_info[j].Option[1] = "EMPTY";
  }

  tft.fillRect(0, 21, tft_width, tft_height, TFT_BLACK );
  displayLowerBar ("DHCP...");

  //Check for DHCP
  if (!ether.dhcpSetup())
  {
    //If Fails
    displayLowerBar ("DHCP FAILED!");
  }
  else
  {
    //If Successfull display info.
    DisplayedScreen = 1;

    displayDHCP();

  }
}
void displayLowerBar (String text) {
  tft.fillRect(0, tft_height - 10, tft_width, 10, TFT_WHITE );
  tft.setTextColor( TFT_BLACK);
  tft.setCursor(5, tft_height - 9, 1);
  tft.println(text);
  if (ENCToBluetooth == 1) {
    SerialBT.println("-----------[" + text + "]-----------" );
  }
}
void displayDHCP() {
  displayLowerBar("DHCP");

  String ipaddy;
  for (unsigned int j = 0; j < 4; ++j) {
    ipaddy  += String(ether.myip[j]);
    if (j < 3) {
      ipaddy += ".";
    }
  }

  tft.fillRect(0, 21, tft_width, tft_height - 31, TFT_BLACK );
  tft.setCursor(0, 22, 1);
  tft.setTextColor(TFT_WHITE);
  tft.println("DHCP IP: " + ipaddy);
  for (unsigned int j = 0; j < 254; ++j) {
    drawtext(DHCP_info[j].Option);

  }
}


void button_init()
{
  btn1.setLongClickHandler([](Button2 & b) {
    btnCick = false;
    int r = digitalRead(TFT_BL);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Press again to wake up",  tft.width() / 2, tft.height() / 2 );
    espDelay(6000);
    digitalWrite(TFT_BL, !r);

    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    delay(200);
    esp_deep_sleep_start();
  });
  btn1.setPressedHandler([](Button2 & b) {
    btnCick = false;

    // Enable routing screen info to Bluetooth serial. 1=enable 0=disable.
    switch (ENCToBluetooth) {
      case 0:
        ENCToBluetooth = 1;
        tft.pushImage(tft_width - 84, 2, 18, 18, mag_glass);
        break;
      case 1:
        ENCToBluetooth = 0;
        tft.pushImage(tft_width - 84, 2, 18, 18, no_mag_glass);
        break;
    }
  });
  btn2.setLongClickHandler([](Button2 & b) {
    btnCick = false;
    DHCP();
  });
  btn2.setPressedHandler([](Button2 & b) {
    btnCick = false;
    switch (DisplayedScreen) {
      case 0:
        break;

      case 1:
        DisplayedScreen = 2;
        displayinfo(LastCapturedPacket);
        break;

      case 2:
        DisplayedScreen = 1;
        displayDHCP();
        break;
    }
  });
}

void button_loop()
{
  btn1.loop();
  btn2.loop();
}
void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  // function implementation

  if (event == ESP_SPP_SRV_OPEN_EVT) {
    BT_STAT = 1;
  }
  if (event == ESP_SPP_CLOSE_EVT) {
    BT_STAT = 0;
  }
}
