#include <LCDWIKI_GUI.h>
#include <LCDWIKI_KBV.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <FastLED.h>

LCDWIKI_KBV mylcd(ILI9486, 40, 38, 39, -1, 41);

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define NUM_LEDS 16
#define DATA_PIN 13
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define BRIGHTNESS 60
#define VOLTS 5
#define MAX_AMPS 500

CRGB leds[NUM_LEDS];

unsigned long currentLedMillis;
unsigned long ledMillis = 63;

int ledCount = 0;
int ledCount2 = 0;
int tracker = 0;
int next = 0;
bool validate; 
int i = 0;
// these are for the timing of the leds

int admin = 0;
int trycount = 0;
int keycount = 0;
int Minutes = 30;
int Seconds = 0;
int oldMinutes, newMinutes, oldSeconds, newSeconds;
int game = 0, gcount = 0;
int gamemode = 0;
// these are for the setup of the games

int btime = 0, rtime = 0;
int bmin, bsec, obm, obs;
int rmin, rsec, orm, ors;
unsigned long bsecMillis = 0, rsecMillis = 0;
unsigned long secMillis = 0;
long interval = 1000;
unsigned long currentBMillis;
unsigned long currentRMillis;
unsigned long currentMillis;
//these are for the domination gamemode


int fpass1, fpass2, fpass3;
int pos1, pos2, pos3, pos4;
int parola;
int rantrig = 0;
int f1 = 0, f2 = 0, f3 = 0;


char password[4];
char entered[4];
char try1[4];
char try2[4];
char try3[4];
int inc1, inc2, inc3; 
//these are for the coalition gamemode


const byte ROWS = 4;
const byte COLS = 4;
char Keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 5, 4, 3, 2 };
byte colPins[COLS] = { 9, 8, 7, 6 };


Keypad keypad = Keypad(makeKeymap(Keys), rowPins, colPins, ROWS, COLS); //you create  a bitmap for your keypad, the pinout of your keypad can be found online

int min, sec, newMin, newSec, oldMin, oldSec, timeCount = 0, numKey = 0, smin = 0, ssec = 0;
int resetCount = 0;
String inputStr; // here will be stored whatever we type

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_AMPS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  mylcd.Init_LCD();
  mylcd.Fill_Screen(BLACK);
  pinMode(A0, INPUT_PULLUP);//this is the admin switch
  sec = EEPROM.read(0);
  min = EEPROM.read(1); 
  gamemode = EEPROM.read(2);//reads the time and game stored in eeprom
  randomSeed(analogRead(3)); //we use a seed to get different random generated passwords, without this you will get the same ones
  pinMode(11, INPUT); 
  pinMode(10, INPUT);
  //the pins of the switch for the domination game, one is for the blue team and one for the red team

  int but = digitalRead(A0);
  if (but == HIGH) {
    admin = 1;
  } else if (but == LOW) {
    admin = 0;
  }
  //checks if the admin switch is active, if it isn't it will proceed to the game

  mylcd.Set_Rotation(1); //0 is for portrait mode for the display
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(3);
  if (admin == 0) {
    if (gamemode == 0) { // I decided to put here the setup of the coalition mode because if I put it in the loop, it will of course loop this part and the game never starts
      Minutes = min;
      Seconds = sec;
      mylcd.Print("Enter password: ", 20, 110);
      while (keycount < 4) {
        char armcode = keypad.getKey();
        armcode == NO_KEY;
        if (armcode != NO_KEY) {
          if ((armcode != '*') && (armcode != '#') && (armcode != 'A') && (armcode != 'B') && (armcode != 'C') && (armcode != 'D')) {
            mylcd.Set_Text_Cousur(300 + keycount * 20, 110);
            mylcd.write(armcode);
            tone(12, 5000, 100);
            password[keycount] = armcode;
            keycount++;
          }
        }
      }

      if (keycount == 4) { //after you type your 4 digit password, the game will start
        mylcd.Fill_Screen(BLACK);
        mylcd.Print("The password is: ", 20, 110);
        mylcd.Set_Text_Cousur(320, 110);
        mylcd.write(password[0]);
        mylcd.write(password[1]);
        mylcd.write(password[2]);
        mylcd.write(password[3]);
        parola = atoi(password);
        delay(1000);
        keycount = 0;
        mylcd.Fill_Screen(BLACK);
      }
    }
  }
}

void loop() {
  if (admin == 1) { // the admin switch is on
    showtime();
    showmod(); //showtime and showmode shouldn't really be modified, basically the display doesn't refresh itself, so I have to do it manually, and this was the best option.
    adminMode();
    if (resetCount == 1) { // also this menu is kinda wacky, I am looking to improve it, first you choose the gamemode and then set the time
      showtime();
      showmod();
      delay(1500);
    }
  } else if (admin == 0) { // the admin switch is off, so the game begins
    if (gamemode == 0) {
      timer();
      coalition();
    } else if (gamemode == 1) {
      domination();
    }
  }
}

