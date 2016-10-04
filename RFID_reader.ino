/*
   @license Released into the public domain, source: https://github.com/miguelbalboa/rfid

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/


//Include libaries
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>


//Defining the pins for the RFID reader
#define RST_PIN         5         // Configurable, see typical pin layout above
#define SS_PIN          53         // Configurable, see typical pin layout above


//define methodes
void setTriggers();
void readEEPROM();

//Card stuff
int newCard[] = {0, 0};
int oldCard;
int loginCode_1[] = {239,                  219,         48,                   236,         404};
int loginCode_2[] = {42,                   42,          123,                  40};
String loginName[] = {"On Body Gadget", "Losse Chip", "Fontys Kaart Thijs", "Wiite Kaart"};

//Timing
int interval = 2500;
unsigned long previousMillis = 0;

//Password for disarming
String password = "github";

//Alarm settings
int toAlarm = 22, toController = 23;
int triggerAlarm, triggerController; //eeprom 0, eeprom 1
boolean alarmDisable = false;

//Serial settings
boolean serialFeedback; //eeprom 2
String inputString = "";

//Give the RFID a thing
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance



void setup() {
  Serial.begin(9600);
  while (!Serial);

  //Set the pinmodes to OUTPUT
  pinMode(toAlarm, OUTPUT);
  pinMode(toController, OUTPUT);

  //Read the eeprom and set the output value's to not active
  setTriggers();

  //Start comminucation with the reader
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  //If feedback is enabled
  if (serialFeedback)
    Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {
  //Get the time
  unsigned long currentMillis = millis();

  //delay function but without the delay
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //reset the card en set the outputs to not active
    oldCard = 0;
    setTriggers();
  }


  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Read the first and second values of the UID
  newCard[0] = mfrc522.uid.uidByte[1];
  newCard[1] = mfrc522.uid.uidByte[2];

  //Check if the new card isn't the same as the last card
  if (newCard[0] != oldCard)
  {
    //Set the newcard as last card
    oldCard = newCard[0];
    if (serialFeedback)
      Serial.print("\nCard UID:");    //Dump UID

    //Read the values and print them in the serial monitor if the feedback is enabled
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (serialFeedback) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], DEC);
      }
    }
    if (serialFeedback)
      Serial.print("\t\t");

    //make somethings for the whileloop
    bool getOut = true;
    bool succes = false;
    int i = 0;
    while (getOut)
    {
      //Check if the values are equal to the values that are programmed
      if (loginCode_1[i] == newCard[0])
      {
        if (loginCode_2[i] == newCard[1])
        {
          //Return the name of the card if feedback is enabled
          if (serialFeedback) {
            Serial.print("Kaart: ");
            Serial.println(loginName[i]);
          }
          //set succes to true so the error message doesn't display
          succes = true;
          previousMillis = currentMillis; //set this line for the delay function at the top of the main
          digitalWrite(toAlarm, triggerAlarm); //Trigger alarm so it will go off
        }
      }
      if (loginCode_1[i + 1] == 404) {
        //If the login code is 404 (value that can't be in a card) so it will end
        getOut = false;
        previousMillis = currentMillis;
      }
      i++; // count i++
    }

    //If the card isn't in the system
    if (succes == false) {
      //Print that the card is unknown if enabled
      if (serialFeedback)
        Serial.println("Kaart: Onbekend");
      //Trigger the controller so the alarm will go of
      digitalWrite(toController, triggerController);
    }
  }
}

/******************************************
   Functionname: setTriggers
   Programmer:  Thijs Tops
   Inputs:      None
   Returning:   None
   Function:    Set the triggers to the not active value (alarm won't if the it's disabled)
******************************************/

void setTriggers() {
  readEEPROM();
  if (!alarmDisable)
    digitalWrite(toAlarm, !triggerAlarm);
  digitalWrite(toController, !triggerController);
}

/******************************************
   Functionname: readEEPROM
   Programmer:  Thijs Tops
   Inputs:      None
   Returning:   None
   Function:    Read the values from the EEPROM and set them in the memory. This to save the values if the power goes off
******************************************/

void readEEPROM() {
  triggerAlarm = EEPROM.read(0);
  triggerController = EEPROM.read(1);
  serialFeedback = EEPROM.read(2);
}

/******************************************
   Functionname: serialEvent
   Programmer:  Thijs Tops
   Triggers:    Serial monitor input
   Inputs:      None
   Returning:   None
   Function:    If there is a serial input see what it is, and do what is explane in more details in the code
******************************************/

