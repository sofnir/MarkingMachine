#include <LiquidCrystal.h> 
#include <Keypad.h>
#include <Stepper.h>
#include <Servo.h>

enum colors {NONE, YELLOW, ORANGE, GREEN, BLUE, RED, BLACK};
int wallColors[6] = {NONE, NONE, NONE, NONE, NONE, NONE}; 
int numbers[6] = {1, 2, 3, 4, 5, 6}; //logic block numbers positions
int mark; //action mark
bool working = false;
int data = -1; //action index
int wall = 0; 
int color = 0;

//LCD
LiquidCrystal lcd(47, 48, 49, 50, 51, 52); 

//KEYPAD
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
byte rowPins[ROWS] = {43, 44, 45, 46}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {40, 41, 42}; //connect to the column pinouts of the keypad
char keys[ROWS][COLS] = 
{
 {'1','2','3'},
 {'4','5','6'},
 {'7','8','9'},
 {'*','0','#'}
};
char key;
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//MENU
int index = 0;

String mode = "Not selected";
String modeOptions[2] = {"Manual <-", "Automatic <-"}; 
String actionOptions[15] = 
{
  "Add block <-",
  "Belt s1 <-",
  "Belt s2 <-",
  "Belt s3 <-",
  "Belt s4 <-",
  "Belt s5 <-",
  "Belt s6 <-",
  "Close Jaws <-",
  "Open Jaws <-",
  "Rotate Servo <-",
  "Reset Servo <-",
  "Columne Left <-",
  "Columne Right <-",
  "Set First Pos <-",
  "Delete Block"
};
String automaticOptions[7] = {"1:", "2:", "3:", "4:", "5:", "6:", "CONFIRM <-"};
String colors[7] = {"NONE", "Yellow", "Orange", "Green", "Blue", "Red", "Black"}; 

//SENSORS
const int analogSensors[8] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A0 - gripper, A7 - rotation, A1-A6 - color
bool sensorsState[8] = {0};

//RUN
#define STEPS 24
Stepper stepperColumne(STEPS, 30, 32, 31, 33);
Stepper stepperBelt(STEPS, 36, 38, 37, 39);
int oneStep = 14;

//jaws
const int jawsPwm = 2;
const int jaws0 = 22;
const int jaws1 = 23;

//servo
Servo myServo;
int servoPosition = 180;

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  //belt
  stepperColumne.setSpeed(500);
  stepperBelt.setSpeed(500);
  
  //jaws
  pinMode(jaws0, OUTPUT);
  pinMode(jaws1, OUTPUT);
  digitalWrite(jaws0, LOW);
  digitalWrite(jaws1, LOW);
  
  //servo
  myServo.attach(4);
  myServo.write(servoPosition);
}

void loop()
{
  if(working == false) 
  {
    key = keypad.getKey(); 
  
    if(mode == "Not selected")
      selectMode();
    else if(mode == "Manual")
      selectAction();
    else if(mode == "Automatic")
      createAlgorithm();
  }
  else if(working == true)
  {
    

    working = false;
    data = -1;
    lcd.clear();   
  }
}

void columneRight()
{ 
  stepperColumne.step(-450); 
  data = 100;
  delay(500);
}

void columneLeft()
{ 
  stepperColumne.step(450); 
  data = 100;
  delay(500);
}

void setSensorsState()
{ 
  for(int i = 1; i <= 6; i++)
  {
    if(analogRead(analogSensors[i]) <= 10)
      sensorsState[i] = true;
    else sensorsState[i] = false;
  } 

  if(analogRead(analogSensors[7]) <= 4)
      sensorsState[7] = true;
    else sensorsState[7] = false;

  double sumSensor0 = 0;
  double valueSensor0;
  for(int i = 0; i < 10; i++)
    sumSensor0 += analogRead(analogSensors[0]);
  valueSensor0 = sumSensor0 / 10;
  if(valueSensor0 <= 29) 
    sensorsState[0] = false;
  else sensorsState[0] = true;
}