void adminMode() { //Again, I am really sorry, it looks horrible, but I will improve it.
  char armcode = keypad.getKey();
  if (gcount == 0) {
    mylcd.Set_Text_Size(2);
    mylcd.Print("1. Coalition, 2. Domination", 20, 150);
    if (armcode == '1') {
      gamemode = 0;
      mylcd.Fill_Screen(BLACK);
      mylcd.Print("Coalition", 20, 150);
      EEPROM.write(2, gamemode);
      gcount = 1;
      game = 1;
    } else if (armcode == '2') {
      gamemode = 1;
      mylcd.Fill_Screen(BLACK);
      mylcd.Print("Domination", 20, 150);
      EEPROM.write(2, gamemode);
      gcount = 1;
      game = 1;
    }
  } else if (gcount == 1) {
    if (game == 1) {
      if ((armcode != '#') && (armcode != '*')) {
        showtime();
        showmod();
        if (armcode != NO_KEY) {
          tone(12, 5000, 100);
          inputStr += armcode;
          numKey++;
          if (timeCount == 0) {
            if (numKey == 2) {
              newSec = inputStr.toInt();
              timeCount = 1;
              inputStr = "";
              numKey = 0;
            }
          } else if (timeCount == 1) {
            if (numKey == 2) {
              newMin = inputStr.toInt();
              oldMin = min;
              oldSec = sec;
              min = newMin;
              sec = newSec;
              smin = oldMin;
              ssec = oldSec;
              if (sec > 59) {
                sec = sec - 60;
                min++;
              }
              EEPROM.write(0, sec);
              EEPROM.write(1, min);
              resetCount = 1;
            }
          }
        }
        if (resetCount == 1) { // after you set the game and time the display will be updated without overlapping.
          showtime();
          showmod();
          delay(1500);
        }
      }
    }
  }
}

void showtime() { //so this shows the unmodified time
  mylcd.Set_Text_colour(BLUE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(5);
  if (smin == newMin) {
    if (min > 10) {
      mylcd.Print_Number_Int(min, 175, 25, 1, 0, 10);
    } else if (min < 10) {
      mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
      mylcd.Print_Number_Int(min, 205, 25, 1, 0, 10);
    }
  } else if (smin != newMin) {
    mylcd.Fill_Rect(175, 25, 60, 35, BLACK);
    if (min > 10) {
      mylcd.Print_Number_Int(min, 175, 25, 1, 0, 10);
    } else if (min < 10) {
      mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
      mylcd.Print_Number_Int(min, 205, 25, 1, 0, 10);
    }
    smin = newMin;
  }
  mylcd.Print(":", 225, 25);
  if (ssec == newSec) {
    if (sec > 10) {
      mylcd.Print_Number_Int(sec, 245, 25, 1, 0, 10);
    } else if (sec < 10) {
      mylcd.Print_Number_Int(0, 245, 25, 1, 0, 10);
      mylcd.Print_Number_Int(sec, 275, 25, 1, 0, 10);
    }
  } else if (ssec != newSec) {
    mylcd.Fill_Rect(245, 25, 60, 35, BLACK);
    if (sec > 10) {
      mylcd.Print_Number_Int(sec, 245, 25, 1, 0, 10);
    } else if (sec < 10) {
      mylcd.Print_Number_Int(0, 245, 25, 1, 0, 10);
      mylcd.Print_Number_Int(sec, 275, 25, 1, 0, 10);
    }
    ssec = newSec;
  }
}


void showmod() {// this shows the modified time
  mylcd.Set_Text_colour(RED);
  mylcd.Set_Text_Size(5);

  mylcd.Print(":", 225, 65);

  if (oldSec == newSec) {
    if (newSec > 10) {
      mylcd.Print_Number_Int(newSec, 245, 65, 1, 0, 10);
    } else if (newSec < 10) {
      mylcd.Print_Number_Int(0, 245, 65, 1, 0, 10);
      mylcd.Print_Number_Int(newSec, 275, 65, 1, 0, 10);
    }
  } else if (oldSec != newSec) {
    mylcd.Fill_Rect(245, 65, 60, 35, BLACK);
    if (newSec > 10) {
      mylcd.Print_Number_Int(newSec, 245, 65, 1, 0, 10);
    } else if (newSec < 10) {
      mylcd.Print_Number_Int(0, 245, 65, 1, 0, 10);
      mylcd.Print_Number_Int(newSec, 275, 65, 1, 0, 10);
    }
    oldSec = newSec;
  }
  if (oldMin == newMin) {
    if (newMin > 10) {
      mylcd.Print_Number_Int(newMin, 175, 65, 1, 0, 10);
    } else if (newMin < 10) {
      mylcd.Print_Number_Int(0, 175, 65, 1, 0, 10);
      mylcd.Print_Number_Int(newMin, 205, 65, 1, 0, 10);
    }
  } else if (oldMin != newMin) {
    mylcd.Fill_Rect(175, 65, 60, 35, BLACK);
    if (newMin > 10) {
      mylcd.Print_Number_Int(newMin, 175, 65, 1, 0, 10);
    } else if (newMin < 10) {
      mylcd.Print_Number_Int(0, 175, 65, 1, 0, 10);
      mylcd.Print_Number_Int(newMin, 205, 65, 1, 0, 10);
    }
    oldMin = newMin;
  }
}

void ran() { //here we generate the fake passwords
  fpass1 = random(1000, 9999);
  fpass2 = random(1000, 9999);
  fpass3 = random(1000, 9999);
  pos1 = random(0, 4);
A:
  pos2 = random(0, 4);
  if (pos1 == pos2) {
    pos2 = random(0, 4);
    if (pos1 == pos2) {
      goto A;
    }
  }
B:
  pos3 = random(0, 4);
  if ((pos3 == pos1) || (pos3 == pos2)) {
    pos3 = random(0, 4);
    if ((pos3 == pos1) || (pos3 == pos2)) {
      goto B;
    }
  }
C:
  pos4 = random(0, 4);
  if ((pos4 == pos1) || (pos4 == pos2) || (pos4 == pos3)) {
    pos4 = random(0, 4);
    if ((pos4 == pos1) || (pos4 == pos2) || (pos4 == pos3)) {
      goto C;
    }
  }
}

void pos() { // here you assign the positions of each password 
  mylcd.Set_Text_Size(3);
  switch (pos1) {
    case 0:
      mylcd.Set_Text_colour(WHITE);
      mylcd.Print_Number_Int(parola, 100, 80, 1, 0, 10);
      break;
    case 1:
      mylcd.Set_Text_colour(WHITE);
      mylcd.Print_Number_Int(parola, 300, 80, 1, 0, 10);
      break;
    case 2:
      mylcd.Set_Text_colour(WHITE);
      mylcd.Print_Number_Int(parola, 100, 150, 1, 0, 10);
      break;
    case 3:
      mylcd.Set_Text_colour(WHITE);
      mylcd.Print_Number_Int(parola, 300, 150, 1, 0, 10);
      break;
    default:
      break;
  }
  if (f1 != 1) {
    switch (pos2) {
      case 0:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass1, 100, 80, 1, 0, 10);
        break;
      case 1:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass1, 300, 80, 1, 0, 10);
        break;
      case 2:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass1, 100, 150, 1, 0, 10);
        break;
      case 3:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass1, 300, 150, 1, 0, 10);
        break;
      default:
        break;
    }
  }
  if (f2 != 1) {
    switch (pos3) {
      case 0:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass2, 100, 80, 1, 0, 10);
        break;
      case 1:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass2, 300, 80, 1, 0, 10);
        break;
      case 2:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass2, 100, 150, 1, 0, 10);
        break;
      case 3:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass2, 300, 150, 1, 0, 10);
        break;
      default:
        break;
    }
  }

  if (f3 != 1) {
    switch (pos4) {
      case 0:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass3, 100, 80, 1, 0, 10);
        break;
      case 1:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass3, 300, 80, 1, 0, 10);
        break;
      case 2:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass3, 100, 150, 1, 0, 10);
        break;
      case 3:
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_Number_Int(fpass3, 300, 150, 1, 0, 10);
        break;
      default:
        break;
    }
  }
}

