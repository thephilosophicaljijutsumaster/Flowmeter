/*
This tutorial Demonstrates the use of Flow Sensor
 Web: blog.circuits4you.com
  The circuit:
Flow Sensor Connections
Yellow --- Pin 6
Red    --- +5V
Black  --- GND     
      
LCD Connections      
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 6
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 4
 * LCD D7 pin to digital pin 3
 * LCD R/W pin to ground
 * 1K resistor:
 * One end to ground
 * Another end to LCD VO pin (pin 3)
 
 This example code is in the public domain.
 */

// include the library code:
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

volatile int FlowPulse=0; //measuring the rising edges of the signal
double Calc,Calc1=0;                               
//int flowsensor = 24;    //The pin location of the sensor
char scaleFactor[1][10]={{"1.0000000"}};
int unit=0;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
//int sensor = 24;
int isFirst = 1;
int isSettingsActive = 0;
//int count=0;
//int holdKey =0;
boolean buttonActive = false;
boolean longPressActive = false;
float buttonTimer=0;
float MinFlowRate=0;
float MaxFlowRate=0;
int longPressTime=0;
byte rowPins[ROWS] = {10, A2, A3, A4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {13, 11, 12 , A5}; //connect to the column pinouts of the keypad
//int inPin =8;

// Variable declaration for measuring pulses
int sensorPin = 2;
unsigned long pulseTime; 
unsigned long lastPulseTime;
int lastSensorState = 0; 
int pulseCount;
int isWaiting=1;
unsigned long wait=0;
char holdKey;
char passKey;
int digKey=0;
String str;
unsigned long t_hold=2000;
//char code[4];
//int data_count;
double totalVol=0;
double totalVol1=0;
//char *charSF;
char flowUnits[10][8]={ "M3/Hr","M3/Min","M3/Sec","LPH","LPM","LPS","GPM US","GPS US","GPM UK","GPS UK"};
char volUnits[10][8]={"M3","M3","M3","Lit","Lit","Lit","Gal US","Gal US","Gal UK","Gal UK"};
double flowFactors[10] = {0.001,0.000016666666667,0.000000277777778,1,0.016666666666667,0.000277777777778,0.0044028675393,0.000073381125656,0.0036661541383,0.000061102568972};
double volFactors[10] = {0.001, 0.001, 0.001, 1, 1, 1, 0.2641720524, 0.2641720524, 0.2199692483, 0.2199692483};
static String currentFlowUnit = flowUnits[3];
//static String revFlowUnit;
static String revCalc;
static String currentVolUnit = volUnits[3];
double currentFlowFactor = flowFactors[3];
double currentVolFactor = volFactors[3];
char avgTime[3]="01";
char decAcc[2]="2";
unsigned long previousMillis = 0;
const long interval = 1000;
int u=0,v=0,w=0,s=0;
char flowString[16];
char flowWord[] = "FL = ";
char tempText[10],revText[10];
char tempCalc[10];
int addr = 0;
int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float voltage = 0.0,volt =0.0;            // calculated voltage
int NUM_SAMPLES = 5;
int isLow =0;
typedef struct {
  double tv=0;
  char sf[10] = "1.0000000"; 
  char at[3]="001";
  char da[2]="01";
  char cvu[8]="Lit";
  double cvf = 1;
  char cfu[8]="LPH";
  double cff = 1;
  int isFirst = 19;
}data_struct;
data_struct data;
data_struct data1;
char tempCVU[8]="Lit";
char tempCFU[8]="LPH";
//int sAT,sDA,sCVU,sCFU;
//char takeInput(int, int [], int []);


Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
   pinMode(sensorPin, INPUT);  
//   pinMode(flowsensor, INPUT); //initializes digital pin 2 as an input
//   Serial.begin(9600);         //This is the setup function where the serial port is initialised,
   attachInterrupt(0, rpm, RISING); //and the interrupt is attached
//   pinMode(inPin,INPUT);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Accurate Devices");
  delay(2000);
    
      EEPROM.get(addr,data1);
      if (data1.isFirst == 19){
      totalVol = data1.tv;
      strcpy(scaleFactor[0],data1.sf);
      strcpy(avgTime,data1.at);
      strcpy(decAcc,data1.da);
      currentVolUnit = String(data1.cvu);
      currentVolFactor = data.cvf;
      currentFlowUnit = String(data1.cfu);
      currentFlowFactor = data1.cff;
}else {
      EEPROM.put(addr,data);
      
}
}

