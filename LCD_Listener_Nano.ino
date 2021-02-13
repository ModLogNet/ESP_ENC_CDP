#include "BluetoothSerial.h"
#include <HardwareSerial.h>
#include "Packet_data.h";
#include <driver/rtc_io.h>
#include "lldp_functions.h"
#include "cdp_functions.h"
//////////////////////////////////////////////////////////////////////////////
#include <EtherCard.h>
#include <Adafruit_ST7735.h>


//Push Button to GPIO 33 pulled down with a 10K Ohm resistor
RTC_DATA_ATTR int bootCount = 0;




// ethernet interface mac address, must be unique on the LAN

byte mymac[] = {  0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
byte Ethernet::buffer[1500];
String Protocal;
int CrntItem = 0;

int Changed = 0;


#include <Adafruit_GFX.h>    // Core graphics librar
#include <Adafruit_ST7735.h> // Hardware-specific library
//#include <SPI.h>

//Use these pins for the shield!
//#define sclk 13
//#define mosi 11
//#define cs   4
//#define dc   8
//#define rst  9  // you can also connect this to the Arduino reset

// Option 1: use any pins but a little slower
//Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

//Print &out = tft;



/////////////////////////////////////////////////////////////////
// Dual CPU stuff                                              //
/////////////////////////////////////////////////////////////////
static int taskCore = 0;

void coreTask( void * pvParameters ) {
  //Bluetooth COMs are running on core 1
  BluetoothSerial SerialBT;
  HardwareSerial MySerial(2);
  MySerial.begin(9600, SERIAL_8N1, 16, 17);
  SerialBT.begin("FarCough");

  while (true) {
    //MySerial.println (".");
    if (MySerial.available()) {
      SerialBT.write(MySerial.read());
    }
    if (SerialBT.available()) {
      MySerial.write(SerialBT.read());
    }
    delay(10);
  }
}

int WokeUP;
int pushBtn = 32;

//volatile int interruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;
volatile boolean KillEth = false;


void IRAM_ATTR isr_handle() {
  portENTER_CRITICAL(&buttonMux);
  Serial.println("Sleeping....");
  KillEth = true;
  timerAlarmEnable(timer);
  portEXIT_CRITICAL(&buttonMux);

}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  ENC28J60::powerDown();
  Serial.println("Turning OFF");
  digitalWrite(4, LOW);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 1); //1 = High, 0 = Low
  esp_deep_sleep_start();
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 6000000, true);

  attachInterrupt(pushBtn, isr_handle, HIGH);
  Serial.begin(9600);
  pinMode (4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode ( pushBtn, INPUT );
  print_wakeup_reason();

  pinMode (4, OUTPUT);
  digitalWrite(4, HIGH);

  //  //tft.fillScreen(ST7735_BLACK);
  // pinMode(13, OUTPUT);
  delay(1000);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));



  //////////////////////////////////////////////////////////////////////////
  Serial.print("Starting to create task on core ");
  Serial.println(taskCore);
  Serial.print("Current Core:");
  Serial.println(xPortGetCoreID());
  xTaskCreatePinnedToCore(
    coreTask,   /* Function to implement the task */
    "coreTask", /* Name of the task */
    10000,      /* Stack size in words */
    NULL,       /* Task input parameter */
    0,          /* Priority of the task */
    NULL,       /* Task handle. */
    taskCore);  /* Core where the task should run */

  Serial.println("Task created...");
  ////////////////////////////////////////////////////////////////////////


  Serial.println("Serial Initialised\nScreen Initialising");
  ////tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  Serial.println("Screen Done");

  ////tft.fillScreen(ST7735_BLACK);

  ////tft.setTextWrap(false);
  ////tft.setRotation(1);
  ////tft.setCursor(0, 0);
  //testdrawtext("Default Interval is 60secs!\n", ST7735_RED);
  // testdrawtext("DHCP:", ST7735_BLUE);

  ether.begin(sizeof (Ethernet::buffer), mymac, 5);
  Serial.println("Ethernet Done");
  if (!ether.dhcpSetup())
  {
    //   testdrawtext("DHCP failed.\n", ST7735_RED);
    Serial.println("DHCP failed.");
  }
  else
  {

    // testdrawtext("DHCP IP:", ST7735_GREEN);
    Serial.print("DHCP IP:");
    for (unsigned int j = 0; j < 4; ++j) {
      Serial.print(String(ether.myip[j]));
      // testdrawtext(String(ether.myip[j]), ST7735_WHITE);
      if (j < 3) {
        Serial.print(".");
        //  testdrawtext(".", ST7735_WHITE);
      }
    }
    testdrawtext("\n", ST7735_WHITE);
    Serial.print("\n");
  }
  delay(1000);
  ENC28J60::enablePromiscuous();
  Serial.println("Ethernet Promiscuous");





}
void loop()
{
  if (KillEth == true) {
    Serial.println("Powering Down ETH");

    delay(5000);
    ENC28J60::powerDown();
    Serial.println("Turning OFF");
    digitalWrite(4, LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 1); //1 = High, 0 = Low
    esp_deep_sleep_start();
  }
  int  plen = ether.packetReceive();
 // Serial.println("plen size:");
  //Serial.print(plen);
  byte buffcheck[1500];
  memcpy( buffcheck, Ethernet::buffer, plen );
  // check if valid tcp data is received

  if (plen >= 0) {
    // unsigned char* data = (unsigned char *) Ethernet::buffer;
    //plen = sizeof(buffcheck);
   // Serial.println("plen size:" + plen);
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
  Serial.println("Protocol: " + Screens.Proto);
  Serial.println("Protocol Ver: " + Screens.ProtoVer);
  Serial.println("Name: " + Screens.Name);
  Serial.println("Chassis ID: " + Screens.ChassisID);
  Serial.println("MAC: " + Screens.MAC);
  Serial.println("Port: " + Screens.Port);
  Serial.println("Model: " + Screens.Model);
  Serial.println("VLAN: " + Screens.VLAN);
  Serial.println("IP: " + Screens.IP);
  Serial.println("VoiceVLAN: " + Screens.VoiceVLAN);
  Serial.println("Cap: " + Screens.Cap);
  Serial.println("SWver: " + Screens.SWver);
  Serial.println("Port Desc: " + Screens.PortDesc);
  Serial.println("Duplex: " + Screens.Dup);
  Serial.println("VTP: " + Screens.VTP);
  Serial.println("TTL: " + Screens.TTL);
  Serial.println("------[END]--------");
}




void set_mac(const byte a[], unsigned int offset, unsigned int length) {
  // unsigned int n = 0;
  // for (unsigned int i = offset; i < offset + length; ++i) {
  // if (i > offset && i % 2 == 0) value_mac_buffer[n++] = ':';

  //  value_mac_buffer[n++] = val2hex(a[i] >> 4);
  //  value_mac_buffer[n++] = val2hex(a[i] & 0xf);
  //  }

  //  value_mac_buffer[n++] = '\0';
  //set_menu(LABEL_MAC, value_mac_buffer);
}


void drawvalues(char *text, uint16_t color) {
  //tft.setTextColor(color);
  //tft.setTextWrap(true);
  //tft.setRotation(1);
}

void testdrawtext(String text, uint16_t color) {
  //tft.setTextColor(color);
  //tft.print(text);
}

void drawscreen () {
  //tft.fillScreen(ST7735_BLACK);
  //tft.setTextWrap(false);
  //tft.setRotation(1);
  //tft.setCursor(0, 0);
  testdrawtext("Default Interval is 60secs!\n", ST7735_RED);
  if (String(ether.myip[1]) == "") {
    testdrawtext("DHCP failed.\n", ST7735_RED);
    Serial.println("DHCP failed.");
  }
  else
  {
    String DHCPtxt = "";
    testdrawtext("DHCP IP:", ST7735_GREEN);
    for (unsigned int j = 0; j < 4; ++j) {
      DHCPtxt = DHCPtxt + (String(ether.myip[j]));
      testdrawtext(String(ether.myip[j]), ST7735_WHITE);
      if (j < 3) {
        DHCPtxt = DHCPtxt + ".";
        testdrawtext(".", ST7735_WHITE);
      }
    }
    testdrawtext("\n", ST7735_WHITE);
    Serial.println("DHCP IP:" + DHCPtxt);
    testdrawtext("Protocal:", ST7735_GREEN);
    testdrawtext(Protocal, ST7735_WHITE);
    testdrawtext("\n", ST7735_WHITE);
  }

  for (unsigned int i = 0; i < 8; ++i) {
    //tft.setCursor(0, i * 10 + 28);
    //    testdrawtext(Names[i], ST7735_GREEN);
    //    testdrawtext(LCD_data[i] + "\n", ST7735_WHITE);
    //   Serial.println("" + Names[i] + "" + LCD_data[i]);
    //    LCD_data[i] = "";
  }

  Serial.println("--------END-------");
  delay(1000);
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); ether.begin(sizeof (Ethernet::buffer), mymac, 5); ether.dhcpSetup(); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