void timer() {
  if (rantrig != 1) {// I used rantrig to run the random function only once
    ran();
    rantrig = 1;
  }
  pos();
  if ((Minutes <= 0) && (Seconds <= 0)) { 
    mylcd.Fill_Screen(BLACK);
    mylcd.Set_Text_Size(5);
    mylcd.Set_Text_Back_colour(BLACK);
    mylcd.Set_Text_colour(RED);
    mylcd.Print("Bomb exploded!", 40, 160);
    mylcd.Set_Text_Size(3);
    mylcd.Print("The password was: ", 50, 80);
    mylcd.Print_Number_Int(parola, 370, 80, 1, 0, 10);
    fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
    FastLED.show();
    delay(100);
    while ((Minutes <= 0) && (Seconds <= 0)) {
      tone(12, 7000, 100);
      delay(100);
      tone(12, 7000, 100);
      delay(100);
      tone(12, 7000, 100);
      delay(100);
      tone(12, 7000, 100);
      delay(100);
      tone(12, 7000, 100);
      delay(100);
      tone(12, 7000, 100);
      delay(100);
    }
  }
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(5);

  mylcd.Print("Time:", 20, 25); // after this will be printed the remaining time.
// i am using the variables with old and new to compare them, if they are the same, if not I delete and put over the new value, if I don't do this the numbers will overlap and we wouldn't understand anything.
  if (Minutes == 0) {
    mylcd.Set_Text_colour(RED);
  } else if (Minutes > 0) {
    mylcd.Set_Text_colour(WHITE);
  }
  if (trycount == 2) {
    mylcd.Set_Text_colour(RED);
  }

  if (oldMinutes == Minutes) {
    if (Minutes > 9) {
      mylcd.Print_Number_Int(Minutes, 175, 25, 1, 0, 10);
    } else if (Minutes < 10) {
      mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Minutes, 205, 25, 1, 0, 10);
    }
  } else if (oldMinutes != Minutes) {
    mylcd.Fill_Rect(175, 25, 60, 35, BLACK);
    if (Minutes > 9) {
      mylcd.Print_Number_Int(Minutes, 175, 25, 1, 0, 10);
    } else if (Minutes < 10) {
      mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Minutes, 205, 25, 1, 0, 10);
    }
    oldMinutes = Minutes;
  }
  mylcd.Print(":", 230, 25);
  if (oldSeconds == Seconds) {
    if (Seconds > 9) {
      mylcd.Print_Number_Int(Seconds, 255, 25, 1, 0, 10);
    } else if (Seconds < 10) {
      mylcd.Print_Number_Int(0, 255, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Seconds, 285, 25, 1, 0, 10);
    }
  } else if (oldSeconds != Seconds) {
    mylcd.Fill_Rect(255, 25, 60, 35, BLACK);
    if (Seconds > 9) {
      mylcd.Print_Number_Int(Seconds, 255, 25, 1, 0, 10);
    } else if (Seconds < 10) {
      mylcd.Print_Number_Int(0, 255, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Seconds, 285, 25, 1, 0, 10);
    }
    oldSeconds = Seconds;
  }

  if (Seconds < 1) {
    Minutes--;
    Seconds = 59;
  }

  if (Seconds > 0) {
    currentMillis = millis();// using the millis to create a delay without delaying the whole code. 
    if (currentMillis - secMillis > interval) {
      tone(12, 5000, 50);
      secMillis = currentMillis;
      Seconds--;
    }
  }
  if (i == 0) { //this is for the stages of the coalition mode, if you fail the colors change, in i == 2 the leds are faster.
    ledTimer1();
  }
  if (i == 1) {
    ledTimer2();
  }
  if (i == 2) {
    ledTimer3();
  }
  if (i == 3) {
    fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
    FastLED.show();
  }
}

