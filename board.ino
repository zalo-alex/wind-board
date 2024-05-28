#define TINY_GSM_MODEM_SIM7600   //A7608 Compatible with SIM7600 AT instructions

#define DEBUG 1

#define NMEA_PATH_KEY "wB3-Pl@7"

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG Serial

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]      = "iot.1nce.net";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Server details
const char server[]   = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int  port       = 80;

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif


TinyGsmClient client(modem);
HttpClient    http(client, server, port);

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      26
#define PIN_RX      27
#define PWR_PIN     4
#define BAT_ADC     35
#define BAT_EN      12
#define  PIN_RI     33
#define  RESET      5

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13

void debug(const char* msg) {
    if (DEBUG) {
        Serial.print(msg);
    }
}
void debugln(const char* msg) {
    debug(msg);
    debug("\n");
}

void debugln(String msg) {
    debugln(msg.c_str());
}
void debug(String msg) {
    debug(msg.c_str());
}

void debugln(int msg) {
    debugln(String(msg));
}
void debug(int msg) {
    debug(String(msg));
}

void waitForNetwork() {
    debug("Waiting for network...");
    if (!modem.waitForNetwork()) {
        debugln(" fail");
        delay(10000);
        return;
    }
    debugln(" success");
}

void getModemName() {
    debug("Modem Name:");
    debugln(modem.getModemName());
}

void getModemInfo() {
    debug("Modem Info:");
    debugln(modem.getModemInfo());
}

void tryUnlockSim() {
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && modem.getSimStatus() != 3) {
        modem.simUnlock(GSM_PIN);
    }
}

void isNetworkConnected() {
    if (modem.isNetworkConnected()) {
        debugln("Network connected");
    }
}

void connectToGPRS() {
    // GPRS connection parameters are usually set after network registration
    debug(F("Connecting to "));
    debug(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        debugln(" fail");
        delay(10000);
        return;
    }
    debugln(" success");

    if (modem.isGprsConnected()) {
        debugln("GPRS connected");
    }
}

void GET(const char *path) {
    debug(F("Performing HTTP GET request... "));
    int err = http.get(resource);

    if (err != 0) {
        debugln(F("failed to connect"));
        delay(10000);
        return;
    }

    int status = http.responseStatusCode();
    debug(F("Response status code: "));
    debugln(status);
    if (!status) {
        delay(10000);
        return;
    }

    debugln(F("Response Headers:"));
    while (http.headerAvailable()) {
        String headerName  = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        debugln("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0) {
        debug(F("Content length is: "));
        debugln(length);
    }
    if (http.isResponseChunked()) {
        debugln(F("The response is chunked"));
    }

    String body = http.responseBody();
    debugln(F("Response:"));
    debugln(body);

    debug(F("Body length is: "));
    debugln(body.length());
}

void shutdown() {
    // Shutdown

    http.stop();
    debugln(F("Server disconnected"));

    modem.gprsDisconnect();
    debugln(F("GPRS disconnected"));
}

void setup()
{
    // Set console baud rate
    Serial.begin(UART_BAUD);
    delay(10);

    Serial.println("setup...");

// BAT EN
    // pinMode(BAT_EN, OUTPUT);
    // digitalWrite(BAT_EN, HIGH);

//A7608 Reset
    // pinMode(RESET, OUTPUT);
    // digitalWrite(PWR_PIN, LOW);
    // delay(100);
    // digitalWrite(RESET, HIGH);
    // delay(1000);
    // digitalWrite(RESET, LOW);

//A7608 Power on
    // pinMode(PWR_PIN, OUTPUT);
    // digitalWrite(PWR_PIN, LOW);
    // delay(100);
    // digitalWrite(PWR_PIN, HIGH);
    // delay(1000);
    // digitalWrite(PWR_PIN, LOW);

    Serial.println("Wait...");

    // delay(3000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.init()) {
        Serial.println("Failed to restart modem, delaying 10s and retrying");
        // return;
    }

    /*
    2 Automatic
    13 GSM Only
    14 WCDMA Only
    38 LTE Only
    */
    String result;
    result = modem.setNetworkMode(38);
    if (modem.waitResponse(10000L) != 1) {
        Serial.println(" setNetworkMode faill");
        return ;
    }
}

void loop()
{

    waitForNetwork();

    getModemName();
    getModemInfo();

    tryUnlockSim();

#if defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

    isNetworkConnected();

    connectToGPRS();    

    GET(resource);

    shutdown();

    // Do nothing forevermore
    while (true) {
        delay(1000);
    }

}