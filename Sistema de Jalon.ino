///////// Para verificar el Script y las librerias del amplificador y del display, favor de dirigirse a la sigueinte pagina/////////////////////
//////////////////////////////https://drive.google.com/drive/u/0/folders/0ANYmirmdcnyOUk9PVA///////////////////////////////////////////////////


#include <Q2HX711.h>
//Configuracion de display LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4); //En algunas ocaciones la direccion del LCD no es 0x3f. En este caso cambie a 0x27 o visceversa.



//Conexion de Pins en Arduino 
const byte hx711_data_pin = 6;    //Data pin del HX711
const byte hx711_clock_pin = 5;   //Clock pin del rom HX711
int tara_button = 8;              //Boton de Tara
int RED=10;
int YELLOW=7;
int GREEN=9;
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin); // preparacion del modulo HX711

//Variables
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////Cambie este valor con el valor de masa conocido////////////////////////////////////////////////////////////////////////////////////////
float y1 = 4.44822; // Ajustado para valor de Patron de Peso. Por motivo de estandarizacion se usara 4.44822 para una pesa de 1 Libra///////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long x1 = 0L;
long x0 = 0L;
long Peso;
float avg_size = 1; // Cantidad de veces que tomara lectura para promediar fuerza
float tara = 0;
bool tara_pushed = false;

//////////////////////////////////////////////////////////



void setup() {
  Serial.begin(2000000);                 // preparacion de puerto serial
  PCICR |= (1 << PCIE0);              //habilitar escaneo de PCMSK0                                                 
  PCMSK0 |= (1 << PCINT0);            //Configurar Pin Digital 8 
  PCMSK0 |= (1 << PCINT3);            //Configurar Pin Digital 10
  pinMode(tara_button, INPUT_PULLUP);
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(YELLOW, OUTPUT);

  lcd.init();                         //Inicializacion de LCD
  lcd.backlight();                    //Activacion de Luz verde de fondo de LCD 

  
  delay(10);                        //Permite la estabilizacion del modulo y el sensor

  
  // Procedimiento de Tara
  for (int ii=0;ii<int(avg_size);ii++){
    delay(100);
    x0+=hx711.read();
  }
  x0/=long(avg_size);
  Serial.println("Coloque Peso Calibrado");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Coloque Pesa  ");
  lcd.setCursor(0,1);
  lcd.print("Calibrada (1 Lb)");
  // Proceso de Calibracion del Sensor
  int ii = 1;
  while(true){
    if (hx711.read()<x0+10000)
    {
      //no hace nada...
    } 
    else 
    {
      ii++;
      delay(1000);
      for (int jj=0;jj<int(avg_size);jj++){
        x1+=hx711.read();
      }
      x1/=long(avg_size);
      break;
    }
  }
  Serial.println("Calibración Completa");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Calibracion   ");
  lcd.setCursor(0,1);
  lcd.print("   Completa");
  


}



void loop() {
  // Promediando lecturas
  long reading = 0;
  for (int jj=0;jj<int(avg_size);jj++)
  {
    reading+=hx711.read();
  }
  reading/=long(avg_size);

  
  
  // Calculo de masa basado en calibracion y ajuste lineal
  float ratio_1 = (float) (reading-x0);
  float ratio_2 = (float) (x1-x0);
  float ratio = ratio_1/ratio_2;
  float mass = y1*ratio;

  if(tara_pushed)
  {
    tara = mass;
    tara_pushed = false;
    Serial.print("TARA");
    Serial.print(".");
    lcd.setCursor(0,0);
    lcd.print("      TARA      ");
    lcd.setCursor(0,1);
    lcd.print("      .         ");    
    delay(10);
    Serial.print(".");
    lcd.setCursor(0,0);
    lcd.print("      TARA      ");
    lcd.setCursor(0,1);
    lcd.print("      ..        "); 
    delay(10);
    Serial.println(".");
    lcd.setCursor(0,0);
    lcd.print("      TARA      ");
    lcd.setCursor(0,1);
    lcd.print("      ...       "); 
    delay(100);   
  }
  
  else
  {
    Serial.print(mass - tara);
    Serial.println(" N");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FUERZA     ");
    lcd.setCursor(0,1);
    lcd.print(mass - tara);
    lcd.print(" N");
  }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////CONFIGURACION DE SALIDAS DIGITALES PARA INDICADORES VERDE, AMARILLO Y ROJO //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  

Peso=-1*y1*ratio;

 if (Peso>=-100 && Peso<=14.99)                               //////////////Valores de Jalon BAJO////////////////////
  {
    digitalWrite(RED,LOW);
    digitalWrite(GREEN,LOW);
    digitalWrite(YELLOW, LOW);
  }
  
  if (Peso>=15 && Peso<=19.99)                               ///////////Valores de Jalon DESEADO//////////////////
  {
    digitalWrite(RED,LOW);
    digitalWrite(GREEN,HIGH);
    digitalWrite(YELLOW, LOW);
  }
  if (Peso>=20 && Peso<=29.99)                                 /////////Valores de Jalon ACEPTABLE///////////////
  {
    digitalWrite(RED,LOW);
    digitalWrite(GREEN,LOW);
    digitalWrite(YELLOW,HIGH);
  }
  if (Peso>=30 && Peso<=100)                                 //////////Valores de Jalon EXCESIVO////////////////
  {
    digitalWrite(RED,HIGH);
    digitalWrite(GREEN,LOW);
    digitalWrite(YELLOW, LOW);
    Serial.println(Peso);
    delay(10);


  }
}//Fin de void loop


//Interrupcion para deteccion de botones
ISR(PCINT0_vect)
{
  if (!(PINB & B00000001))
  {
    tara_pushed = true;           //Tara button was pushed
  }
  
}

 
  
