/* RS232 LCD SERIAL TESTER BY PER EMIL SKJOLD.
**HARDWARE:**
- Arduino Uno
- LCD Button shield
- RS232 serial shield (see connection)

**LIBRARY:**
- LiquidCrystal https://github.com/arduino-libraries/LiquidCrystal

**LCD:**
- 2x16 character lcd display.
- Serial Data prints on line 1. Menu prints on line 2.

**MENUS:**
- Menu 1 = Baud rate
- Menu 2 = Send counter on/off
- Menu 3 = Send Preset string
- Menu 4 = Record on/off
- Menu 5 = playback
- Menu 6 = Display non chars on/off
- Menu 7 = RX bytes counter (bytes received)

**CONNECTION:**
- RX PIN 0, TX PIN 1.

**HOMEPAGE:**
- https://skjolddisplay.com/projects/handheld-rs232-serial-data-tester/
- https://www.youtube.com/watch?time_continue=13&v=vVRO6BWD5QU

HISTORY:
- v15 Added menu: 'Non chars = On/Off' (Non chars is bytes below Dec32).
  Added menu: 'RX bytes: 0'.
- v16 Key press retrig avoided.
- v17 Added menu: Echo On/Off (returns data to sender).  
- v18 New analog values for buttons. 
      Put backlight() function in setup, and in loop as before. more info here http://arduino.cc/forum/index.php?topic=96747.0


*/

//EEPROM STRING
#include <EEPROM.h>
int addr = 0;// the current address in the EEPROM (i.e. which byte we're going to write to next)
boolean rec = false; //rec on/off controll

//LCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // pins for lcd display
const int lcdBacklightPin = 10;
unsigned long lastLight = 0; //lcd led


//KEYS
long baud[] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 256000};
byte baudIndex = 4; // default index 4(9600)
//int adc_key_val[5] = {50, 200, 400, 600, 800 }; //ANALOG VALUES FROM BUTTON SHIELD
int adc_key_val[5] = {50, 250, 450, 650, 850 }; // OTHER ANALOG VALUES, thanks to Tim.
byte NUM_KEYS = 5;
int adc_key_in;
byte key = -1;
byte oldkey = -1;
unsigned long lastHold = millis();
boolean holding = true; //sperr

//AUTOSEND
long previousMillis = 0;        // store last time x was updated
unsigned int testCounter = 0;
boolean sendCounterState = false;

//MENU
const byte menuTotalItems = 8; //number of total items in menu
byte menuSelectedItem = 1;
unsigned int rxByteCounter = 0;
boolean displayNonChars = false;
boolean echoState = false;

void menuUp() {
  menuSelectedItem++;
  if (menuSelectedItem > menuTotalItems) menuSelectedItem = 1; //reset
  updateMenuSelection();
}

void menuDown() {
  menuSelectedItem--;
  if (menuSelectedItem < 1) menuSelectedItem = menuTotalItems; //reset
  updateMenuSelection();
}

void msgSendt() {
  GOTOlcdLine2();
  lcd.print("Sendt           ");
  delay(100);
}

//RIGHT BUTTON,CHANGE MENU ITEM
void menuItemRightClick() {
  switch (menuSelectedItem) {

    case 1://CHANGE BAUD
      baudIndex++; //tell opp
      if (baudIndex > 11) baudIndex = 0;   //reset baudIndex
      //baudIndex % 11;
      Serial.end() ; //UNNGÅ HENG
      delay(150);
      GOTOlcdLine2();
      Serial.begin(baud[baudIndex]);
      break;

    case 2: // FLIP COUNTER STATE
      sendCounterState = !sendCounterState;
      break;


    case 3: // SEND PRESET
      intermec();
      msgSendt();
      break;


    case 4: //FLIP REC STATE
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

    case 5: //START PLAYBACK
      //Serial.println(F("Sending recorded:"));
      playback();
      break;
    
    case 6: //FLIP displayNonChars STATE
      displayNonChars=!displayNonChars;
      break;

    case 7: //CLEAR RX COUNTER
      rxByteCounter=0;
      break;
      
    case 8: //FLIP echoState
      echoState=!echoState;
      break;
      
  }
  updateMenuSelection();
}

