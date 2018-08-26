#include <DS3231.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <OneWire.h>                         //dodaj biblitekę OneWire
#include <DallasTemperature.h>               //dodaj biblitekę obsługującą DS18B20


#define DS3231_I2C_ADDRESS 0x68
// Software SPI (slower updates, more flexible pin options):
// pin 12 - Serial clock out (SCLK)
// pin 7 - Serial data out (DIN)
// pin 4 - Data/Command select (D/C)
// pin 25 - LCD chip select (CS)(SCE)
// pin 5 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 7, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!
int one_wire = 10;                            //Transmisja 1-Wire na pinie 10

OneWire oneWire(one_wire);                   //wywołujemy transmisję 1-Wire na pinie 10
DallasTemperature sensors(&oneWire);         //informujemy Arduino, ze przy pomocy 1-Wire
//chcemy komunikowac sie z czujnikiem

DS3231  rtc(SDA, SCL);

volatile int counter = 0;
float temp0 = 0;
float temp1 = 0;
float e = 0;
float e0 = 0;

float i = 0;
float d = 0;
int triac_delay = 0;


String time = "";

unsigned int onTimeH = 8;
unsigned int onTimeM = 0;

unsigned int offTimeH = 22;
unsigned int offTimeM = 30;

unsigned int onTimeHtmp = 8;
unsigned int onTimeMtmp = 0;

unsigned int offTimeHtmp = 22;
unsigned int offTimeMtmp = 30;

bool mainScreen = true;
bool editScreen = false;
bool temp0edit = false;
bool lightOnTimeEdit = false;
bool lightOffTimeEdit = false;
bool temp0editSelected = true;
bool lightOnEditSelected = false;
bool lightOffEditSelected = false;



float temp0set = 35;
float temp0temporary = 35;

bool up = false;
bool down = false;
bool left = false;
bool right = false;

bool btnEdit = false;
bool btnOk = false;
bool btnBack = false;
bool btnFour = false;




/**
   set update on a high edge
*/
ISR(PCINT0_vect) {
  if ( (PINB & (1 << PINB0)) == 1 ) {
    
    /* LOW to HIGH pin change */
    EIMSK &= ~(1 << INT0);
    //PORTB |= (1<<PB1);
    //OCR0A = 1;
    //TIMSK0 |= (1<< OCIE0A); // Timer/Counter0 Compare Match A interrupt is enabled
    //TCNT0 = 0x00;
    // _delay_us(1000);
    //PORTB &= ~(1<<PB1);
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    _delay_us(20);                       // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    //Serial.begin(57600);
    EIMSK |= (1 << INT0);
    OCR0A = triac_delay; // 1 => 0.000064 s
    TCCR0A |= (1 << WGM21); //Wlaczenie trybu CTC (Clear on compare match)
    TIMSK0 |= (1 << OCIE0A); //Wystapienie przerwania gdy zliczy do podanej wartosci
    TCCR0B |= (1 << CS02)|(1 << CS00); //Ustawienie dzielnika na 256, po czym start

    counter++;
  }
  else {

  }
}

 ISR (TIMER0_COMPA_vect) { //Obsluga przerwania
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    _delay_us(20);                       // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  TCCR0B = 0;
  TCCR0A = 0;
  TIMSK0 = 0;
  TCNT0 = 0;
}



