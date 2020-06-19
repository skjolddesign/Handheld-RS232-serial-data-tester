/* RS232 LCD SERIAL TESTER BY PER EMIL SKJOLD.
   LCD:
    2x16 character lcd display.
    Data on LCD line 1. Menu on LCD line 2.
   MENU:
    Menu 1 = Baud rate
    Menu 2 = Send counter
    Menu 3 = Send Preset string
    Menu 4 = Record on/off
    Menu 5 = playback
   PIN CONNECTION:
    RX PIN 0, TX PIN 1.
   HOMEPAGE:
    https://skjolddisplay.com/projects/handheld-rs232-serial-data-tester/
    https://www.youtube.com/watch?time_continue=13&v=vVRO6BWD5QU&feature=emb_title
   HISTORY:
    v15 Added menu item, display non chars. Bytes received below Dec32 is displayed on lcd.
        Added menu item, RX bytes counter.


*/

//EEPROM STRING
#include <EEPROM.h>
// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;
boolean rec = false; //minne for styring opptak til eeprom

//LCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // pins for lcd display
const int light = 10;
unsigned long lastLight = 0; //lcd led
boolean displayNonChars = false;

//KEYS
long baud[] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 256000};
int baudIndex = 4;
int adc_key_val[5] = {50, 200, 400, 600, 800 }; //ANALOG VALUES FROM BUTTON SHIELD
int NUM_KEYS = 5;
int adc_key_in;
int key = -1;
int oldkey = -1;
boolean left = false; //MINNE FOR AV PÅ LEFT KNAPP AUTOSEND
unsigned long lastHold = millis();
boolean holding = true; //sperr

//AUTOSEND
long previousMillis = 0;        // store last time x was updated
unsigned int testRunde = 0;
boolean autoSendStatus = false;

//MENU
const byte menuItems = 6; //number of total items in menu
int menuSelected = 0;
unsigned int rxByteCounter = 0;


void menyOpp() {
  menuSelected++;
  if (menuSelected > menuItems) menuSelected = 0; //reset
  updateMenuSelection();
}

void menyNed() {
  menuSelected--;
  if (menuSelected < 0) menuSelected = menuItems; //reset
  updateMenuSelection();
}

void msgSendt() {
  GOTOlcdLine2();
  lcd.print("Sendt           ");
  delay(500);
}

//OK BUTTON,CHANGE MENU ITEM
void caseChangeItem() {
  switch (menuSelected) {

    case 0://CHANGE BAUD
      baudIndex++; //tell opp
      if (baudIndex > 11) baudIndex = 0;   //reset baudIndex
      Serial.end() ; //UNNGÅ HENG
      delay(150);
      GOTOlcdLine2();
      Serial.begin(baud[baudIndex]);
      break;

    case 1: // FLIP COUNTER STATE
      autoSendStatus = !autoSendStatus;
      break;


    case 2: // SEND 
      intermec();
      msgSendt();
      break;


    case 3: //FLIP REC STATE
      if (rec == true) { //DO
        //STOPP
        rec = false;
        //rec end:
        EEPROM.write(addr, 255); //lagre TOM FOR LES LOOP AVBRYT
        Serial.println(F("Rec Stoppped"));
      }
      else {
        //START
        addr = 0; //nullstill adresse
        rec = true;
        Serial.println(F("Rec Started   "));
      }
      break;

    case 4: //START PLAYBACK
      //Serial.println(F("Sending recorded:"));
      playback();
      break;
    
    case 5: //FLIP displayNonChars STATE
      displayNonChars=!displayNonChars;
      break;
      
  }
  updateMenuSelection();
}

//UP DOWN MENU ITEM CYCLE 
void updateMenuSelection() {
  GOTOlcdLine2();
  switch (menuSelected) {


    case 0: //BAUD
      lcd.print("Baud = ");
      lcd.print(baud[baudIndex]);
      lcd.print("      ");
      Serial.print("Baudrate =  ");
      Serial.println(baud[baudIndex]);
      break;


    case 1: //COUNTER
      lcd.print("Send count = ");
      if (autoSendStatus == true) {
        lcd.print("On ");
        
      }
      else {
        lcd.print("Off");
      }
      break;

    case 2: //PRESET
      lcd.print("Send Preset      ");
      break;


    case 3: //RECORD
      lcd.print("Record = ");
      if (rec == true) lcd.print("On    ");
      else lcd.print("Off   ");
      break;


    case 4: //playbackK
      lcd.print("Playback        ");
      break;

    case 5: //displayNonChars
      lcd.print("Non char = ");
      if (displayNonChars == true) lcd.print("On   ");
      else lcd.print("Off  ");
      break;
    
    case 6: //display rx bytes
      lcd.print("RX bytes: ");
      lcd.print(rxByteCounter);
      lcd.print("     ");
      break;
      
  }
  

  GOTOlcdLine1(); //goto serial rx line when done
}