void coalition() {
  keycount = 0;
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(3);
  mylcd.Print("Press # to erase", 20, 290);
  mylcd.Print("Code: ", 20, 225);
  while (keycount < 4) {
    timer();
    char disarmcode = keypad.getKey();
    if ((disarmcode == '#') || (disarmcode == '*')) {
      keycount = 0;
      inputStr = "";
      tone(12, 5000, 100);
      mylcd.Fill_Rect(120, 225, 100, 25, BLACK);
    } else if (disarmcode != NO_KEY) {
      tone(12, 5000, 100);
      mylcd.Set_Text_Size(3);
      mylcd.Set_Text_colour(GREEN);
      mylcd.Set_Text_Cousur(120 + keycount * 20, 225);
      mylcd.write(disarmcode);
      entered[keycount] = disarmcode;
      keycount++;
    }
  }

  if (keycount == 4) {
    mylcd.Fill_Rect(120, 225, 80, 35, BLACK);
    if (entered[0] == password[0] && entered[1] == password[1] && entered[2] == password[2] && entered[3] == password[3]) {
      int end = 0;
      mylcd.Fill_Screen(BLACK);
      mylcd.Set_Text_Size(5);
      mylcd.Set_Text_Back_colour(BLACK);
      mylcd.Set_Text_colour(CYAN);
      mylcd.Print("Bomb defused!", 40, 120);
      mylcd.Print("Time:", 20, 25);
      if (Minutes > 9) {
        mylcd.Print_Number_Int(Minutes, 175, 25, 1, 0, 10);
      } else if (Minutes < 10) {
        mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
        mylcd.Print_Number_Int(Minutes, 205, 25, 1, 0, 10);
      }
      mylcd.Print(":", 230, 25);
      if (Seconds > 9) {
        mylcd.Print_Number_Int(Seconds, 255, 25, 1, 0, 10);
      } else if (Seconds < 10) {
        mylcd.Print_Number_Int(0, 255, 25, 1, 0, 10);
        mylcd.Print_Number_Int(Seconds, 285, 25, 1, 0, 10);
      }
      mylcd.Set_Text_Size(4);
      mylcd.Set_Text_colour(WHITE);
      mylcd.Print("Tries:", 260, 225);
      if (trycount == 1) {
        mylcd.Set_Text_colour(RED);
        mylcd.Print("X", 400, 225);
      }
      if (trycount == 2) {
        mylcd.Set_Text_colour(RED);
        mylcd.Print("XX", 400, 225);
      }
      fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
      FastLED.show();
      delay(100);
      end = 1;
      while (end == 1) {
        tone(12, 2000, 500);
        delay(500);
        tone(12, 3000, 500);
        delay(500);
        tone(12, 2000, 500);
        delay(500);
        tone(12, 4000, 500);
        delay(500);
      }
    } else {
      trycount++;
      if (Minutes > 0) {
        Minutes = Minutes / 2;
      }
      if (Seconds > 0) {
        Seconds = Seconds / 2;
      }
      if (trycount == 1) {
        i = 1;
        try1[0] = entered[0];
        try1[1] = entered[1];
        try1[2] = entered[2];
        try1[3] = entered[3];
        mylcd.Set_Text_Size(4);
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print("Tries:", 260, 225);
        mylcd.Set_Text_colour(RED);
        mylcd.Print("X", 400, 225);
        inc1 = atoi(try1);
        if (inc1 == fpass1) {
          keycount = 0;
          f1 = 1;
          if (f1 == 1) {
            switch (pos2) {
              case 0:
                mylcd.Fill_Rect(100, 80, 100, 35, BLACK);
                break;
              case 1:
                mylcd.Fill_Rect(300, 80, 100, 35, BLACK);
                break;
              case 2:
                mylcd.Fill_Rect(100, 150, 100, 35, BLACK);
                break;
              case 3:
                mylcd.Fill_Rect(300, 150, 100, 35, BLACK);
                break;
              default:
                break;
            }
          }
        }
        if (inc1 == fpass2) {
          f2 = 1;
          if (f2 == 1) {
            switch (pos3) {
              case 0:
                mylcd.Fill_Rect(100, 80, 100, 35, BLACK);
                break;
              case 1:
                mylcd.Fill_Rect(300, 80, 100, 35, BLACK);
                break;
              case 2:
                mylcd.Fill_Rect(100, 150, 100, 35, BLACK);
                break;
              case 3:
                mylcd.Fill_Rect(300, 150, 100, 35, BLACK);
                break;
              default:
                break;
            }
          }
        }
        if (inc1 == fpass3) {
          f3 = 1;
          if (f3 == 1) {
            switch (pos4) {
              case 0:
                mylcd.Fill_Rect(100, 80, 100, 35, BLACK);
                break;
              case 1:
                mylcd.Fill_Rect(300, 80, 100, 35, BLACK);
                break;
              case 2:
                mylcd.Fill_Rect(100, 150, 100, 35, BLACK);
                break;
              case 3:
                mylcd.Fill_Rect(300, 150, 100, 35, BLACK);
                break;
              default:
                break;
            }
          }
        }
      }
      if (trycount == 2) {
        i = 2;
        keycount = 0;
        interval = interval / 5;
        try2[0] = entered[0];
        try2[1] = entered[1];
        try2[2] = entered[2];
        try2[3] = entered[3];
        mylcd.Set_Text_Size(4);
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print("Tries:", 260, 225);
        mylcd.Set_Text_colour(RED);
        mylcd.Print("XX", 400, 225);
        inc2 = atoi(try2);
        if (inc2 == fpass1) {
          f1 = 1;
          if (f1 == 1) {
            switch (pos2) {
              case 0:
                mylcd.Fill_Rect(100, 80, 100, 35, BLACK);
                break;
              case 1:
                mylcd.Fill_Rect(300, 80, 100, 35, BLACK);
                break;
              case 2:
                mylcd.Fill_Rect(100, 150, 100, 35, BLACK);
                break;
              case 3:
                mylcd.Fill_Rect(300, 150, 100, 35, BLACK);
                break;
              default:
                break;
            }
          }
        }
        if (inc2 == fpass2) {
          f2 = 1;
          if (f2 == 1) {
            switch (pos3) {
              case 0:
                mylcd.Fill_Rect(100, 80, 100, 35, BLACK);
                break;
              case 1:
                mylcd.Fill_Rect(300, 80, 100, 35, BLACK);
                break;
              case 2:
                mylcd.Fill_Rect(100, 150, 100, 35, BLACK);
                break;
              case 3:
                mylcd.Fill_Rect(300, 150, 100, 35, BLACK);
                break;
              default:
                break;
            }
          }
          if (inc2 == fpass3) {
            f3 = 1;
            if (f3 == 1) {
              switch (pos4) {
                case 0:
                  mylcd.Fill_Rect(100, 80, 100, 35, BLACK);
                  break;
                case 1:
                  mylcd.Fill_Rect(300, 80, 100, 35, BLACK);
                  break;
                case 2:
                  mylcd.Fill_Rect(100, 150, 100, 35, BLACK);
                  break;
                case 3:
                  mylcd.Fill_Rect(300, 150, 100, 35, BLACK);
                  break;
                default:
                  break;
              }
            }
          }
        }
      }
      if (trycount == 3) {
        i = 3;
        keycount = 0;
        try3[0] = entered[0];
        try3[1] = entered[1];
        try3[2] = entered[2];
        try3[3] = entered[3];
        Minutes = 0;
        Seconds = 0;
      }
    }
  }
}

