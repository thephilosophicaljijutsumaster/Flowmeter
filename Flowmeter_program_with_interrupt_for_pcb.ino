
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

volatile int FlowPulse=0; //measuring the rising edges of the signal
double Calc,Calc1=0;                               
char scaleFactor[10]="1.0000000";
int unit=0;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
int isFirst = 1;
int isSettingsActive = 0;
boolean buttonActive = false;
boolean longPressActive = false;
float buttonTimer=0;
float MinFlowRate=0;
float MaxFlowRate=0;
int longPressTime=0;
byte rowPins[ROWS] = {10, A2, A3, A4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {13, 11, 12 , A5}; //connect to the column pinouts of the keypad
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
double totalVol=0;
double totalVol1=0;
char flowUnits[10][8]={ "M3/Hr","M3/Min","M3/Sec","LPH","LPM","LPS","GPM US","GPS US","GPM UK","GPS UK"};
char volUnits[10][8]={"M3","M3","M3","Lit","Lit","Lit","Gal US","Gal US","Gal UK","Gal UK"};
double flowFactors[10] = {0.001,0.000016666666667,0.000000277777778,1,0.016666666666667,0.000277777777778,0.0044028675393,0.000073381125656,0.0036661541383,0.000061102568972};
double volFactors[10] = {0.001, 0.001, 0.001, 1, 1, 1, 0.2641720524, 0.2641720524, 0.2199692483, 0.2199692483};
static String currentFlowUnit = flowUnits[3];
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
  int isFirst = 11;
}data_struct;
data_struct data;
data_struct data1;
char tempCVU[8]="Lit";
char tempCFU[8]="LPH";
int TotalFlowPulse=0;

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
   pinMode(sensorPin, INPUT);  
   attachInterrupt(0, rpm, RISING); //and the interrupt is attached
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);
   lcd.print("Accurate Devices");
   delay(2000);
    
      EEPROM.get(addr,data1);
      if (data1.isFirst == 11){
      totalVol = data1.tv;
      strcpy(scaleFactor,data1.sf);
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
        strcpy(data.sf,scaleFactor);
        strcpy(data.at,avgTime);
        strcpy(data.da,decAcc);
        EEPROM.put(addr,data);

      isLow = 1;
    }
    else if(volt > 0.85){
      isLow = 0;
    }
 Calc=0;
  sei();            //Enables interrupts
  if (currentMillis - previousMillis >= atof(avgTime)*1000){
    cli();            //Disable interrupts
    Calc = FlowPulse * atof(scaleFactor) / ((currentMillis - previousMillis)/1000);
    Calc1 = ( Calc* currentFlowFactor); //(Pulse frequency x 60) / 7.5 Q, = flow rate in L/hour
    totalVol += Calc * (currentMillis - previousMillis)/3600/1000;
    totalVol1 = totalVol * currentVolFactor;
    previousMillis = currentMillis;
    TotalFlowPulse+=FlowPulse;
    FlowPulse=0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FL=");
    lcd.setCursor(15,0);
    lcd.rightToLeft();
    lcd.print(currentFlowUnit);
    lcd.print(" ");
    lcd.print(Calc1,atof(decAcc));   // print the Flow Rate
    lcd.leftToRight();
    lcd.print(" ");
    lcd.print(Calc1,atof(decAcc));   // print the Flow Rate
    lcd.print(" ");
    lcd.print(currentFlowUnit);
    lcd.setCursor(0,1);
    lcd.print(TotalFlowPulse);
//     lcd.print("TT=");
    lcd.setCursor(15,1);
    lcd.rightToLeft();
    lcd.print(currentVolUnit);
    lcd.print(" ");
    lcd.print(totalVol1,atof(decAcc));   // print the Flow Rate
    lcd.leftToRight();
    lcd.print(" ");
    lcd.print(totalVol1,atof(decAcc));   // print the Flow Rate
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
              lcd.clear();
              lcd.setCursor(0,0);                   // Set cursor to the top row
              lcd.print("Scale Factor");            // Display Scale Factor on lcd
              lcd.setCursor(0,1);                   // Set cursor to the bottom row
              lcd.print(scaleFactor);         // Display the current value of scale factor 
              lcd.setCursor(0,1); 
              char *charSF=takeInput(scaleFactor,9,1,cPos,9,0);
              charSF = &scaleFactor[0];
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
              lcd.setCursor(0,0); 
              lcd.print("Reset Total :");
              lcd.setCursor(0,1);
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

}

void rpm ()     //This is the function that the interupt calls 
{ 
  FlowPulse++;  //This function measures the rising and falling edge of the hall effect sensors signal
}

float measure(){
  analogReference(INTERNAL);
        while (sample_count < NUM_SAMPLES) {
        sum += analogRead(A0);
        sample_count++;
    }
    voltage = ((float)sum / (float)NUM_SAMPLES * 1.1) / 1024.0;
    sample_count = 0;
    sum = 0;
    return voltage;
}

