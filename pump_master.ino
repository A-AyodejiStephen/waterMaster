#include <LiquidCrystal.h>

//#include <SPI.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_PCD8544.h>

/*
 * PUMP MASTER test board v1.1
 */

// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);

const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12, bk = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define trig 19
#define echo 18
#define MAX_DURATION 235293L
#define BUTTON_LEFT    14
#define BUTTON_MIDDLE  15
#define BUTTON_RIGHT   16
#define BUZZER         2
#define PUMP           4

#define LEFT           0x01
#define OK             0x02
#define  RIGHT          0x03
#define LEFT_LONG       0x04
#define OK_LONG         0x05
#define RIGHT_LONG      0x06

#define MANUAL      0x00
#define SEMI_AUTO   0x01
#define TIMED       0x02
#define AUTO        0x03


char text[16];
uint16_t distance, level, referenceDistance = 1400;
uint8_t mode = SEMI_AUTO;
bool pumpStatus = false, hold = false;

//===================================================================================================================================
uint8_t buttonScan()
{
  static uint32_t button_Timer;
  
//  if (BUTTON_LEFT == 0)
//  {
//    button_Timer = millis() + 3000;
//    delay(100);
//    while (BUTTON_LEFT == 0)
//    {
//      if (millis() > button_Timer)
//      {
//        if (hold == false)
//        {
//          hold = true;
//          return LEFT_LONG;
//        }else 
//        {
//          return 0x00;
//        }
//        
//      }
//    }
//    hold = false;
//    return  LEFT;
//  }else if (BUTTON_MIDDLE == 0)
//  {
//    button_Timer = millis() + 3000;
//    delay(100);
//    while (BUTTON_MIDDLE == 0)
//    {
//      if (millis() > button_Timer)
//      {
//        if (hold == false)
//        {
//          hold = true;
//          return OK_LONG;
//        }else
//        {
//          return 0x00;
//        }
//        
//      }
//    }
//    hold = false;
//    return OK;
//  }else if (BUTTON_RIGHT == 0)
//  {
//    button_Timer = millis() + 3000;
//    delay(100);
//    while (BUTTON_RIGHT == 0)
//    {
//      if (millis() > button_Timer)
//      {
//        if (hold == false)
//        {
//          hold = true;
//          return RIGHT_LONG;
//        }else
//        {
//          return 0x00;
//        }
//        
//      }
//    }
//    hold = false;
//    return RIGHT;
//  }

  if (digitalRead(BUTTON_LEFT) == LOW)
  {
    delay(100);
    if (digitalRead(BUTTON_LEFT) == LOW)
    {
      return LEFT;
    }
  }else if (digitalRead(BUTTON_MIDDLE) == LOW)
  {
    delay(100);
    if (digitalRead(BUTTON_MIDDLE) == LOW)
    {
      return OK;
    }
  }else if (digitalRead(BUTTON_RIGHT) == LOW)
  {
    delay(100);
    if (digitalRead(BUTTON_RIGHT) == LOW)
    {
      return RIGHT;
    }
  }else
  {
    return 0;
  }
}
//--------------------------------------------------------------------------------------------------