void domination() {
  countdown();
  int blu = digitalRead(10);
  int red = digitalRead(11);
  if ((red == HIGH) && (blu == LOW)) {
    currentBMillis = millis();
    if (currentBMillis - bsecMillis > 1000) {
      fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
      FastLED.show();
      bsecMillis = currentBMillis;
      bsec++;
    }
    if (bsec > 59) {
      bmin++;
      bsec = 0;
    }
  }
  if ((blu == HIGH) && (red == LOW)) {
    currentRMillis = millis();
    if (currentRMillis - rsecMillis > 1000) {
      fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
      FastLED.show();
      rsecMillis = currentRMillis;
      rsec++;
    }
    if (rsec > 59) {
      rmin++;
      rsec = 0;
    }
  }
  if ((red == HIGH) && (blu == HIGH)) {
    fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
    FastLED.show();
  }

  btime = bmin * 60 + bsec;
  rtime = rmin * 60 + rsec;

  if (Minutes == 0) {
    if (Seconds == 0) {
      winner();
    }
  }
}

void countdown() {
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(5);

  if (oldMinutes == Minutes) {
    if (Minutes > 9) {
      mylcd.Print_Number_Int(Minutes, 175, 25, 1, 0, 10);
    } else if (Minutes < 10) {
      mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Minutes, 205, 25, 1, 0, 10);
    }
  } else if (oldMinutes != Minutes) {
    mylcd.Fill_Rect(175, 25, 60, 35, BLACK);
    if (Minutes > 9) {
      mylcd.Print_Number_Int(Minutes, 175, 25, 1, 0, 10);
    } else if (Minutes < 10) {
      mylcd.Print_Number_Int(0, 175, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Minutes, 205, 25, 1, 0, 10);
    }
    oldMinutes = Minutes;
  }
  mylcd.Print(":", 230, 25);
  if (oldSeconds == Seconds) {
    if (Seconds > 9) {
      mylcd.Print_Number_Int(Seconds, 255, 25, 1, 0, 10);
    } else if (Seconds < 10) {
      mylcd.Print_Number_Int(0, 255, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Seconds, 285, 25, 1, 0, 10);
    }
  } else if (oldSeconds != Seconds) {
    mylcd.Fill_Rect(255, 25, 60, 35, BLACK);
    if (Seconds > 9) {
      mylcd.Print_Number_Int(Seconds, 255, 25, 1, 0, 10);
    } else if (Seconds < 10) {
      mylcd.Print_Number_Int(0, 255, 25, 1, 0, 10);
      mylcd.Print_Number_Int(Seconds, 285, 25, 1, 0, 10);
    }
    oldSeconds = Seconds;
  }

  mylcd.Set_Text_colour(BLUE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(5);

  if (obm == bmin) {
    if (bmin > 9) {
      mylcd.Print_Number_Int(bmin, 50, 150, 1, 0, 10);
    } else if (bmin < 10) {
      mylcd.Print_Number_Int(0, 50, 150, 1, 0, 10);
      mylcd.Print_Number_Int(bmin, 80, 150, 1, 0, 10);
    }
  } else if (obm != bmin) {
    mylcd.Fill_Rect(50, 150, 60, 35, BLACK);
    if (bmin > 9) {
      mylcd.Print_Number_Int(bmin, 50, 150, 1, 0, 10);
    } else if (bmin < 10) {
      mylcd.Print_Number_Int(0, 50, 150, 1, 0, 10);
      mylcd.Print_Number_Int(bmin, 80, 150, 1, 0, 10);
    }
    obm = bmin;
  }
  mylcd.Print(":", 105, 150);
  if (obs == bsec) {
    if (bsec > 9) {
      mylcd.Print_Number_Int(bsec, 130, 150, 1, 0, 10);
    } else if (bsec < 10) {
      mylcd.Print_Number_Int(0, 130, 150, 1, 0, 10);
      mylcd.Print_Number_Int(bsec, 160, 150, 1, 0, 10);
    }
  } else if (obs != bsec) {
    mylcd.Fill_Rect(130, 150, 60, 35, BLACK);
    if (bsec > 9) {
      mylcd.Print_Number_Int(bsec, 130, 150, 1, 0, 10);
    } else if (bsec < 10) {
      mylcd.Print_Number_Int(0, 130, 150, 1, 0, 10);
      mylcd.Print_Number_Int(bsec, 160, 150, 1, 0, 10);
    }
    obs = bsec;
  }

  mylcd.Set_Text_colour(RED);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(5);

  if (orm == rmin) {
    if (rmin > 9) {
      mylcd.Print_Number_Int(rmin, 250, 150, 1, 0, 10);
    } else if (rmin < 10) {
      mylcd.Print_Number_Int(0, 250, 150, 1, 0, 10);
      mylcd.Print_Number_Int(rmin, 280, 150, 1, 0, 10);
    }
  } else if (orm != rmin) {
    mylcd.Fill_Rect(240, 150, 60, 35, BLACK);
    if (rmin > 9) {
      mylcd.Print_Number_Int(rmin, 250, 150, 1, 0, 10);
    } else if (rmin < 10) {
      mylcd.Print_Number_Int(0, 250, 150, 1, 0, 10);
      mylcd.Print_Number_Int(rmin, 280, 150, 1, 0, 10);
    }
    orm = rmin;
  }
  mylcd.Print(":", 305, 150);
  if (ors == rsec) {
    if (rsec > 9) {
      mylcd.Print_Number_Int(rsec, 330, 150, 1, 0, 10);
    } else if (rsec < 10) {
      mylcd.Print_Number_Int(0, 330, 150, 1, 0, 10);
      mylcd.Print_Number_Int(rsec, 360, 150, 1, 0, 10);
    }
  } else if (ors != rsec) {
    mylcd.Fill_Rect(330, 150, 60, 35, BLACK);
    if (rsec > 9) {
      mylcd.Print_Number_Int(rsec, 330, 150, 1, 0, 10);
    } else if (rsec < 10) {
      mylcd.Print_Number_Int(0, 330, 150, 1, 0, 10);
      mylcd.Print_Number_Int(rsec, 360, 150, 1, 0, 10);
    }
    ors = rsec;
  }

  if (Seconds < 1) {
    Minutes--;
    Seconds = 59;
  }

  if (Seconds > 0) {
    currentMillis = millis();
    if (currentMillis - secMillis > interval) {
      tone(12, 5000, 50);
      secMillis = currentMillis;
      Seconds--;
    }
  }
}