void serialEvent() {

  //Example code! -------------------------
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
   //Example code! -------------------------

      //If the input is "?" and return the help page
      if (inputString.indexOf("?") > -1) {
        Serial.println("\n0: Change triggers");
        Serial.println("1: Serial feedback");
        Serial.println("2: Trigger Alarm");
        Serial.println("3: Trigger Controller");
        Serial.println("4: Test Alarm");
        Serial.println("5: Alarm off");
        Serial.println("6: Disarm alarm");
        Serial.println("7: Enable alarm");
        Serial.println("8: Not in use");
        Serial.println("9: Not in use");
      }

      //If the input is "0_1" then change the trigger output from the alarm
      else if (inputString.indexOf("0_1") > -1) {
        Serial.print("Set triggerAlarm: ");
        if (triggerAlarm == 0) {
          EEPROM.write(0, 1);
          Serial.println("HIGH\n");
        } else {
          EEPROM.write(0, 0);
          Serial.println("LOW\n");
        }
        setTriggers();

      }

      //If the input is "0_2" then change the trigger output from the controller
      else if (inputString.indexOf("0_2") > -1) {
        Serial.print("Set triggerController: ");
        if (triggerController == 0) {
          EEPROM.write(1, 1);
          Serial.println("HIGH\n");
        }
        else {
          EEPROM.write(1, 0);
          Serial.println("LOW\n");
        }
        setTriggers();

      }

      //If the input is "0" then return the page for the triggers
      else if (inputString.indexOf("0") > -1) {
        Serial.print("0_1: triggerAlarm: ");
        if (triggerAlarm == 1)
          Serial.println("HIGH");
        else
          Serial.println("LOW");

        Serial.print("0_2: triggerController: ");
        if (triggerController == 1)
          Serial.println("HIGH\n");
        else
          Serial.println("LOW\n");

      }

      //reverse the feedback
      else if (inputString.indexOf("1_1") > -1) {
        Serial.print("Changed serialFeedback to: ");
        if (serialFeedback) {
          Serial.println("false\n");
          EEPROM.write(2, 0);
        }
        else {
          Serial.println("true\n");
          EEPROM.write(2, 1);
        }
        readEEPROM();
      }

      //Return the state of the feedback
      else if (inputString.indexOf("1") > -1) {
        Serial.println("1_1: Toggle serialfeedback");
        Serial.print("\tCurrent state: ");
        if (serialFeedback)
          Serial.println("true\n");
        else
          Serial.println("false\n");
      }

      //Trigger the alarm
      else if (inputString.indexOf("2") > -1) {
        digitalWrite(toAlarm, triggerAlarm);
        Serial.println("triggerd alarm");
      }

      //Trigger the controller
      else if (inputString.indexOf("3") > -1) {
        digitalWrite(toController, triggerController);
        Serial.println("triggerd Controller");
      }

      //Test the alarm by first triggering the controller and couple seconds later the alarm so sound will go away
      else if (inputString.indexOf("4") > -1) {
        digitalWrite(toAlarm, !triggerAlarm);
        digitalWrite(toController, triggerController);
        Serial.println("triggerd Controller");
        delay(2500);
        digitalWrite(toAlarm, triggerAlarm);
        Serial.println("triggerd alarm");
        delay(1000);
        setTriggers();
        Serial.println("Test Done!");
        if (alarmDisable)
          digitalWrite(toAlarm, triggerAlarm);
      }

      //If the alarm goes off and the cards won't deactivate it then this command will do it
      else if (inputString.indexOf("5") > -1)
      {
        digitalWrite(toAlarm, triggerAlarm);
        digitalWrite(toController, !triggerController);
        delay(1000);
        digitalWrite(toAlarm, !triggerAlarm);
        Serial.println("Alarm is quite!");
      }

      //Disarm the alarm, works only with the password at the declaration of this program
      else if (inputString.indexOf("6") > -1) {
        if (inputString.indexOf(password) > -1) {
          digitalWrite(toAlarm, triggerAlarm);
          Serial.println("Alarm disabled");
          alarmDisable = true;
        } else
          Serial.println("No password provided");
      }

      //If armed this wil dearm it
      else if (inputString.indexOf("7") > -1) {
        digitalWrite(toAlarm, !triggerAlarm);
        Serial.println("Alarm enabled");
        alarmDisable = false;
      }
      inputString = "";
    }
  }
}
