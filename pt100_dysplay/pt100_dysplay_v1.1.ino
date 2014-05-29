#include <LiquidCrystal.h>
#include <PID_v1.h>

// Control loop beta temperature with PT100 RTD
// Autor: Sergio Pablo Peñalve
// catriel - Rio Negro
// Translation to English: Mats Karlsson [2014]

// LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  // Conventional version

// Modification
LiquidCrystal lcd(6, 7, 2, 3, 4, 5);     // Use the first row of pins

double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint,5,0.2,0.5, DIRECT);
 
//#define _uno
 #define _mega_2560
 
// Calibration method

// Use a dry or oil bath thermostatic in lack thereof use thermometer, pyrometer or
// thermocouple frozen water and boiled water

// The PT100 RTD resistance varies linearly with the temperature so meassuring two points
// in low temp and high temp can generate a linear function with these points and
// transform the ADC value in degrees C°.


// Another way is to calculate the voltage and resistance rule of three to the value ADC obtain T°

 
float Ay1= 17.2;  // 1st RTD °C cold calibration points
float Ax1=306;    // ADC

float By1= 17.2;  // 2nd RTD °C cold calibration points
float Bx1=302;    // ADC

float Ay2=60;     // 1st RTD °C hot calibration points
float Ax2=623;    // ADC
 
float By2=60;     // 2nd RTD °C hot calibration points
float Bx2=618;    // ADC

int t1,t2,i,cant;
float acum1,acum2,m1,m2,c1,c2,temp1,temp2;
int setP=15;      // Standard temperature for flow or other

void setup()
{
  cant=10000;

  #ifdef _uno    
  analogReference(INTERNAL);     // Arduino UNO
  #endif
       
  #ifdef _mega_2560    
  analogReference(INTERNAL1V1);  // Arduino Mega 2560
  #endif
  
  Serial.begin(9600);
  
  myPID.SetMode(AUTOMATIC);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
}


void loop()
{
  // Sampling analog signal
  for (i=0;i<10000;i++)
  {
  t1 = analogRead(1);
  t2 = analogRead(2);

  acum1=acum1+(float)t1;
  acum2=acum2+(float)t2;
  }

  acum1=acum1/cant;
  acum2=acum2/cant;

  /*
  if ((acum1 = 0 )||( acum2=0))
  {
    Output=0;
  }
  */
 
  // Calculate linear function of temperature
  // y=m * b + c
  // where m is the slope
  // y - b are points
  // c is the constant
   
  m1=pendiente(Ay1,Ax1,Ay2,Ax2);
  m2=pendiente(By1,Bx1,By2,Bx2);

  c1=constante(m1,Ay1,Ax1);
  c2=constante(m2,By1,Bx1);

  temp1=obt_temp(m1,c1,acum1);
  temp2=obt_temp(m2,c1,acum2);
  lcd.clear();
  lcd.print("PV:"); 
  lcd.print(temp1);

  Setpoint=obt_adc(m1,c1,setP);
  lcd.setCursor(0, 1); 
  lcd.print("SP:");
  lcd.print(obt_temp(m1,c1,Setpoint));

 // Check fails in some RTD
  Input=acum1;
  if (acum1<= Ax1)
  {
    Input = acum2;
  }
  if(acum2<=Ax1)
  {
    Input = acum1;
  }

  if (Input < Ax1 or Input < Bx1)
  {
   myPID.SetMode(MANUAL);
   lcd.clear();
   lcd.print("Falla en RTD"); 
  }/*else 
  {
  Input =acum1;
  }*/

  // Input = acum1;
  myPID.Compute();

// Control priority
#ifdef _uno
  analogWrite(9,Output);
  analogWrite(10,Output);
  analogWrite(11,Output);
#endif

#ifdef _mega_2560
  analogWrite(11,Output);
  analogWrite(12,Output);
  analogWrite(13,Output);
#endif
if (Output>0)
  {
   lcd.print(" OUT:ON ");
  }
   else
  {
   lcd.print(" OUT:OFF");   
  }
// Data show the end

  menu1();

// This is for debug can be removed

  // Analog values ​​1 and 2
  /*
  Serial.print("Input RTD 1--> ");
  Serial.println(t1);

  Serial.print("Input RTD 2--> ");
  Serial.println(t2);
  */
  
  // Temperatures 1 and 2
  /*
  Serial.print("temperature t0: ");
  Serial.println(temp1);
  Serial.print("temperature t1: ");
  Serial.println(temp2);
  */
  
  /*
  Serial.print("Input RTD Acum 10000 PV--> ");
  Serial.println(acum1);
  Serial.print("Input RTD Acum 10000 PV--> ");
  Serial.println(acum2);
  */
  
  /*
  Serial.print("Temperature -> ");
  Serial.println(temp);
  */
  
  Serial.print("Input temp --> ");
  Serial.println(Input);
  
    
  Serial.print("Setpoint temp --> ");
  Serial.println(Setpoint);
  
  Serial.print("Output");
  Serial.println(Output);
}

// Slope function with respect to two points remember math and wondered served to fuck this lol

float pendiente(float y1,float x1,float y2, float x2)
{
 float m=0;
 m=(y2-y1)/(x2-x1);
 return m;
}

// I get the constant clearing
float constante(float m,float y1,float x1)
{
  float c=0;   
  c=y1-(m*x1);
  return c;
}

// esta es la funcion lineal
float obt_temp(float m,float c,float x1)
{
  float y1=(m*x1)+c;
  return y1;
}


// esta es una funcion inversa le damos la temp y nos devuelve el ADC para el PID
float obt_adc(float m,float c,float y1)
{
  float x1=(y1 - c)/m;
  return x1;
}


void menu1()
{
analogReference(DEFAULT);
  int boton=analogRead(0);
  Serial.println(boton);
  if (boton == 0)
       {       
         Serial.println("Right");  // Right
       }else
  if (boton > 90 && boton < 200)   // 90 - 200
       {
       Serial.println("Up");
       setP=setP++;
       } else 
  if (boton > 200 && boton < 300)
       {       
       Serial.println("Down");     // Down
       setP=setP--;
       }else
  if (boton > 400 && boton < 500)
       {       
         Serial.println("Left");   // Left
       }else
  if (boton > 600 && boton < 700)
       {       
         Serial.println("Select"); // Select
       }
  
  #ifdef _uno    
  analogReference(INTERNAL);       // Arduino UNO
  #endif

  #ifdef _mega_2560    
  analogReference(INTERNAL1V1);    // Arduino Mega 2560
  #endif
}