void loop(){
 unsigned long currentMillis = millis();
 volt=measure();
    if (volt < 0.8 && isLow == 0){
        currentVolUnit.toCharArray(data.cvu,8);
        data.cvf=currentVolFactor;
        currentFlowUnit.toCharArray(data.cfu,8);
        data.cff=currentFlowFactor;
        data.tv=totalVol;
        strcpy(data.sf,scaleFactor[0]);
        strcpy(data.at,avgTime);
        strcpy(data.da,decAcc);
        EEPROM.put(addr,data);

      isLow = 1;
    }
    else if(volt > 0.9){
      isLow = 0;
    }
 Calc=0;
  sei();            //Enables interrupts
  if (currentMillis - previousMillis >= atof(avgTime)*1000){
    // save the last time you blinked the LED
    cli();            //Disable interrupts
    Calc = FlowPulse * 60 / 7.5 * atof(scaleFactor[0]) / ((currentMillis - previousMillis)/1000);
//    Calc = FlowPulse * 60 / 7.5 * atof(scaleFactor[0]) / atof(avgTime);
    Calc1 = ( Calc* currentFlowFactor); //(Pulse frequency x 60) / 7.5 Q, = flow rate in L/hour
    totalVol += Calc * (currentMillis - previousMillis)/3600/1000;
    totalVol1 = totalVol * currentVolFactor;
    previousMillis = currentMillis;
    FlowPulse=0;
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print("FL=");
//    for(v=0;v<sizeof(flowWord);v++){
//      flowString[v]=flowWord[v];}
    lcd.setCursor(15,0);
    lcd.rightToLeft();
    lcd.print(currentFlowUnit);
    lcd.print(" ");
    lcd.print(Calc1);   // print the Flow Rate
    lcd.leftToRight();
    lcd.print(" ");
    lcd.print(Calc1);   // print the Flow Rate
    lcd.print(" ");
    lcd.print(currentFlowUnit);

    lcd.setCursor(0,1);
    lcd.print("TT=");
//    for(v=0;v<sizeof(flowWord);v++){
//      flowString[v]=flowWord[v];}
    lcd.setCursor(15,1);
    lcd.rightToLeft();
    lcd.print(currentVolUnit);
    lcd.print(" ");
    lcd.print(totalVol1);   // print the Flow Rate
    lcd.leftToRight();
    lcd.print(" ");
    lcd.print(totalVol1);   // print the Flow Rate
    lcd.print(" ");
    lcd.print(currentVolUnit);
    totalVol += Calc * (currentMillis - previousMillis)/3600/1000;
    totalVol1 = totalVol * currentVolFactor;
    previousMillis = currentMillis;
    FlowPulse=0;
    
 }
 

   char key = keypad.getKey();
  
   if (key){
     holdKey = key;
//     Serial.println(key);
   }
  
   if (keypad.getState() == HOLD) {
      if (isFirst==1){
           t_hold = millis();
           isFirst = 0;
      }

      if (millis()-t_hold > 2000){
            password();

            isSettingsActive = 1;
            isFirst = 1;
      }
  
  }
  else{
    isFirst = 1;

   }
}

 
int password()
 {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Password");
    lcd.setCursor(0,1);
    
    int cPos[] = {0,1};
    char result[5]="00000";
    char timeLabel[3]="sec";
    int a=0;
    char key;
    char *code= takeInput(result,4,0,cPos,9,1);  // take password from the user
    code = &result[0];

        if (code[0] == '1' && code[1] == '6' && code[2] == '2' && code[3] == '0'){  // Compare password
              lcd.setCursor(0,1);                   // Set cursor at bottom 
              
// Configuring the value of scale factor
              lcd.clear();
              lcd.setCursor(0,0);                   // Set cursor to the top row
              lcd.print("Scale Factor");            // Display Scale Factor on lcd
              lcd.setCursor(0,1);                   // Set cursor to the bottom row
              lcd.print(scaleFactor[0]);         // Display the current value of scale factor 
              lcd.setCursor(0,1); 
              char *charSF=takeInput(scaleFactor[0],9,1,cPos,9,0);
              charSF = &scaleFactor[0][0];
              setUnit();
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Average Time");
              lcd.setCursor(0,1);
              lcd.print(avgTime);
              lcd.setCursor(0,1); 
              char *charTime=takeInput(avgTime,2,0,cPos,9,0);
              charTime = &avgTime[0];
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Decimal Accuracy");
              lcd.setCursor(0,1);
              lcd.print(decAcc);
              lcd.setCursor(0,1); 
              char *charAcc=takeInput(decAcc,1,0,cPos,3,0);
              charAcc = &decAcc[0];
              }
              else if (code[0] == '1' && code[1] == '6' && code[2] == '3' && code[3] == '0'){
                        resetVol();
              }
              else{
              lcd.setCursor(0,1);
              lcd.print("INCORRECT");
              delay(2000);
        } 
return;
}