void winner() {
  mylcd.Fill_Rect(0, 25, 480, 35, BLACK);
  if (btime > rtime) {
    mylcd.Set_Text_Size(5);
    mylcd.Fill_Screen(BLUE);
    mylcd.Set_Text_colour(WHITE);
    mylcd.Set_Text_Back_colour(BLUE);
    mylcd.Print("Blue team won!", 20, 20);
    if (bmin > 9) {
      mylcd.Print_Number_Int(bmin, 150, 70, 1, 0, 10);
    } else if (bmin < 10) {
      mylcd.Print_Number_Int(0, 150, 70, 1, 0, 10);
      mylcd.Print_Number_Int(bmin, 180, 70, 1, 0, 10);
    }
    mylcd.Print(":", 205, 70);
    if (bsec > 9) {
      mylcd.Print_Number_Int(bsec, 230, 70, 1, 0, 10);
    } else if (bsec < 10) {
      mylcd.Print_Number_Int(0, 230, 70, 1, 0, 10);
      mylcd.Print_Number_Int(bsec, 260, 70, 1, 0, 10);
    }

    mylcd.Set_Text_Size(5);
    mylcd.Set_Text_colour(RED);
    mylcd.Set_Text_Back_colour(BLUE);
    if (rmin > 9) {
      mylcd.Print_Number_Int(rmin, 150, 120, 1, 0, 10);
    } else if (rmin < 10) {
      mylcd.Print_Number_Int(0, 150, 120, 1, 0, 10);
      mylcd.Print_Number_Int(rmin, 180, 120, 1, 0, 10);
    }
    mylcd.Print(":", 205, 120);
    if (rsec > 9) {
      mylcd.Print_Number_Int(rsec, 230, 120, 1, 0, 10);
    } else if (rsec < 10) {
      mylcd.Print_Number_Int(0, 230, 120, 1, 0, 10);
      mylcd.Print_Number_Int(rsec, 260, 120, 1, 0, 10);
    }
    delay(1000000);
  }

  if (btime < rtime) {
    mylcd.Set_Text_Size(5);
    mylcd.Fill_Screen(RED);
    mylcd.Set_Text_colour(WHITE);
    mylcd.Set_Text_Back_colour(RED);
    mylcd.Print("Red team won!", 20, 20);
    if (rmin > 9) {
      mylcd.Print_Number_Int(rmin, 150, 70, 1, 0, 10);
    } else if (rmin < 10) {
      mylcd.Print_Number_Int(0, 150, 70, 1, 0, 10);
      mylcd.Print_Number_Int(rmin, 180, 70, 1, 0, 10);
    }
    mylcd.Print(":", 205, 70);
    if (rsec > 9) {
      mylcd.Print_Number_Int(rsec, 230, 70, 1, 0, 10);
    } else if (rsec < 10) {
      mylcd.Print_Number_Int(0, 230, 70, 1, 0, 10);
      mylcd.Print_Number_Int(rsec, 260, 70, 1, 0, 10);
    }

    mylcd.Set_Text_Size(5);
    mylcd.Set_Text_colour(BLUE);
    mylcd.Set_Text_Back_colour(RED);
    if (bmin > 9) {
      mylcd.Print_Number_Int(bmin, 150, 120, 1, 0, 10);
    } else if (bmin < 10) {
      mylcd.Print_Number_Int(0, 150, 120, 1, 0, 10);
      mylcd.Print_Number_Int(bmin, 180, 120, 1, 0, 10);
    }
    mylcd.Print(":", 205, 120);
    if (bsec > 9) {
      mylcd.Print_Number_Int(bsec, 230, 120, 1, 0, 10);
    } else if (bsec < 10) {
      mylcd.Print_Number_Int(0, 230, 120, 1, 0, 10);
      mylcd.Print_Number_Int(bsec, 260, 120, 1, 0, 10);
    }
    delay(1000000);
  }

  if (btime == rtime) {
    mylcd.Set_Text_Size(5);
    mylcd.Fill_Screen(BLACK);
    mylcd.Set_Text_colour(WHITE);
    mylcd.Set_Text_Back_colour(BLACK);
    mylcd.Print("We got a tie", 50, 20);
    mylcd.Set_Text_colour(BLUE);
    mylcd.Set_Text_Back_colour(BLACK);
    if (bmin > 9) {
      mylcd.Print_Number_Int(bmin, 50, 150, 1, 0, 10);
    } else if (bmin < 10) {
      mylcd.Print_Number_Int(0, 50, 150, 1, 0, 10);
      mylcd.Print_Number_Int(bmin, 80, 150, 1, 0, 10);
    }
    mylcd.Print(":", 105, 150);
    if (bsec > 9) {
      mylcd.Print_Number_Int(bsec, 130, 150, 1, 0, 10);
    } else if (bsec < 10) {
      mylcd.Print_Number_Int(0, 130, 150, 1, 0, 10);
      mylcd.Print_Number_Int(bsec, 160, 150, 1, 0, 10);
    }

    mylcd.Set_Text_colour(RED);
    mylcd.Set_Text_Back_colour(BLACK);
    if (rmin > 9) {
      mylcd.Print_Number_Int(rmin, 240, 150, 1, 0, 10);
    } else if (rmin < 10) {
      mylcd.Print_Number_Int(0, 240, 150, 1, 0, 10);
      mylcd.Print_Number_Int(rmin, 270, 150, 1, 0, 10);
    }
    mylcd.Print(":", 305, 150);
    if (rsec > 9) {
      mylcd.Print_Number_Int(rsec, 330, 150, 1, 0, 10);
    } else if (rsec < 10) {
      mylcd.Print_Number_Int(0, 330, 150, 1, 0, 10);
      mylcd.Print_Number_Int(rsec, 360, 150, 1, 0, 10);
    }
    delay(1000000);
  }
}

