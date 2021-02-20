#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "C:\Users\ayode\Documents\Arduino\libraries\Adafruit_GFX_Library\Fonts\DSEG14_Classic_Italic_7Bitmaps.h"
#include "EEPROM.h"

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define trig 9
#define echo 10
#define MAX_DURATION 235293L

#define PUMPCTRL  8  

#define AUTO    1
#define S_AUTO  0
#define TIMER   2

#define UP  0x01
#define OK  0x02
#define DOWN 0x03


#define TT_ADD 0x0
#define TH_ADD1 0x1
#define TH_ADD2 0x2
#define TD_ADD1 0x3
#define TD_ADD2 0x4
#define CM_ADD 0x5
#define OV_ADD1 0x6
#define OV_ADD2 0x7
#define UV_ADD1 0x8
#define UV_ADD2 0x9
#define WF_ADD 0xA
#define WL_ADD 0xB
#define H_ADD  0xC
#define M_ADD  0xD
#define SYS_ADD 0xE



#define VERTICAL  0
#define HORIZONTAL 1

#define OVERVOLTAGE_L   201
#define OVERVOLTAGE_U   280
#define UNDERVOLTAGE_L  140
#define UNDERVOLTAGE_U  200
#define WATERFULL_L     50
#define WATERFULL_U     99
#define WATERLOW_L      1
#define WATERLOW_U      70
#define TANKHEIGHT_U    390
#define TANKHEIGHT_L    50
#define TANKDIAMETER_U    390
#define TANKDIAMETER_L    50


uint16_t tankHeight = 140, tankDiameter = 140;
uint16_t volume = 920, capacity = PI * pow((tankDiameter / 20),2) * ((tankHeight - 10 ) / 10);
uint8_t tankType = VERTICAL;
uint8_t waterFull = 99, waterLow = 50, percentage;

bool isPumpON = false, alarm = true, isOvervoltage = false, isUndervoltage = false, isWaterFull = false, isWaterLow = false, isTimeup = false;
int16_t voltage = 221, overvoltage = 250, undervoltage = 170;

uint32_t displayTimer;
uint8_t hour = 0, minute = 0, seconds = 0;
uint8_t hourSet = 0, minuteSet = 30;

uint8_t controlMode = S_AUTO, _page = 2, systemFlags = 0x1;