void showSensorsState()
{

   for(int i = 0; i < 8; i++)
  {
     Serial.print(analogRead(analogSensors[i]));
     Serial.print(" "); 
  }

  Serial.println();
  
  for(int i = 0; i < 8; i++)
  { 
     Serial.print(sensorsState[i]);
     Serial.print(" "); 
  }

  Serial.println();
}

void addBlock()
{
  while (sensorsState[0] == false) 
  {
    stepperBelt.step(-oneStep);
    setSensorsState();
  }
  delay(500);
  stepperBelt.step(-450);
  data = 100;
  delay(500);
}

void setFirstPosition()
{
  while (sensorsState[0] == false) 
  {
    stepperBelt.step(oneStep);
    setSensorsState();
  }
  delay(500);
  stepperBelt.step(-300);
  data = 100;
  delay(500);
}

void setColorPosition(int colorIndex)
{ 
  while (sensorsState[colorIndex] == false) 
  {
    setSensorsState();
    stepperBelt.step(-oneStep);
  }
    
  stepperBelt.step(0);
  data = 100;
  delay(500);
}

void openJaws()
{
  delay(250);
  analogWrite(jawsPwm, 0);
  digitalWrite(jaws0, HIGH);
  delay(25);
  analogWrite(jawsPwm, 110);
  delay(500);
  digitalWrite(jaws0, LOW);
  data = 100;
  delay(500);
}

void closeJaws()
{
  analogWrite(jawsPwm, 0);
  digitalWrite(jaws1, HIGH);
  delay(30);
  analogWrite(jawsPwm, 120);
  delay(750);
  digitalWrite(jaws1, LOW);   
  data = 100;
  delay(500);
}

void rotateServo()
{
  myServo.write(servoPosition - 90);
  servoPosition -= 90;
  data = 100;
  delay(500);
}

void resetServo()
{
  myServo.write(180);  
  servoPosition = 180;
  data = 100;
  delay(500);
}

void deleteBlock()
{
  setColorPosition(6);
  data = 100;
  delay(500);
}

void numbersRight()
{
  int tab[6];
  for(int i =0; i < 6; i++)
    tab[i] = numbers[i];
  numbers[1] = tab[4];
  numbers[2] = tab[1];
  numbers[3] = tab[2];
  numbers[4] = tab[3];
}

void numbersLeft()
{
  int tab[6];
  for(int i =0; i < 6; i++)
    tab[i] = numbers[i];
  numbers[1] = tab[2];
  numbers[2] = tab[3];
  numbers[3] = tab[4];
  numbers[4] = tab[1];
}

void numbersRotate()
{
  int tab[6];
  for(int i =0; i < 6; i++)
    tab[i] = numbers[i];
  numbers[0] = tab[3];
  numbers[1] = tab[0];
  numbers[3] = tab[5];
  numbers[5] = tab[1];
}

void setWall(int wall)
{  
  while(wall == 1)
  {
    wall = 0;
  }
  
  while(wall == 2)
  {
    closeJaws();
    columneRight();
    columneRight();
    openJaws();
    columneLeft();
    columneLeft();
    closeJaws();
    rotateServo();
    openJaws();
    resetServo();  
    numbersRight();
    numbersRight();
    numbersRotate();
    wall = 0;
  }

  while(wall == 3)
  {
    closeJaws();
    columneRight();
    openJaws();
    columneLeft();
    closeJaws();
    rotateServo();
    openJaws();
    resetServo();
    numbersRight();
    numbersRotate();
    wall = 0;
 }

 while(wall == 4)
 {
  closeJaws();
  rotateServo();
  openJaws();
  resetServo();
  numbersRotate();
  wall = 0;
 }
  
  while(wall == 5)
  {
    closeJaws();
    columneLeft();
    openJaws();
    columneRight();
    closeJaws();
    rotateServo();
    openJaws();
    resetServo();
    numbersLeft();
    numbersRotate();
    wall = 0;
  }

  while(wall == 6)
  {
    closeJaws();
    rotateServo();
    rotateServo();
    openJaws();
    resetServo();
    numbersRotate();
    numbersRotate();
    wall = 0;
  }
  
  for(int i = 0; i < 6; i++)
    Serial.print(numbers[i]);
  Serial.println();
}