//UP DOWN MENU ITEM CYCLE 
void updateMenuSelection() {
  GOTOlcdLine2();
  switch (menuSelectedItem) {


    case 1: //BAUD
      lcd.print("Baud = ");
      lcd.print(baud[baudIndex]);
      lcd.print("      ");
      Serial.print("Baudrate =  ");
      Serial.println(baud[baudIndex]);
      break;


    case 2: //COUNTER
      lcd.print("Send count = ");
      if (sendCounterState == true) {
        lcd.print("On ");
        
      }
      else {
        lcd.print("Off");
      }
      break;

    case 3: //PRESET
      lcd.print("Send Preset      ");
      break;


    case 4: //RECORD
      lcd.print("Record = ");
      if (rec == true) lcd.print("On    ");
      else lcd.print("Off   ");
      break;


    case 5: //playbackK
      lcd.print("Playback        ");
      break;

    case 6: //displayNonChars
      lcd.print("Non char = ");
      if (displayNonChars == true) lcd.print("On   ");
      else lcd.print("Off  ");
      break;
    
    case 7: //display rx bytes
      lcd.print("RX bytes: ");
      lcd.print(rxByteCounter);
      lcd.print("     ");
      break;

    case 8: //echo (return all bytes)
      lcd.print("Echo = ");
      if (echoState == true) lcd.print("On   ");
      else lcd.print("Off  ");
      break;
      
  }
  

  GOTOlcdLine1(); //goto serial rx line when done
}



void setup() {
  backlight(); // controls backlight. see link in function.
  Serial.begin(baud[baudIndex]);
  Serial.println(F("RS232 Serial data tester by Per Emil Skjold."));
  Serial.println(F("Use arrow up, down, right to scroll menu."));

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Handheld serial");
  lcd.setCursor(0, 1);
  lcd.print("data tester");
  delay(3000);
  lcd.clear();
  lcd.print("by Per Emil S");
  delay(2000);
  lcd.clear();
  lcd.print("rx pin = 0");
  lcd.setCursor(0, 1);
  lcd.print("tx pin = 1");
  delay(2000);
  lcd.clear();
  lcd.print("Menu = arrow up,");
  lcd.setCursor(0, 1);
  lcd.print("down, right.");
  delay(2000);
  lcd.clear();
  lcd.print("v18 Ready");

  updateMenuSelection();
}

// count bytes, and update menu now if in the menu
void updateMenu6RxBytes(){
      if(menuSelectedItem==7){
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
    //delay(100);// wait a bit for the entire message to arrive
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
      
      //echo data back to sender
      if (echoState) Serial.write(inchar);
      


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
    }// serial end
    
    //SERIAL END, SAVE END TO EEPROM
    //**************************************************
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
  delay (20);  //20ms debounce timer. Need this to sort messages on screen.

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
    if (sendCounterState == true) {
      Serial.print(F("Count Test nr."));
      Serial.println(testCounter);
      testCounter++;
    }

  }
}


//READ BUTTONS
void readAnalog() {
  adc_key_in = analogRead(0);    // read the value from the sensor
  key = get_key(adc_key_in);  // convert into key press. KEY ER -1 NÅR INGEN ER NEDE

 if (key != oldkey) {  // new keypress is detected
  oldkey=key;
  if (key >= 0)  {   // keypress is detected
    // ONE SHOT EVENTS ON KLICK DOWN
    lastLight = millis(); //SKRU PÅ LYS
    backlight();
    doKeyDown();
    delay(100);  // 200ms button debounce time
  }
}  

//  // BUTTON HOLD, NOT IN USE..
//  else {
//    //RESET HOLD VED KNAPP AV
//    unsigned long currentMillis = millis();
//    lastHold = currentMillis; //sett startpunktet for holde tid
//    holding = false; //klargjør
//  }
}


void doKeyDown() {

  //UP KEY (1)
  if (key == 1) { //skift funksjon sørger for at println går kun en loop.
    menuDown();
  }

  //DOWN KEY (2)
  if (key == 2) { //skift funksjon sørger for at println går kun en loop.
    menuUp();
  }

  //LEFT
  //   if (key == 3){ //skift funksjon sørger for at println går kun en loop.

  //        }

  //RIGHT
  if (key == 0) {
    menuItemRightClick();
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

// not sure what this is 10years later..
void backlight() {
  // // set the pin to input mode to turn the backlight on and to output and low to turn the LED off.
  // // http://arduino.cc/forum/index.php?topic=96747.0
  //    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  unsigned long currentMillis = millis();
  if ((currentMillis - lastLight) >= 60000 ) { //AV ETTER 60sek.
    //OFF
    pinMode(lcdBacklightPin, OUTPUT);
    digitalWrite(lcdBacklightPin, LOW);
  }
  else {
    //ON
    pinMode(lcdBacklightPin, INPUT);
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
  //delay(100);

}