//############################################################################################
void saveSettings(uint8_t address)
{
  switch(address)
  {
    case TT_ADD:  EEPROM.write(TT_ADD, tankType);
                  break;
    case TH_ADD1: EEPROM.write(TH_ADD1, (tankHeight & 0xFF));
                  EEPROM.write(TH_ADD2, (tankHeight >> 8));
                  break;
    case TD_ADD1: EEPROM.write(TD_ADD1, (tankDiameter & 0xFF));
                  EEPROM.write(TD_ADD2, (tankDiameter >> 8));
                  break;
    case CM_ADD:  EEPROM.write(CM_ADD, controlMode);
                  break;
    case OV_ADD1: EEPROM.write(OV_ADD1, (overvoltage & 0xFF));
                  EEPROM.write(OV_ADD2, (overvoltage  >> 8));
                  break;
    case UV_ADD1: EEPROM.write(UV_ADD1, (undervoltage & 0xFF));
                  EEPROM.write(UV_ADD2, (undervoltage  >> 8));
                  break;
    case WF_ADD:  EEPROM.write(WF_ADD, waterFull);
                  break;
    case WL_ADD:  EEPROM.write(WL_ADD, waterLow);
                  break;
    case H_ADD:   EEPROM.write(H_ADD, hourSet);
                  break;
    case M_ADD:   EEPROM.write(M_ADD, minuteSet);
                  break;
    case SYS_ADD: EEPROM.write(SYS_ADD, systemFlags);
                  break;
    default:      EEPROM.write(TT_ADD, tankType);
                  EEPROM.write(TH_ADD1, (tankHeight & 0xFF));
                  EEPROM.write(TH_ADD2, (tankHeight >> 8));
                  EEPROM.write(TD_ADD1, (tankDiameter & 0xFF));
                  EEPROM.write(TD_ADD2, (tankDiameter >> 8));
                  EEPROM.write(CM_ADD, controlMode);
                  EEPROM.write(OV_ADD1, (overvoltage & 0xFF));
                  EEPROM.write(OV_ADD2, (overvoltage  >> 8));
                  EEPROM.write(UV_ADD1, (undervoltage & 0xFF));
                  EEPROM.write(UV_ADD2, (undervoltage  >> 8));
                  EEPROM.write(WF_ADD, waterFull);
                  EEPROM.write(WL_ADD, waterLow);
                  EEPROM.write(H_ADD, hourSet);
                  EEPROM.write(M_ADD, minuteSet);
                  EEPROM.write(SYS_ADD, systemFlags);
                  break;
  }
}
//-------------------------------------------------------------------------------------------
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
      distance += (duration * 0.034) / 2;
    }
    i++;
    delay(60);
  }

  return ((distance / i) & 0xffff);
}
//-------------------------------------------------------------------------------------------
uint8_t getTextWidth(char text[])
{
  return (strlen(text)*5) + strlen(text) - 1;
}
//-------------------------------------------------------------------------------------------
void printTextCenter(char text[], uint8_t xStart, uint8_t xStop, uint8_t y)
{
  uint8_t textx =xStart + ( ((xStop - xStart) - getTextWidth(text)) / 2 );
  display.setCursor(textx,y);
  display.print(text);
}
//-------------------------------------------------------------------------------------------
void printTextCenter(char text[], uint8_t xStart, uint8_t xStop, uint8_t yStart, uint8_t yStop)
{
  uint8_t textx = xStart + ( ((xStop - xStart) - getTextWidth(text)) / 2 );
  uint8_t texty = yStart + ( ((yStop - yStart) - 7) / 2);
  display.setCursor(textx,texty);
  display.print(text);
}
//-------------------------------------------------------------------------------------------
void printTextRight(char text[], uint8_t xStart, uint8_t xStop, uint8_t y)
{
  uint8_t textx = xStop - xStart - getTextWidth(text) - 1;
  display.setCursor(textx,y);
  display.print(text);
}
//-------------------------------------------------------------------------------------------
void printTextLeft(char text[], uint8_t xStart, uint8_t y)
{
  display.setCursor(xStart,y);
  display.print(text);
}
//-------------------------------------------------------------------------------------------
void drawTextBox(uint8_t x, uint8_t y, uint8_t w, uint16_t color)
{
  display.drawRoundRect(x,y,w,11,2,color);
  display.display();
}
//-------------------------------------------------------------------------------------------
void highlightTextBox(uint8_t x, uint8_t y, uint8_t w, uint16_t color)
{
  display.fillRoundRect(x,y,w,11,2,color);
  display.display();
}
//-------------------------------------------------------------------------------------------
void startUpScreen()
{
  display.clearDisplay(); 
  
  char text[13] = "WATER-MASTER";
  uint8_t txty = 20;
  uint8_t x = 41,y = txty+9, w, h;
  
  display.setTextSize(1);
//  display.setFont(&FreeMono9pt7b);
  display.setTextColor(BLACK);

  printTextCenter(text,0,84,0,48);
  display.display();

  display.setCursor(x,y);
  uint8_t barLength = (getTextWidth(text) / 2) + 2;
  for (int i = 0; i < barLength; i++)
  {
    display.drawPixel(x-i,y, BLACK);
    display.drawPixel(x+i,y, BLACK);
    display.drawPixel(x-i,y-11, BLACK);
    display.drawPixel(x+i,y-11, BLACK);
    display.display();
    delay(50);
  }

  for (int i = 0; i < 12; i++)
  {
    display.drawPixel(x-barLength,y-11+i, BLACK);
    display.drawPixel(x+barLength,y-i, BLACK);
    display.display();
    delay(50);
  }

  delay(2000);
}
//-------------------------------------------------------------------------------------------
void drawTank(uint8_t tankx, uint8_t tanky, uint8_t tankWidth, uint8_t tankHeight, uint8_t percentage)
{
  //Tank outline

  uint8_t tCoverx = tankx+(tankWidth/4)+1;
  uint8_t tCovery = tanky-3;
  uint8_t tCoverWidth = tankWidth/2;
  uint8_t tCoverHeight = 4;
//  display.drawRoundRect(tankx,tanky,tankWidth,tankHeight,3,BLACK);
//  display.fillRoundRect(tCoverx,tCovery, tCoverWidth, tCoverHeight,2,BLACK);
//  display.display();

  display.drawRoundRect(tankx, tanky,tankWidth, tankHeight/5,3,BLACK);
  for (int i = 1; i < 5; i++)
  {
    display.drawRoundRect(tankx, tanky+(i*((tankHeight/5)-1)),tankWidth, tankHeight/5,2,BLACK);
  }
//  display.drawRoundRect(tankx, tanky+28,tankWidth, tankHeight/5,3,BLACK);
  display.fillRoundRect(tCoverx,tCovery, tCoverWidth, tCoverHeight,2,BLACK);
  display.fillRect(tankx+3, tanky+2, tankWidth-6, tankHeight-8 , WHITE);

  for (int i = 4; i > 0; i--)
  {
    if (percentage > ((4-i)*20 ))
    {
      display.fillRoundRect(tankx, tanky+(i*((tankHeight/5)-1)),tankWidth, tankHeight/5,2,BLACK);
    }
  }
  
}
//-------------------------------------------------------------------------------------------
void drawPump(uint8_t pumpx, uint8_t pumpy, uint8_t pumpWidth, uint8_t pumpHeight)
{
  uint8_t outletWidth = 3;
  display.drawRoundRect(pumpx,pumpy,pumpWidth,pumpHeight,3,BLACK);
  display.fillRoundRect(pumpx+4,pumpy-2,4,pumpHeight+4,2,BLACK);
  display.fillRoundRect(pumpx+pumpWidth-8,pumpy-2,4,pumpHeight+4,2,BLACK);
  for (int i = 3; i < 20; i+=4)
  {
    display.drawFastHLine(pumpx+10,pumpy+i, 10, BLACK);
  }
  display.fillRoundRect(pumpx+pumpWidth-1, pumpy+pumpHeight/4,outletWidth,pumpHeight/2,1,BLACK);
  display.drawFastHLine(pumpx+pumpWidth+outletWidth-1,pumpy-2+pumpHeight/2,5,BLACK);
  display.drawFastHLine(pumpx+pumpWidth+outletWidth-1,pumpy+2+pumpHeight/2,5,BLACK);

  display.drawFastVLine(pumpx+10, pumpy+pumpHeight-1, 5, BLACK);
  display.drawFastVLine(pumpx+20, pumpy+pumpHeight-1, 5, BLACK);
}
//-------------------------------------------------------------------------------------------
void page(uint8_t _page)
{
  switch(_page)
  {
    case 0: page1();
            break;
    case 1: page2();
            break;
    case 2: page3();
            break;
    default: break;
  }
}
//-------------------------------------------------------------------------------------------
void page1()
{
  display.clearDisplay();

  uint8_t tankW = 30;
  uint8_t tankH = 35;
  uint8_t tankx = 5;
  uint8_t tanky = 15;  
  
  drawTank(tankx,tanky,tankW, tankH, percentage);
  //Demarcation
  display.drawLine(42, 0, 42, 48, BLACK);
  

  //Info Texts
//  display.setFont(&DSEG14_Classic_Italic_7);
  display.setTextSize(0);
  display.fillRect(0,0,84,11,BLACK);
  display.setTextColor(WHITE);
  printTextCenter("TANK",0,84,2);
  static char buff[8];
  sprintf(buff, "%i%%", percentage);
  display.drawRect(42,11,42,19,BLACK);
  display.setTextColor(BLACK);
  printTextCenter(buff,42,83,11,30);

  sprintf(buff, "%iL", volume);
  display.fillRect(42,30,42,19,BLACK);
  display.setTextColor(WHITE);
  printTextCenter(buff,42,83,30,47);
  display.setTextColor(BLACK);
  display.display();
  
}
//-------------------------------------------------------------------------------------------
void page2()
{
  display.clearDisplay();

  //PUMP ICON
  drawPump(2,18, 30, 20);

  //Demarcation
  display.drawLine(42, 0, 42, 48, BLACK);

   //Info Texts
//  display.setTextSize(0);
//  display.setCursor(53,5);
//  display.print("PUMP");
  display.fillRect(0,0,84,11,BLACK);
  display.setTextColor(WHITE);
  printTextCenter("PUMP", 0,84,2);
  display.drawRect(42,11,42,19,BLACK);
  display.setTextColor(BLACK);
  if (isPumpON)
  {
    printTextCenter("ON",42,83,11,30);
  }else
  {
    printTextCenter("OFF",42,83,11,30);
  }
  
  static char buff[6];
  sprintf(buff, "%iV", voltage);
  display.fillRect(42,30,42,19,BLACK);
  display.setTextColor(WHITE);
  printTextCenter(buff,42,83,30,47);
  display.setTextColor(BLACK);
  display.display();
  
}