// Function for taking value from the user through LCD
// count        -> number of digit to be taken from the user
// statList[]   -> array of position of digits that should not be altered (e.g. period symbol in float variable ""scaleFactor"")
// cursorPos[]  -> array of cursor position [col,row]

char *takeInput(char value[], int count, int statList, int cursorPos[], int maxi,int mask)
{
//  int ind=0;
//  for(ind=0;ind<count;ind++){
//    value[ind]='0';
//  }
  int i=0,j=0,row,col,dk=0,flag=0;
  int skip=0;
  char k;
  col = cursorPos[0];
  row = cursorPos[1];  
  while (i <= count){
    if (i==count){
                  col = cursorPos[0];
                  row = cursorPos[1];
                  lcd.setCursor(col,row); // move cursor to show each new char
                  i=0;
                }
    k = keypad.getKey();
    flag=0;
    if (i != statList || statList == 0){
              lcd.cursor();
             if (k != NO_KEY){
              if (k != '3'){                 
                 while (k != '1' && k != '3'){
                    if (k == '2'){
                      dk++;
                      if (dk > maxi){
                        dk=0;
                      }
              
                      lcd.setCursor(col,row); // move cursor to show each new char
                      lcd.print(dk);                
                      lcd.noCursor();
                    }
                     k = keypad.getKey();
                     flag=1;
                  }
                     if (flag == 0){
                        value[i]=value[i];
                     } 
                     else{
                        value[i]= dk + '0'; // convert digKey to character and store into data array
                         }
                        lcd.setCursor(col,row); // move cursor to show each new char
                        if (mask == 0){
                            lcd.print(value[i]);
                        }
                        else{
                            lcd.print("*");
                        }
                        col+=1;
                        i+=1;
                        dk=0;
                        flag=0;
                 }
                 else{
                  lcd.noCursor();
                  return value;
                  break;
                 }
             }
    }
    else{
      col+=1;
      lcd.setCursor(col,row);
      value[i]=value[i];
      i+=1;
    }
         }
          lcd.noCursor();
          return value;
}

void setUnit(){
  char b;
  int a=0,back=0;
  int x=0;
  for(x=0;x<sizeof(flowUnits)[0];x++){
    if (currentFlowUnit==flowUnits[x]){
      a=x;
    }
  }
  //Configuring the value of Flowrate unit
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Flow unit");
              lcd.setCursor(0,1);
              lcd.print(currentFlowUnit);
        while(back != 1){
              b=keypad.getKey();
              if (b != NO_KEY){
                while (b != '3'){
                  if (b == '2'){
                    a+=1;
                    if (a>9){
                      a=0;
                    }
                    currentFlowUnit = flowUnits[a];
                    currentFlowFactor = flowFactors[a];
                    currentVolUnit = volUnits[a];
                    currentVolFactor = volFactors[a];
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Flow unit");
              lcd.setCursor(0,1);
              lcd.print(currentFlowUnit);
                    }
              b=keypad.getKey();
                }  
                back = 1;
                }
              }
}