void ledTimer1() {
  if (tracker == 0) {
    currentLedMillis = millis();
    if (ledCount < 16) {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        if (ledCount % 2 == 0) {
          leds[ledCount] = CRGB::Green;
          FastLED.show();
        } else {
          leds[ledCount] = CRGB::Purple;
          FastLED.show();
        }
        ledCount++;
      }
    } else {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        leds[ledCount2] = CRGB::Black;
        FastLED.show();
        ledCount2++;
        if (ledCount2 == 16) {
          ledCount = 15;
          ledCount2 = 15;
          tracker = 1;
        }
      }
    }
  } else if (tracker == 1) {
    currentLedMillis = millis();
    if (ledCount >= 0) {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        if (ledCount % 2 == 0) {
          leds[ledCount] = CRGB::Green;
          FastLED.show();
        } else {
          leds[ledCount] = CRGB::Purple;
          FastLED.show();
        }
        ledCount--;
      }
    } else {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        leds[ledCount2] = CRGB::Black;
        FastLED.show();
        ledCount2--;
        if (ledCount2 == 0) {
          ledCount = 0;
          ledCount2 = 0;
          tracker = 0;
        }
      }
    }
  }
}


void ledTimer2() {
  if (tracker == 0) {
    currentLedMillis = millis();
    if (ledCount < 16) {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        if (ledCount % 2 == 0) {
          leds[ledCount] = CRGB::Orange;
          FastLED.show();
        } else {
          leds[ledCount] = CRGB::Yellow;
          FastLED.show();
        }
        ledCount++;
      }
    } else {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        leds[ledCount2] = CRGB::Black;
        FastLED.show();
        ledCount2++;
        if (ledCount2 == 16) {
          ledCount = 15;
          ledCount2 = 15;
          tracker = 1;
        }
      }
    }
  } else if (tracker == 1) {
    currentLedMillis = millis();
    if (ledCount >= 0) {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        if (ledCount % 2 == 0) {
          leds[ledCount] = CRGB::Orange;
          FastLED.show();
        } else {
          leds[ledCount] = CRGB::Yellow;
          FastLED.show();
        }
        ledCount--;
      }
    } else {
      if (currentLedMillis > ledMillis) {
        ledMillis += 63;
        validate = !validate;
        leds[ledCount2] = CRGB::Black;
        FastLED.show();
        ledCount2--;
        if (ledCount2 == 0) {
          ledCount = 0;
          ledCount2 = 0;
          tracker = 0;
        }
      }
    }
  }
}

