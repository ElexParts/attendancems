#include <Arduino.h>
#include <SIM900.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <sms.h>

// Check GSM.cpp for configuring GSM pins.
// #define _GSM_TXPIN_ 2
// #define _GSM_RXPIN_ 3
#define GSM_STATUS_PIN 7
#define SS_PIN 9
#define RST_PIN 8

File myFile;
MFRC522 mfrc522(SS_PIN, RST_PIN);
SMSGSM sms;

boolean started = false;
char own_phone_num[] = "639000000000";
char test_sms_text[] = "Test message.";

void setup()
{
    pinMode(GSM_STATUS_PIN, OUTPUT);

    // Serial connection.
    Serial.begin(9600);

    Serial.print("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("initialization failed!");
        while (1);
    }
    Serial.println("initialization done.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    myFile = SD.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
        Serial.print("Writing to test.txt...");
        myFile.println("testing 1, 2, 3.");
        // close the file:
        myFile.close();
        Serial.println("done.");
    } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }

    // re-open the file for reading:
    myFile = SD.open("test.txt");
    if (myFile) {
        Serial.println("test.txt:");

        // read from the file until there's nothing else in it:
        while (myFile.available()) {
            Serial.write(myFile.read());
        }
        // close the file:
        myFile.close();
    } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }

    // SPI connection.
    Serial.print("Initializing RFID...");
    SPI.begin();
	mfrc522.PCD_Init();
    Serial.println("Scan PICC to see UID and type...");
    Serial.println("initialization done.");
    
    // Start configuration of shield with baudrate.
    // For http uses is raccomanded to use 4800 or slower.
    Serial.println("Initializing GSM Shield...");
    if (gsm.begin(2400)) {
        Serial.println("\nstatus = READY");
        started = true;
    } else Serial.println("\nstatus = IDLE");

    if(started) {
        // Enable this two lines if you want to send an SMS.
        // if (sms.SendSMS(own_phone_num, test_sms_text))
        // Serial.println("\nSMS sent OK");

        // Turn on LED when GSM is ready.
        digitalWrite(GSM_STATUS_PIN, HIGH);
    }
    Serial.println("initialization done.");
};

void loop()
{
    char position;
    char phone_num[20]; // array for the phone number string
    char sms_text[100]; // array for the SMS text string

    if(started) {
        position = sms.IsSMSPresent(SMS_UNREAD);
        if (position) {
            // there is new SMS => read it
            sms.GetSMS(position, phone_num, sms_text, 100);
            #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG SMS phone number: ", 0);
            gsm.DebugPrint(phone_num, 0);
            gsm.DebugPrint("\r\n          SMS text: ", 0);
            gsm.DebugPrint(sms_text, 1);
            #endif
        }

        delay(1000);
    }

    // Look for new cards
    if (! mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Select one of the cards
    if (! mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Dump debug info about the card. PICC_HaltA() is automatically called.
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
};