void indexMove(String tabStr[], int tabSize) 
{ 
  if (key == '8') //MOVE UP
  {
    lcd.clear();

    if(index < tabSize-1)
      index++;
    else 
      index = 0;
  }
  else if (key == '2') // MOVE DOWN
  {
    lcd.clear();

    if(index > 0)
      index--;
    else 
      index = tabSize-1;
  }
}

void selectMode()
{
  lcd.setCursor(0, 0);
  lcd.print("Select mode:");
  lcd.setCursor(0, 1);
  lcd.print(modeOptions[index]);

  indexMove(modeOptions, 2);
  
  if(key == '5') 
  {
    if(index == 0) 
    {
      lcd.clear();
      mode = "Manual";
      index = 0;
    }
    else 
    {
      lcd.clear();
      mode = "Automatic";
      index = 0;
    }
  }
}

void selectAction()
{
  lcd.setCursor(0, 0);
  lcd.print("Select action:");
  lcd.setCursor(0, 1);
  lcd.print(actionOptions[index]);
  
  indexMove(actionOptions, 15);

  if(key == '0')
  {
    lcd.clear();
    mode = "Not selected";
    index = 0;
  }

  if(key == '5')
  {
    working = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-> " + actionOptions[index]);
    lcd.setCursor(0, 1);
    lcd.print("working...");

    setSensorsState();
    
    if(index >= 1 && index <=6)
      setColorPosition(data);
    else if(index == 0) 
      addBlock();
    else if(index == 7) 
      closeJaws();
    else if(index == 8) 
      openJaws(); 
    else if(index == 9) 
      rotateServo();      
    else if(index == 10) 
      resetServo();
    else if(index == 11)  
      columneLeft();
    else if(index == 12) 
      columneRight();
    else if(index == 13) 
      setFirstPosition(); 
    else if(index == 14) 
      deleteBlock();
  }
}

void createAlgorithm()
{ 
  lcd.setCursor(0, 0);
  lcd.print("Set wall colors:");
  lcd.setCursor(0, 1);
  
  if(index < 6)
    lcd.print(automaticOptions[index] + " < " + colors[wallColors[index]] + " >");
  else 
    lcd.print(automaticOptions[index]);

  if (key == '8') //MOVE UP
  {
    lcd.clear();

    if(index < 6)
      index++;
    else
      index = 0;
  }
  else if(key == '6') //MOVE RIGHT
  {
    lcd.clear();
    
    if(wallColors[index] < 6)
      wallColors[index]++;
    else
      wallColors[index] = 0;
  }
  else if (key == '2') // MOVE DOWN
  {
    lcd.clear();

    if(index > 0)
      index--;
    else 
      index = 6;
  }
  else if(key == '4') // MOVE LEFT
  {
    lcd.clear();
    
    if(wallColors[index] > 0)
      wallColors[index]--;
    else 
      wallColors[index] = 6;
  }
  else if(key == '5' && index == 6)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    for(int i = 0; i < 6; i++)
      lcd.print(wallColors[i]);
    addBlock();
    for(int wallIndex = 0; wallIndex < 6; wallIndex++) 
    {
      if(wallColors[wallIndex] != NONE)
      {
        int j = 0;
        while(numbers[j] != wallIndex + 1)
          j++;
        wall = j + 1;
        setWall(wall);
        setColorPosition(wallColors[wallIndex]);
        delay(1000);
        setFirstPosition();
        delay(2000);
      }
    }
  }
  else if(key == '0')
  {
    lcd.clear();
    index = 0;
    
    for(int i = 0; i < 6; i++)
      wallColors[i] = 0;
    
    mode = "Not selected";
  }
}