void setup()   {

  PCICR |= (1 << PCIE0);
  DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
  PCMSK0 |= (1 << PCINT0);
  DDRB |= (0 << PB0);
  PORTB |= (1 << PB0);

  pinMode(9, OUTPUT);
  // Serial.begin(57600);
  //OCR0A = 1;
  //TIMSK0 |= (1<< OCIE0A); // Timer/Counter0 Compare Match A interrupt is enabled
  //TCNT0 = 0x00;
  EICRA |= (1 << ISC01) | (1 << ISC00); //INT0 rising edge - not working

  //TCCR0A |= (1<<COM0A1)|(0<<COM0A0)|(1<<CS01)|(1<<CS00); //64 prescaler, output compare, clkio=1mhz, 15625Hz, 0,000064s period

  display.begin();
  // init done
  Wire.begin();
  rtc.begin();
  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.clearDisplay();
  sensors.begin();                          //rozpocznij odczyt z czujnika
  pinMode(13, OUTPUT);           // set pin to input
  // turn on pullup resistors
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(TUESDAY);     // Set Day-of-Week to SUNDAY

  rtc.setTime(14, 55, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(22, 8, 2017);   // Set the date to January 1st, 2014
  EIMSK |= (1 << INT0);
  sei();
}

// 8 - zero cross

void loop() {
  int val = 0;
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;

  bool btnOk = false;
  bool btnBack = false;
  bool btnEdit = false;
  bool btnFour = false;
  int button = analogRead(A3);

  if ((972 <= button) && (button <= 975)) {
    left = true;
  }
  else if ((904 <= button) && (button <= 907)) {
    right = true;
  }
  else if ((995 <= button) && (button <= 998)) {
    up = true;
  }
  else if ((943 <= button) && (button <= 946)) {
    down = true;
  }
  else if ((457 <= button) && (button <= 459)) {
    btnOk = true;
  }
  else if ((662 <= button) && (button <= 664)) {
    btnBack = true;
  }
  else if ((778 <= button) && (button <= 780)) {
    btnEdit = true;
  }
  else if ((853 <= button) && (button <= 855)) {
    btnFour = true;
  }
  else {

  }

  //ekran edycji, wybór opcji edycji
  if (mainScreen && btnEdit) {
    editScreen = true;
    btnEdit = false;
    mainScreen = false;
  }
  else if (editScreen && temp0editSelected) {
    if (btnOk) {
      temp0edit = true;
      editScreen = false;
      delay(300);
      btnOk = false;
    }
    else if (down) {
      lightOnEditSelected = true;
      temp0editSelected = false;
      delay(300);
      down = false;
    }
    else if (up) {
      lightOffEditSelected = true;
      temp0editSelected = false;
      delay(300);
      up = false;
    }
  }
  else if (editScreen && lightOnEditSelected) {
    if (btnOk) {
      lightOnTimeEdit = true;
      editScreen = false;
      delay(300);
      btnOk = false;
    }
    else if (down) {
      lightOffEditSelected = true;
      lightOnEditSelected = false;
      delay(300);
      down = false;
    }
    else if (up) {
      temp0editSelected = true;
      lightOnEditSelected = false;
      delay(300);
      up = false;
    }
  }
  else if (editScreen && lightOffEditSelected) {
    if (btnOk) {
      lightOffTimeEdit = true;
      editScreen = false;
      delay(300);
      btnOk = false;
    }
    else if (down) {
      temp0editSelected = true;
      delay(300);
      down = false;
    }
    else if (up) {
      lightOnEditSelected = true;
      delay(300);
      up = false;
    }
  }
  else {}

  if (editScreen && btnBack) {
    editScreen = false;
    mainScreen = true;
    btnBack = false;
  }

  if (temp0edit) {
    if (up) {
      temp0temporary += 0.1;
      delay(100);
    }
    else if (down) {
      temp0temporary -= 0.1;
      delay(100);
    }
    else if (btnOk) {
      temp0set = temp0temporary;
      temp0edit = false;
      editScreen = true;
      delay(200);
    }
    else if (btnBack) {
      temp0edit = false;
      editScreen = true;
      delay(200);
    }
    else {
    }
  }
  if (lightOnTimeEdit) {
    if (down) {
      if (onTimeMtmp < 60) {
        onTimeMtmp -= 1;
        delay(100);
      }
      else {
        onTimeMtmp = 59;
        onTimeHtmp -= 1;
      }
      if (onTimeHtmp > 24) {
        onTimeHtmp = 24;
      }
    }
    else if (up) {
      if (onTimeMtmp < 60) {
        onTimeMtmp += 1;
        delay(100);
      }
      else {
        onTimeMtmp = 00;
        onTimeHtmp += 1;
      }
      if (onTimeHtmp > 24) {
        onTimeHtmp = 24;
      }
    }
    else if (btnOk) {
      onTimeH = onTimeHtmp;
      onTimeM = onTimeMtmp;
      lightOnTimeEdit = false;
      editScreen = true;
      delay(200);
    }
    else if (btnBack) {
      lightOnTimeEdit = false;
      editScreen = true;
      delay(200);
    }
    else {
    }
  }
    else if (lightOffTimeEdit) {
      if (down) {
        if (offTimeMtmp < 60) {
          offTimeMtmp -= 1;
          delay(100);
        }
        else {
          offTimeMtmp = 59;
          offTimeHtmp -= 1;
        }
        if (offTimeHtmp > 24) {
          offTimeHtmp = 24;
        }
      }
      else if (up) {
        if (offTimeMtmp < 60) {
          offTimeMtmp += 1;
          delay(100);
        }
        else {
          offTimeMtmp = 00;
          offTimeHtmp += 1;
        }
        if (offTimeHtmp > 24) {
          offTimeHtmp = 24;
        }
      }
      else if (btnOk) {
        offTimeH = offTimeHtmp;
        offTimeM = offTimeMtmp;
        lightOffTimeEdit = false;
        editScreen = true;
        delay(200);
      }
      else if (btnBack) {
        lightOffTimeEdit = false;
        editScreen = true;
        delay(200);
      }
      else {
      }
    }


    





    display.clearDisplay();


    if (counter >= 100) {
      counter = 0;
      sensors.requestTemperatures();            //zazadaj odczyt temperatury z czujnika
      temp0 = sensors.getTempCByIndex(0);
      temp1 = sensors.getTempCByIndex(1);
      time = rtc.getTimeStr();

      e = temp0set - temp0;
      i += e;
      d = (e - e0);

      triac_delay = int(155 - 0.08*e - 0.08*i + 0.9*d);
      if (triac_delay > 153){
        triac_delay = 153;
      }
      if (triac_delay < 4){
        triac_delay = 4;
      }
      e0 = e;
    }

    if (mainScreen) {
      display.setCursor(1, 1);                 //ustaw kurs w pozycji 1 kolumna 1 wiersz
      display.print("Temp0: ");                  //wyświetl "Temp: "
      display.setCursor(40, 1);                //ustaw kursor w pozycji 34 kolumna 20 wiersz

      display.print(temp0);//wyświetl odczytaną temperaturę z czujnika
      display.print((char)247);                 //wyświetl znak stopnia
      display.print("C");                       //....

      display.setCursor(1, 10);                 //ustaw kurs w pozycji 5 kolumna 20 wiersz
      display.print("Temp1: ");                  //wyświetl "Temp: "
      display.setCursor(40, 10);                //ustaw kursor w pozycji 34 kolumna 20 wiersz
      display.print(temp1);//wyświetl odczytaną temperaturę z czujnika
      display.print((char)247);                 //wyświetl znak stopnia
      display.print("C");                       //....

      display.setCursor(1, 20);

      display.print(time);
      display.setCursor(1, 30);

      display.print(triac_delay);
    }
    else if (editScreen) {
      display.setCursor(1, 1);
      display.print("Temp0");
      display.setCursor(1, 10);
      display.print("Dzien");
      display.setCursor(1, 20);
      display.print("Noc");

      if (temp0editSelected) {
        display.drawRect(35, 1, 7, 7, BLACK);
      }
      else if (lightOnEditSelected) {
        display.drawRect(35, 11, 7, 7, BLACK);
      }
      else if (lightOffEditSelected) {
        display.drawRect(25, 20, 7, 7, BLACK);
      }

    }
    else if (temp0edit) {
      display.setCursor(1, 1);
      display.print(temp0temporary);
    }
    else if (lightOnTimeEdit) {
      display.setCursor(1, 1);
      display.print("Dzien: ");
      display.setCursor(1, 20);
      display.print(onTimeHtmp);
      display.print(":");
      if (onTimeMtmp < 10) {
        display.print("0");
      }
      display.print(onTimeMtmp);
      display.print(":");
      display.print("00");
    }
    else if (lightOffTimeEdit) {
      display.setCursor(1, 1);
      display.print("Noc: ");
      display.setCursor(1, 20);
      display.print(offTimeHtmp);
      display.print(":");
      if (offTimeMtmp < 10) {
        display.print("0");
      }
      display.print(offTimeMtmp);
      display.print(":");
      display.print("00");
    }

    display.display();

  }