//-------------------------------------------------------------------------------------------
void page3()
{
  display.clearDisplay();

  uint8_t circlex = 20;
  uint8_t circley = 27;
  uint8_t circler = 15;
  uint8_t border = 2;
  //INFO ICON
  display.fillCircle(circlex,circley,circler,BLACK);
  display.fillCircle(circlex,circley,circler-border,WHITE);
  display.fillRect(circlex-2,circley-circler+2+border, 4,(circler*2)-(border*2)-4, BLACK);
  display. fillRect(circlex-2,circley+circler-border-2-8, 4, 4, WHITE);

  //Demarcation
  display.drawLine(42, 0, 42, 48, BLACK);

  char buff[8];
  //Info Texts
  display.setTextSize(0);
  display.fillRect(0,0,84,11,BLACK);
  display.setTextColor(WHITE);
  printTextCenter("SYSTEM",0,84,2);
  display.drawRect(42,11,42,19,BLACK);
  display.setTextColor(BLACK);
  if (controlMode == AUTO)
  {
    printTextCenter("AUTO",42,83,11,30);
  }else if (controlMode == S_AUTO)
  {
    printTextCenter("S-AUTO",42,83,11,30);
  }else if (controlMode == TIMER)
  {
    printTextCenter("TIMER",42,83,11,30);
  }
  sprintf(buff,"%i:%i",hour,minute);
  display.fillRect(42,30,42,19,BLACK);
  display.setTextColor(WHITE);
  printTextCenter(buff,42,83,30,47);
  display.setTextColor(BLACK);
  display.display();
  
}
//-------------------------------------------------------------------------------------------
void waterVolume()
{
  static uint32_t waterVolumeTimer = millis() + 1500;
  

  if (millis() > waterVolumeTimer)
  {
    waterVolumeTimer = millis() + 1500;

    uint16_t distance = getDistance();

    percentage = 100 - (((distance - 10) * 100) / tankHeight);

    if (distance >= tankHeight)
    {
      volume = capacity - 100;
    }else
    {
      volume = 3.142 * pow((tankDiameter / 20),2) * ((tankHeight - 10 - distance) / 10) ;
    }
  }
  

}
//-------------------------------------------------------------------------------------------
void getVoltage()
{
  static uint32_t getVoltageTimer = millis() + 1000;
  uint32_t accum = 0;
  uint16_t temp;
  if (millis() > getVoltageTimer)
  {
    getVoltageTimer = millis() + 1000;

    for (int i = 0; i < 32; i++)
    {
      accum += analogRead(A3) * 4.88;
      delay(1);
    }
    voltage = ((accum / 32) * 87) / 1000;
    if (isPumpON) voltage +=28;
  }
}
//-------------------------------------------------------------------------------------------
uint8_t getButton()
{
#define BUTTON_DELAY  50
  uint8_t  result = 0x0;
  if (digitalRead(14) == LOW)
  {
    delay(BUTTON_DELAY);
    if (digitalRead(14) == LOW)
    {
      result = DOWN;
      while(digitalRead(14) == LOW) ;
    }
  }else if (digitalRead(15) == LOW)
  {
    delay(BUTTON_DELAY);
    if (digitalRead(15) == LOW)
    {
      result = OK;
      while(digitalRead(15) == LOW) ;
    }
  }else if (digitalRead(16) == LOW)
  {
    delay(BUTTON_DELAY);
    if (digitalRead(16) == LOW)
    {
      result = UP;
      while(digitalRead(16) == LOW) ;
    }
  }

  return result;
}
//-------------------------------------------------------------------------------------------
void typeForm(uint8_t tType, bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,10,42,11,WHITE);
    display.drawRect(42,10,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,10,42,11,BLACK);
    display.setTextColor(WHITE);
    display.drawFastHLine(5,47,30, WHITE);
    display.drawFastVLine(38,15,31, WHITE);
  }
  char text[7];
  sprintf(text,"TYPE:%i", tType);
  printTextLeft(text, 44, 12);
}
//-------------------------------------------------------------------------------------------
void heightForm(uint8_t height, bool highlight)
{
 
  
  if (!highlight)
  {
    display.fillRect(42,20,42,11,WHITE);
    display.drawRect(42,20,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,20,42,11,BLACK);
    display.setTextColor(WHITE);
    display.drawFastHLine(5,47,30, WHITE);
    display.drawFastVLine(38,15,31, BLACK);
  }
  char text[7];
  sprintf(text,"H:%i", height);
  printTextLeft(text, 44, 22);
}
//-------------------------------------------------------------------------------------------
void diameterForm(uint8_t diameter, bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,30,42,11,WHITE);
    display.drawRect(42,30,42,11,BLACK);
    display.setTextColor(BLACK);   
  }else
  {
    display.fillRect(42,30,42,11,BLACK);
    display.setTextColor(WHITE);
    display.drawFastVLine(38,15,31, WHITE);
    display.drawFastHLine(5,47,30, BLACK);
  }
  char text[7];
  sprintf(text,"D:%i", diameter);
  printTextLeft(text, 44, 32);
}
//-------------------------------------------------------------------------------------------
void modeForm(uint8_t mode, bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,10,42,11,WHITE);
    display.drawRect(42,10,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,10,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  if (mode == AUTO) sprintf(text, "AUTO");
  else if (mode == S_AUTO) sprintf(text, "S-AUTO");
  else if (mode == TIMER) sprintf(text, "TIMER");
  printTextLeft(text, 44, 12);
}
//-------------------------------------------------------------------------------------------
void waterFullForm(uint8_t level, bool highlight)
{
 
  
  if (!highlight)
  {
    display.fillRect(42,20,42,11,WHITE);
    display.drawRect(42,20,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,20,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  sprintf(text,"WF:%i", level);
  printTextLeft(text, 44, 22);
}
//-------------------------------------------------------------------------------------------
void waterLowForm(uint8_t level, bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,30,42,11,WHITE);
    display.drawRect(42,30,42,11,BLACK);
    display.setTextColor(BLACK);   
  }else
  {
    display.fillRect(42,30,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  sprintf(text,"WL:%i", level);
  printTextLeft(text, 44, 32);
}
//-------------------------------------------------------------------------------------------
void overvoltageForm(int16_t _voltage, bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,40,42,11,WHITE);
    display.drawRect(42,40,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,40,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  sprintf(text,"OV:%i", _voltage);
  printTextLeft(text, 44, 42);
}
//-------------------------------------------------------------------------------------------
void undervoltageForm(uint8_t _voltage, bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,10,42,11,WHITE);
    display.drawRect(42,10,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,10,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  sprintf(text,"UV:%i", _voltage);
  printTextLeft(text, 44, 12);
}
//-------------------------------------------------------------------------------------------
void alertForm(bool alert, bool highlight)
{
 
  
  if (!highlight)
  {
    display.fillRect(42,20,42,11,WHITE);
    display.drawRect(42,20,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,20,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  if (alert) sprintf(text,"AL:ON");
  else sprintf(text,"AL:OFF");
  printTextLeft(text, 44, 22);
}
//-------------------------------------------------------------------------------------------
void timerForm(bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,30,42,11,WHITE);
    display.drawRect(42,30,42,11,BLACK);
    display.setTextColor(BLACK);   
  }else
  {
    display.fillRect(42,30,42,11,BLACK);
    display.setTextColor(WHITE);
  }
  char text[7];
  sprintf(text,"%i:%i", hour, minute);
  printTextCenter(text, 44, 84, 32);
}
//-------------------------------------------------------------------------------------------
void saveForm(bool highlight)
{
 
  if (!highlight)
  {
    display.fillRect(42,40,42,11,WHITE);
    display.drawRect(42,40,42,11,BLACK);
    display.setTextColor(BLACK);
  }else
  {
    display.fillRect(42,40,42,11,BLACK);
    display.setTextColor(WHITE);
    display.drawFastHLine(5,47,30, WHITE);
    display.drawFastVLine(38,15,31, WHITE);
  }
  printTextCenter("SAVE", 44,84, 41);
}
//-------------------------------------------------------------------------------------------
void switchActive(uint8_t _active, bool isTank)
{
  if (isTank)
  {
    typeForm(1,false);
    heightForm(tankHeight, false);
    diameterForm(tankDiameter, false);
    saveForm(false);
    switch(_active)
    {
      case 0: typeForm(1,true);
              break;
      case 1: heightForm(tankHeight, true);
              break;
      case 2: diameterForm(tankDiameter, true);
              break;
      case 3: saveForm(true);
              break;
      default:
              break;
    }
  }else
  {
    if ((_active < 4) || (_active == 0xf))
    {
      modeForm(controlMode,false);
      waterFullForm(waterFull, false);
      waterLowForm(waterLow, false);
      overvoltageForm(overvoltage, false);
    }else{
      undervoltageForm(undervoltage,false);
      alertForm(alarm, false);
      display.fillRect(42,30,42,11,WHITE);
      display.drawRect(42,30,42,11,BLACK);
      saveForm(false);
    }
    switch(_active)
    {
      case 0: modeForm(controlMode,true);
              break;
      case 1: waterFullForm(waterFull, true);
              break;
      case 2: waterLowForm(waterLow, true);
              break;
      case 3: overvoltageForm(overvoltage, true);
              break;
      case 4: undervoltageForm(undervoltage,true);
              break;
      case 5: alertForm(alarm, true);
              break;
      case 6: saveForm(true);
              break;
      default: 
              break;
    }
    
  }
  
    display.display(); 
}
//-------------------------------------------------------------------------------------------
void setActiveParameter(uint8_t _active, bool isTank)
{
  uint32_t blinkTimer = millis() + 1000;
  bool toggle = false;
  uint8_t button = getButton();
  const bool tank = true, pump = false; 

  if (isTank)
  {
    while (button != OK)
    {
      button = getButton();
      if (millis() > blinkTimer)
      {
        blinkTimer = millis() + 500;
        if (toggle)
        {
          switchActive(0xf, tank);
          toggle = !toggle;
        }else
        {
          switchActive(_active, tank);
          toggle = !toggle;
        }
        
      }
      if (button == UP)
      {
        switch (_active)
        {
        case 0: if (tankType < 1) tankType++;
                break;
        case 1: if (tankHeight < 400) tankHeight+=5;
                break;
        case 2: if (tankDiameter < 400) tankDiameter+=5;
                break;
        default:
          break;
        }
        // switchActive(_active, tank);
      }else if (button == DOWN)
      {
        switch (_active)
        {
        case 0: if (tankType > 0) tankType--;
                break;
        case 1: if (tankHeight > 50) tankHeight-=5;
                break;
        case 2: if (tankDiameter > 50) tankDiameter-=5;
                break;
        default:
          break;
        }
        // switchActive(_active, tank);
      }
    }
    switchActive(_active, tank);
  }else
  {
    while (button != OK)
    {
      button = getButton();
      if (millis() > blinkTimer)
      {
        blinkTimer = millis() + 500;
        if (toggle)
        {
          if (_active < 4) switchActive(0xf, pump);
          else switchActive(0xe, pump);
          toggle = !toggle;
        }else
        {
          switchActive(_active, pump);
          toggle = !toggle;
        }
        
      }
      if (button == UP)
      {
        switch (_active)
        {
          case 0: if (controlMode < TIMER) controlMode++;
                  break;
          case 1: if (waterFull < WATERFULL_U) waterFull++;
                  if ((waterFull - waterLow) < 10 ) waterLow -= 10 - (waterFull - waterLow);  //Setting rule
                  break;
          case 2: if (waterLow < WATERLOW_U) waterLow++;
                  if ((waterFull - waterLow) < 10 ) waterLow -= 10 - (waterFull - waterLow);  //Setting rule
                  break;
          case 3: if (overvoltage < OVERVOLTAGE_U) overvoltage++;
                  break;
          case 4: if (undervoltage < UNDERVOLTAGE_U) undervoltage++;
                  break;
          case 5: alarm = !alarm;
                  break;
          default:
                  break;
        }
        // switchActive(_active, pump);
      }else if (button == DOWN)
      {
        switch (_active)
        {
          case 0: if (controlMode > S_AUTO) controlMode--;
                  break;
          case 1: if (waterFull > WATERFULL_L) waterFull--;
                  if ((waterFull - waterLow) < 10 ) waterLow -= 10 - (waterFull - waterLow);  //Setting rule
                  break;
          case 2: if (waterLow > WATERLOW_L) waterLow--;
                  if ((waterFull - waterLow) < 10 ) waterLow -= 10 - (waterFull - waterLow);  //Setting rule
                  break;
          case 3: if (overvoltage > OVERVOLTAGE_L) overvoltage--;  // overvoltage = (overvoltage > OVERVOLTAGE_L) ? overvoltage-- : overvoltage;
                  break;
          case 4: if (undervoltage > UNDERVOLTAGE_L) undervoltage--;
                  break;
          case 5: alarm = !alarm;
                  break;
          default:
                  break;
        }
        // switchActive(_active, pump);
      }
    }
    switchActive(_active, pump);
  }
  
  
}
//-------------------------------------------------------------------------------------------
void tankSettings()
{
  bool isSaved = false;
  uint8_t active = 0;
  const bool tank = true, pump = false;
  display.clearDisplay();
  drawTank(5,15,30, 35, 0);

  //Info Texts
  display.setTextSize(0);
  display.fillRect(0,0,84,11,BLACK);
  display.setTextColor(WHITE);
  printTextCenter("TANK SETTINGS",0,84,2);
  
  typeForm(1,true);
  heightForm(tankHeight, false);
  diameterForm(tankDiameter, false);
  saveForm(false);
  display.display(); 
  uint8_t button = getButton();
  while(isSaved == false)
  {
    button = getButton();

    if (button == DOWN)
    {
      if (active < 3) active++;
      switchActive(active,tank);
    }else if (button == UP)
    {
      if (active > 0) active--;
      switchActive(active,tank);
    }else if (button == OK)
    {
      if (active == 3) isSaved = true;
      else setActiveParameter(active,tank);
    }

  }
  saveSettings(TT_ADD);
  saveSettings(TH_ADD1);
  saveSettings(TD_ADD1);
   
}
//-------------------------------------------------------------------------------------------
void pumpSettings()
{
  bool isSaved = false;
  uint8_t active = 0;
  const bool tank = true, pump = false;
  display.clearDisplay();
  drawPump(2,18, 30, 20);

  //Info Texts
  display.setTextSize(0);
  display.fillRect(0,0,84,11,BLACK);
  display.setTextColor(WHITE);
  printTextCenter("PUMP SETTINGS",0,84,2);
  
  modeForm(controlMode,true);
  waterFullForm(waterFull, false);
  waterLowForm(waterLow, false);
  overvoltageForm(overvoltage, false);
  display.display(); 
  uint8_t button = getButton();
  while(isSaved == false)
  {
    button = getButton();

    if (button == DOWN)
    {
      if (active < 6 )active++;
      switchActive(active, pump);
    }else if (button == UP)
    {
      if (active > 0) active--;
      switchActive(active, pump);
    }else if (button == OK)
    {
      if (active == 6) isSaved = true;
      else setActiveParameter(active, pump);
    }

  }
  saveSettings(CM_ADD);
  saveSettings(OV_ADD1);
  saveSettings(UV_ADD1);
  saveSettings(WF_ADD);
  saveSettings(WL_ADD);
  saveSettings(H_ADD);
  saveSettings(M_ADD);
  saveSettings(SYS_ADD);
}
//-------------------------------------------------------------------------------------------
void timerUpdate()
{
  static uint32_t counter = millis() + 1000;

  if (millis() > counter)
  {
    counter = millis() + 1000;

    if (isPumpON && (controlMode == TIMER))
    {
      seconds++;
      if (seconds == 60)
      {
        seconds = 0;
        minute++;
        if (minute == 60)
        {
          minute = 0;
          hour++;
        }
      }
    }
  }
}
//-------------------------------------------------------------------------------------------
void checkParameters()
{
  //Overvoltage
  if (voltage > overvoltage) isOvervoltage = true;
  else if (voltage < (overvoltage - 5)) isOvervoltage = false;
  //Undervoltage
  if (voltage < undervoltage) isUndervoltage = true;
  else if (voltage > (undervoltage + 5)) isUndervoltage = false;
  //Water Full
  if (percentage >= waterFull) isWaterFull = true;
  else if (percentage < (waterFull - 2)) isWaterFull = false;
  //Water Low
  if (percentage < waterLow) isWaterLow = true;
  else if (percentage >= (waterLow + 2)) isWaterLow = false;
  //Timer check
  if ((minute == minuteSet) && (hour == hourSet)) isTimeup = true;
  else isTimeup = false;
}
//-------------------------------------------------------------------------------------------
void control()
{
  static uint32_t controlTimer = millis() + 1000;

  if (controlTimer < millis())
  {
    controlTimer = millis() + 1000;

    checkParameters();

    //AUTO MODE Control
    switch (controlMode)
    {
    case S_AUTO : 
        if (isPumpON)
        {
          if (isWaterFull || isOvervoltage)
          {
            isPumpON = false;
            digitalWrite(PUMPCTRL, isPumpON);
          }
        }
        break;
    case AUTO:
        if (isPumpON)
        {
          if (isWaterFull || isOvervoltage)
          {
            isPumpON = false;
            digitalWrite(PUMPCTRL, isPumpON);
          }
        }else
        {
          if (!isPumpON)
          {
            if (isWaterLow)
            {
              isPumpON = true;
              digitalWrite(PUMPCTRL, isPumpON);
            }
          }
        }
        break;
    case TIMER:
        if (isPumpON)
        {
          if (isWaterFull || isOvervoltage || isTimeup)
          {
            isPumpON = false;
            digitalWrite(PUMPCTRL, isPumpON);
          }
        }
        break;
    default:
      break;
    }
  }
}
//-------------------------------------------------------------------------------------------
void retrieveSavedSettings()
{
  uint8_t dummy;

  //Tank Type Settings
  dummy = EEPROM.read(TT_ADD);
  if (dummy > 1)
  {
    tankType = VERTICAL;
    saveSettings(TT_ADD);
  }

  //Tank Height and Diameter Settings
  dummy = EEPROM.read(TH_ADD1);
  tankHeight = (EEPROM.read(TH_ADD2) << 8) | dummy;
  if (tankHeight > 400 || tankHeight < 50)
  {
    tankHeight = 140; //Default tank height in cm
    saveSettings(TH_ADD1);
  }
  dummy = EEPROM.read(TD_ADD1);
  tankDiameter = (EEPROM.read(TD_ADD2) << 8) | dummy;
  if (tankDiameter > 400 || tankDiameter < 50)
  {
    tankDiameter = 140; //Default tank Diameter in cm
    saveSettings(TD_ADD1);
  }
  //Control Mode Settings
  controlMode = EEPROM.read(CM_ADD);
  if (controlMode > TIMER)
  {
    controlMode = S_AUTO;
    saveSettings(CM_ADD);
  }
  //Overvoltage and Undervoltage Settings
  dummy = EEPROM.read(OV_ADD1);
  overvoltage = (EEPROM.read(OV_ADD2) << 8) | dummy;
  if (overvoltage > OVERVOLTAGE_L || overvoltage < OVERVOLTAGE_L)
  {
    overvoltage = 250;
    saveSettings(OV_ADD1);
  }
  dummy = EEPROM.read(UV_ADD1);
  undervoltage = (EEPROM.read(UV_ADD2) << 8) | dummy;
  if (undervoltage > UNDERVOLTAGE_U || undervoltage < UNDERVOLTAGE_L)
  {
    undervoltage = 170;
    saveSettings(UV_ADD1);
  }
  //Tank Water Full and Tank Water Low Settings
  waterFull = EEPROM.read(WF_ADD);
  if (waterFull > WATERFULL_U || waterFull < WATERFULL_L)
  {
    waterFull = WATERFULL_U;
    saveSettings(WF_ADD);
  }
  waterLow = EEPROM.read(WL_ADD);
  if (waterLow > WATERLOW_U || waterLow < WATERLOW_L)
  {
    waterLow = 50;
    saveSettings(WL_ADD);
  }
  minuteSet = EEPROM.read(M_ADD);
  if (minuteSet > 59)
  {
    minuteSet = 30;
    saveSettings(M_ADD);
  }
  hourSet = EEPROM.read(H_ADD);
  if (hourSet > 24)
  {
    hourSet = 0;
    saveSettings(H_ADD);
  }
}
//-------------------------------------------------------------------------------------------
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
//  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  digitalWrite(trig,LOW);
  pinMode(echo,INPUT_PULLUP);
  digitalWrite(echo,HIGH);
  pinMode(A3, INPUT);
  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(PUMPCTRL, OUTPUT);

  retrieveSavedSettings();

  display.begin();
  display.setContrast(30);

  display.clearDisplay();
  display.display();
  
  delay(2000);
  startUpScreen();
  
  
}

// the loop function runs over and over again forever
uint8_t refresh = 0;


void loop() 
{
  waterVolume();
  getVoltage();
  control();
  timerUpdate();
  uint8_t button = getButton();

  if (button == OK)
  {
    isPumpON = !isPumpON;
    digitalWrite(PUMPCTRL, isPumpON);
    if (isPumpON && (controlMode == TIMER))
    {
      minute = 0; hour = 0; seconds = 0; isTimeup = false;
    }
    _page = 1;
  }else if (button == UP)
  {
     tankSettings();
  }else if (button == DOWN)
  {
    pumpSettings();
  }
  
  
  if (displayTimer < millis())
  {
    displayTimer = millis() + 500;
    refresh++;
    if (refresh == 8)
    {
      _page++; if (_page > 2) _page = 0;
      refresh = 0;
    }  
    page(_page);
  }
                     
}