void setup() {
  Serial.begin(baud[baudIndex]);
  Serial.println(F("RS232 Serial data tester by Per Emil Skjold."));
  Serial.println(F("Use arrow up, down, right to scroll menu."));

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Serial Tester");
  lcd.setCursor(0, 1);
  lcd.print("v15 by PES");
  delay(3000);
  lcd.clear();
  lcd.print("Menu arrow up,");
  lcd.setCursor(0, 1);
  lcd.print("down, right.");
  delay(3000);
  lcd.clear();
  lcd.print("Ready");

  updateMenuSelection();
}

// count bytes, and update menu now if in the menu
void updateMenu6RxBytes(){
      if(menuSelected==6){
      lcd.setCursor(0, 1);
      lcd.print("RX bytes: ");
      lcd.print(rxByteCounter);
      lcd.print("     ");
      }
}

void loop(){

  // SERIAL READ
  if (Serial.available()) { //SERIAL PORT STARTET
    boolean freshdata = true;
    // wait a bit for the entire message to arrive
    //delay(100);
    //LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
    lastLight = millis(); //SKRU PÅ LYS
    // read all the available characters
    while (Serial.available() > 0) { //SERIAL MOTTAS
      char inchar = Serial.read();
      rxByteCounter+=1;

      // clear line 1 when data starts
      if(freshdata){
        GOTOlcdLine1();
        lcd.print("                ");//clear old data on line 1
        freshdata=false;
        GOTOlcdLine1();
      }
 
      // display data, with non chars if selected
      if (displayNonChars) lcd.write(inchar);
      else if (inchar >= 31) lcd.write(inchar); //write only ascii chars to lcd
      


      // RECORD TO EEPROM
      //***********************************************************
      if (rec == true) {
        //SAVE CHAR TO EEPROM
        if (addr <= 1023) {
          EEPROM.write(addr, inchar); //lagre til adresse
          addr = addr + 1; //gjør klar til neste adresse
          GOTOlcdLine1();
          lcd.print("Saved ");
          lcd.print(addr);
          lcd.print(" bytes   ");
          // GOTOlcdLine1();
        }
        else { //FULL
          GOTOlcdLine1();
          lcd.print("EEPROM full      ");
        }
      }
      //**************************************************
    }//WHILE SERIAL SERIAL END
    //lcd.print("                ");//FILL
    updateMenu6RxBytes(); //THIS WILL UPDATE COUNTER WHEN RX MESSAGE IS DONE
    GOTOlcdLine1();

    //ADD LINE END 13 Og 10 BYTE TO EEPROM AT SERIAL END.
    if (rec == true) {
      if (addr <= 1023) {
        //EEPROM.write(addr, 13); //lagre CR
        //addr+=1; //set for next adress
        //EEPROM.write(addr, 10); //lagre LF
        //addr+=1; //set for next adress
        GOTOlcdLine1();
        lcd.print("Saved ");
        lcd.print(addr);
        lcd.print(" bytes   ");
      }
      else { //FULL
        GOTOlcdLine1();
        lcd.print("EEPROM full      ");
      }
    }

  }//IF SERIAL SLUTT



  //*************************************************************
  readAnalog();
  delay (20);  //debounce timer.


  sendCount();
  backlight();
}


void sendCount() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > 1000) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    //DO
    //AUTO TEST STRENG UT
    if (autoSendStatus == true) {
      Serial.print(F("Count Test nr."));
      Serial.println(testRunde);
      testRunde++;
    }

  }
}