uint16_t getDistance()
{
  uint32_t duration = 0, distance = 0;
  uint16_t i = 0;
  
  while (i < 2)
  {
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig,LOW);
    duration = pulseIn(echo, HIGH);
    
    if (duration > MAX_DURATION)
    {
      distance = 0xffff;
      i = 1;
      break;
    }else
    {
      distance += (duration * 0.34) / 2;
    }
    i++;
    delay(60);
  }

  return ((distance / i) & 0xffff);
}
//--------------------------------------------------------------------------------------------------
void configurationMode()
{
  
  
}
//--------------------------------------------------------------------------------------------------
void getWaterLevel()
{
  static uint32_t getWaterLevel_Timer = 0;
  static uint8_t samples = 0;
  static uint32_t sum = 0;
  
  if (getWaterLevel_Timer < millis())
  {
    
    if (samples == 16) 
    {
      getWaterLevel_Timer = millis() + 5000;
      distance = sum / samples;
      if (distance > 100)
      {
        level = 100 - (((distance - 100) * 100) / referenceDistance);
      }else
      {
        level = 100;
      }
      
      samples = 0; sum = 0;
      return;
    }

    sum += getDistance();
    samples++;
    
  }
}
//--------------------------------------------------------------------------------------------------
void updateLCD()
{
  lcd.setCursor(5,0);
  for (int i = 0; i < 10; i++)
  {
    if (level > (i*10))
    {
      lcd.write(0xFF);
    }else
    {
      lcd.print(" ");
    }
  }

  lcd.setCursor(6,1); lcd.print(level); lcd.print("%   ");
  lcd.setCursor(5,2);

  if (mode == AUTO)
  {
    lcd.print("AUTO      ");
  }else if (mode == SEMI_AUTO)
  {
    lcd.print("SEMI-AUTO ");
  }else if (mode == TIMED)
  {
    lcd.print("TIMED     ");
  }else if (mode == MANUAL)
  {
    lcd.print("MANUAL    ");
  }

  lcd.setCursor(7,3);
  if (pumpStatus) lcd.print("ON ");
  else lcd.print("OFF");

  lcd.setCursor(12,3);lcd.print(distance);
  lcd.print("   ");
  
}
//--------------------------------------------------------------------------------------------
void modeSet()
{
  uint8_t key = 0;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Mode Set");
  lcd.setCursor(0,1); lcd.print("Mode:");

  while (key != OK)
  {
    lcd.setCursor(6,1);
    if (mode == AUTO)
    {
      lcd.print("<AUTO>     ");
    }else if (mode == SEMI_AUTO)
    {
      lcd.print("<SEMI-AUTO>");
    }else if (mode == TIMED)
    {
      lcd.print("<TIMED>    ");
    }else if (mode == MANUAL)
    {
      lcd.print("<MANUAL>   ");
    }

    key = buttonScan();

    if (key == RIGHT)
    {
      if (mode < 0x03) mode++;
    }else if (key == LEFT)
    {
      if (mode > 0x00) mode--;
    }

    delay(500);
    
  }

  homeScreen();
}
//--------------------------------------------------------------------------------------------
void homeScreen()
{
//  uint8_t xPMargin = 10, yPMargin = 3;
//  display.clearDisplay();
//
//  //Tank Icon Outline
//  display.drawRoundRect(xPMargin,yPMargin+4,30,38,2, 0xffff);
//  display.fillRect(xPMargin+11, yPMargin, 8, 4, 0xffff);
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("    [          ]    ");
  lcd.setCursor(0,1); lcd.print("Level:");
  lcd.setCursor(0,2); lcd.print("Mode:");
  lcd.setCursor(0,3); lcd.print("Status:");
}

//##################################################################################################################
void setup() 
{

//  display.begin();
//  display.setContrast(50);
//  display.display(); // show splashscreen
//  delay(2000);
//  display.clearDisplay();   // clears the screen and buffer

  lcd.begin(20,4);
  
  pinMode(trig, OUTPUT);
  digitalWrite(trig,LOW);
  pinMode(echo,INPUT);
  pinMode(BUTTON_LEFT,INPUT_PULLUP);
  pinMode(BUTTON_MIDDLE,INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  delay(100);
  lcd.print("Starting...");
  delay(3000);
  homeScreen();
}
//----------------------------------------------------------------------------------------------------------------------------
void loop() 
{
  uint8_t key = 0;
  
  getWaterLevel();
  updateLCD();
  key = buttonScan();

  if (key == OK)
  {
    pumpStatus = !(pumpStatus);
    digitalWrite(PUMP,pumpStatus);
  }else if (key == RIGHT)
  {
    modeSet();
  }
  
  key = 0;
 

}
