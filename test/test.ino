#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "C:\Users\ayode\Documents\Arduino\libraries\Adafruit_GFX_Library\Fonts\DSEG14_Classic_Italic_7Bitmaps.h"


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

#define VERTICAL  0
#define HORIZONTAL 1

uint16_t tankHeight = 140, tankDiameter = 140;
uint16_t volume = 920, capacity = PI * pow((tankDiameter / 20),2) * ((tankHeight - 10 ) / 10);
uint8_t tankType = VERTICAL;

bool isPumpON = false;
int16_t voltage = 221;

uint32_t displayTimer;
uint8_t hour = 0, minute = 0;

uint8_t controlMode = S_AUTO, _page = 2;


//############################################################################################
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

  uint8_t percentage = 100 - (((getDistance() - 10) * 100) / tankHeight);
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
  uint8_t pumpHeight = 20;
  uint8_t pumpWidth = 30;
  uint8_t pumpx = 2;
  uint8_t pumpy = ((48 - pumpHeight) / 2) - 1+5;
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
  display.fillRect(circlex-2,circley-circler+2+border, 4,(circler*2)-(border*2)-2, BLACK);
  display. fillRect(circlex-2,circley+circler-border-2-6, 4, 4, WHITE);

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
    printTextCenter("AUTO",42,83,11,30);
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
    voltage = ((accum / 32) * 85) / 1000;
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
void switchActive(uint8_t _active)
{
  switch(_active)
    {
      case 0: typeForm(1,true);
              heightForm(tankHeight, false);
              diameterForm(tankDiameter, false);
              saveForm(false);
              break;
      case 1: typeForm(1,false);
              heightForm(tankHeight, true);
              diameterForm(tankDiameter, false);
              saveForm(false);
              break;
      case 2: typeForm(1,false);
              heightForm(tankHeight, false);
              diameterForm(tankDiameter, true);
              saveForm(false);
              break;
      case 3: typeForm(1,false);
              heightForm(tankHeight, false);
              diameterForm(tankDiameter, false);
              saveForm(true);
              break;
      default: typeForm(1,false);
              heightForm(tankHeight, false);
              diameterForm(tankDiameter, false);
              saveForm(false);
              break;
    }
    display.display(); 
}
//-------------------------------------------------------------------------------------------
void setActiveParameter(uint8_t _active)
{
  uint32_t blinkTimer = millis() + 1000;
  bool toggle = false;
  uint8_t button = getButton();
  while (button != OK)
  {
    button = getButton();
    if (millis() > blinkTimer)
    {
      blinkTimer = millis() + 500;
      if (toggle)
      {
        switchActive(0xf);
        toggle = !toggle;
      }else
      {
        switchActive(_active);
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
      switchActive(_active);
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
      switchActive(_active);
    }
  }
}
//-------------------------------------------------------------------------------------------
void tankSettings()
{
  bool isSaved = false;
  uint8_t active = 0;
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
      switchActive(active);
    }else if (button == UP)
    {
      if (active > 0) active--;
      switchActive(active);
    }else if (button == OK)
    {
      if (active == 3) isSaved = true;
      else setActiveParameter(active);
    }

  }

   
}
//-------------------------------------------------------------------------------------------
void pumpSettings()
{

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

  display.begin();
  display.setContrast(30);

  display.clearDisplay();
  display.display();
  
  delay(2000);
  startUpScreen();
  // uint8_t x,y;
  // // tankSettings();
  // display.setCursor(0,0);
  // while (1)
  // {
  //   if (getButton() == OK)
  //   {
  //     if (x < 83) x++;
  //     display.drawPixel(x,y,BLACK);
  //     display.display();
  //   }else if (getButton() == DOWN)
  //   {
  //     if (y <47) y++;
  //     display.drawPixel(x,y,BLACK);
  //     display.display();
  //   }else if (getButton() == UP)
  //   {
  //     if (y > 0) y--;
  //     display.drawPixel(x,y,WHITE);
  //     display.display();
  //   }
  // }
  
}

// the loop function runs over and over again forever
uint8_t refresh = 0;


void loop() 
{
  waterVolume();
  getVoltage();
  uint8_t button = getButton();

  if (button == OK)
  {
    isPumpON = !isPumpON;
    digitalWrite(PUMPCTRL, isPumpON);
    _page = 1;
  }else if (button == UP)
  {
     tankSettings();
  }else if (button == DOWN)
  {
    // pumpSettings();
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