void resetVol(){
  char b;
  int a=0,back=0;;
  //Configuring the value of Flowrate unit
              lcd.clear();
//              lcd.setCursor(0,1);
//              lcd.print(totalVol1,atof(decAcc));
//              lcd.print(" ");
//              lcd.print(currentVolUnit);
              lcd.setCursor(0,0); 
              lcd.print("Reset Total :");
              lcd.setCursor(0,1);
//              lcd.cursor();
              lcd.print("No ");
        while(back != 1){
              b=keypad.getKey();
              if (b != NO_KEY){
                while (b != '3'){
                  if (b == '2'){
                    a+=1;
                    if (a>1){
                      a=0;
                    }
             if (a==0){
                lcd.setCursor(0,1);
                lcd.print("No ");
              }
              else{
                lcd.setCursor(0,1);
                lcd.print("Yes");
              }    
                  }
              
              b=keypad.getKey();
                }  
                back = 1;
              }
          }
          if (a==1){
            totalVol =0;
          }
//          lcd.noCursor();
}

//void resetVol(){
//  char b;
//  int a=0,back=0;;
//  //Configuring the value of Flowrate unit
//              lcd.clear();
//              lcd.setCursor(0,1);
//              lcd.print(totalVol1,atof(decAcc));
//              lcd.print(" ");
//              lcd.print(currentVolUnit);
//              lcd.setCursor(0,0); 
//              lcd.print("RST_Total(Y/N):");
//              lcd.cursor();
//              lcd.print("N");
//        while(back != 1){
//              b=keypad.getKey();
//              if (b != NO_KEY){
//                while (b != '3'){
//                  if (b == '2'){
//                    a+=1;
//                    if (a>1){
//                      a=0;
//                    }
//             if (a==0){
//                lcd.setCursor(15,0);
//                lcd.print("N");
//              }
//              else{
//                lcd.setCursor(15,0);
//                lcd.print("Y");
//              }    
//                  }
//              
//              b=keypad.getKey();
//                }  
//                back = 1;
//              }
//          }
//          if (a==1){
//            totalVol =0;
//          }
//          lcd.noCursor();
//}

void rpm ()     //This is the function that the interupt calls 
{ 
  FlowPulse++;  //This function measures the rising and falling edge of the hall effect sensors signal
}

//void EEPROMWritelong(int address, long value)
//      {
//      //Decomposition from a long to 4 bytes by using bitshift.
//      //One = Most significant -> Four = Least significant byte
//      byte four = (value & 0xFF);
//      byte three = ((value >> 8) & 0xFF);
//      byte two = ((value >> 16) & 0xFF);
//      byte one = ((value >> 24) & 0xFF);
//
//      //Write the 4 bytes into the eeprom memory.
//      EEPROM.write(address, four);
//      EEPROM.write(address + 1, three);
//      EEPROM.write(address + 2, two);
//      EEPROM.write(address + 3, one);
//      }
//
//long EEPROMReadlong(long address)
//      {
//      //Read the 4 bytes from the eeprom memory.
//      long four = EEPROM.read(address);
//      long three = EEPROM.read(address + 1);
//      long two = EEPROM.read(address + 2);
//      long one = EEPROM.read(address + 3);
//
//      //Return the recomposed long by using bitshift.
//      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
//      }

float measure(){
  analogReference(INTERNAL);
        while (sample_count < NUM_SAMPLES) {
        sum += analogRead(A0);
        sample_count++;
//        delay(10);
    }
   voltage = ((float)sum / (float)NUM_SAMPLES * 1.1) / 1024.0;
//    Serial.print(voltage);
//    Serial.println (" V");
    sample_count = 0;
    sum = 0;
//    analogReference(DEFAULT);
    return voltage;
}