void ledTimer3() {
  if (tracker == 0) {
    currentLedMillis = millis();
    if (ledCount < 16) {
      if (currentLedMillis > ledMillis) {
        ledMillis += 31;
        validate = !validate;
        leds[ledCount] = CRGB::Red;
        FastLED.show();
        ledCount++;
      }
    } else {
      if (currentLedMillis > ledMillis) {
        ledMillis += 31;
        validate = !validate;
        leds[ledCount2] = CRGB::Black;
        FastLED.show();
        ledCount2++;
        if (ledCount2 == 16) {
          ledCount = 15;
          ledCount2 = 15;
          tracker = 1;
        }
      }
    }
  } else if (tracker == 1) {
    currentLedMillis = millis();
    if (ledCount >= 0) {
      if (currentLedMillis > ledMillis) {
        ledMillis += 31;
        validate = !validate;
        leds[ledCount] = CRGB::Red;
        FastLED.show();
        ledCount--;
      }
    } else {
      if (currentLedMillis > ledMillis) {
        ledMillis += 31;
        validate = !validate;
        leds[ledCount2] = CRGB::Black;
        FastLED.show();
        ledCount2--;
        if (ledCount2 == 0) {
          ledCount = 0;
          ledCount2 = 0;
          tracker = 0;
        }
      }
    }
  }
}