//READ BUTTONS
//***************************************************************************
void readAnalog() {
  adc_key_in = analogRead(0);    // read the value from the sensor
  key = get_key(adc_key_in);  // convert into key press. KEY ER -1 NÅR INGEN ER NEDE

  //  if (key != oldkey)   // new keypress is detected
  if (key >= 0)   // keypress is detected
  {
    // ONE SHOT EVENTS ON KLICK DOWN
    lastLight = millis(); //SKRU PÅ LYS
    backlight();
    doKeyDown();
    delay(400);  // wait for debounce time

    adc_key_in = analogRead(0);    // read the value from the sensor
    key = get_key(adc_key_in);    // convert into key press

    if (key != oldkey)   //NY KNAPP
    {
      oldkey = key;
      if (key >= 0) {
        //  lcd.setCursor(0, 1);
        //  lcd.print(msgs[key]);  //BUTTON TEXT
      }
    }
  }

  // BUTTON HOLD, NOT IN USE..
  else {
    //RESET HOLD VED KNAPP AV
    unsigned long currentMillis = millis();
    lastHold = currentMillis; //sett startpunktet for holde tid
    holding = false; //klargjør
  }
}


//***************************************************************************
void doKeyDown() {

  //UP KEY (1)
  if (key == 1) { //skift funksjon sørger for at println går kun en loop.
    menyNed();
  }

  //DOWN KEY (2)
  if (key == 2) { //skift funksjon sørger for at println går kun en loop.
    menyOpp();
  }

  //LEFT
  //   if (key == 3){ //skift funksjon sørger for at println går kun en loop.

  //        }

  //RIGHT
  if (key == 0) {
    caseChangeItem();
  }

}


void GOTOlcdLine1() {
  lcd.setCursor(0, 0);
  //Serial.println(F("rad 1"));
}

void GOTOlcdLine2() {
  lcd.setCursor(0, 1);
  // Serial.println(F("rad 2"));
}

//INTERMEC
//***************************************************************************
void intermec() {
  Serial.println(F("#START"));
  Serial.println(F("86"));
  Serial.println(F("OEP01;11.01.12;0,000;;;232089310000"));
  Serial.println(F("PRIS"));
  Serial.println(F("Se Hylleetikett"));
  Serial.println(F("Svinefilet"));
  Serial.println(F("INGREDIENSER:"));
  Serial.println(F("Svinekjott norsk opprinnelse"));
  Serial.println(F(""));
  Serial.println(F("Kjolevare 0-4 °C"));
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("Siste forbruksdag:       "));
  Serial.println(F("141"));
  Serial.println(F("NORGE10.2"));
  Serial.println(F("                         "));
  Serial.println(F(""));
  Serial.println(F("21.12.11"));
  Serial.println(F("#SLUTT"));
}




// Convert ADC value to key number
int get_key(unsigned int input) {
  int k;
  for (k = 0; k < NUM_KEYS; k++)
  {
    if (input < adc_key_val[k])
    {
      return k;
    }
  }
  if (k >= NUM_KEYS)k = -1;  // No valid key pressed
  return k;
}


void backlight() {
  // // set the pin to input mode to turn the backlight on and to output and low to turn the LED off.
  // // http://arduino.cc/forum/index.php?topic=96747.0
  //    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  unsigned long currentMillis = millis();
  if ((currentMillis - lastLight) >= 60000 ) { //AV ETTER 60sek.
    //OFF
    pinMode(light, OUTPUT);
    digitalWrite(light, LOW);
  }
  else {
    //ON
    pinMode(light, INPUT);
  }
}




//playback EEPROM
void playback() {
  byte data = 0;
  //int address = 0;
  //lcd.setCursor(0, 0);

  GOTOlcdLine2();//plassering
  for (int address = 0; address <= 1023; address++) { //LOOP FRA 0 TIL 1023
    data = EEPROM.read(address);// read a byte from the current address of the EEPROM
    if (data == 255)break; //AVBRYT LOOP VED TOM EEPROM VERDI
    //  Serial.print(i);
    //  Serial.print("\t"); //TAB
    //  Serial.print(data, DEC);//SOM INT 0-255, HEX FF, 1BYTE,
    //  Serial.print("--");
    Serial.write(data); //BRUK WRITE, FOR Å SKRIVE BYTE VERDIER SOM ASCII
    //  lcd.write(data); //VIS PÅ LCD OGSÅ
    //  Serial.print(data); //SOM BYTE TALL
    //  lcd.write(data);
  }
  // Serial.println(); //SKRIVE NEWLINE PÅ NY
  //lcd.write("                 ");
  //GOTOlcdLine1();
  lcd.print("Sendt       ");
  delay(500);



}

