//www.elegoo.com
//2016.12.9

//Compilar como
// Placa:       "Arduino Nano"
// Procesador:  "ATMega328P (old Bootloader)
// Puerto:      El que sea


//We always have to include the library
#include "LedControl.h"
#include <Wire.h>
#include <DS3231.h>     // Esta libreria es la de Elegoo y funciona. 
// Si falla la clase DS3231 y RTCDateTime, borrar las librerias RTC y volver a cargar esta
#include <SimpleDHT.h>
#include <LiquidCrystal.h>

// Variables Globales
byte        temperature = 0;
byte        humidity = 0;
int         acumTemperatura = 0;
int         acumHumedad = 0;
byte        data[40] = {0};
int         pinDHT11 = 2;
String      temp = "valor de la temperatura";
String      humedad = "valor de la humedad";
int         numMedidas = 0;  

/* we always wait a bit between updates of the display */
unsigned long delaytime1=500;
unsigned long delaytime2=50;


// initialize the library with the numbers of the interface pins
//ARDUINO MEGA
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//ARDUINO NANO
//LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2


/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 11 is connected to the DataIn  -- 4 en el Nano
 pin 10 is connected to LOAD(CS)  -- 5 en el Nano
 pin 13 is connected to the CLK   -- 6 en el Nano
 We have only a single MAX72XX.
 */

// Clases
 
//LedControl lc=LedControl(11,13,10,4);
LedControl  lc=LedControl(4,6,5,4);
DS3231      clock;                                                                                               
RTCDateTime dt; 
SimpleDHT11 dht11;

void setup() {
  Serial.begin(9600);

  numMedidas = 0;

  // Initialize DS3231
  Serial.println("Initialize DS3231");;
  clock.begin();

  // ---------- OJO -----------
  // --------------------------
  //Compilar y cargar la primera vez cuando instalamos el reloj
  // luego comentar y volver a cargar para manterner la hora
  //clock.setDateTime(__DATE__, __TIME__);  

  
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  lc.shutdown(1,false);
  lc.shutdown(2,false);
  lc.shutdown(3,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,2);
  lc.setIntensity(1,2);
  lc.setIntensity(2,2);
  lc.setIntensity(3,2);
  /* and clear the display */
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  lc.clearDisplay(3);

  lcd.begin(16, 2);
  lcd.print("HOLA, MUNDO!");
  lcd.setCursor(0,0);
  lcd.print("                  ");
  lcd.setCursor(0,1);
  lcd.print("                  ");  
  
  
}


/*
  This function lights up a some Leds in a row.
 The pattern will be repeated on every row.
 The pattern will blink along with the row-number.
 row number 4 (index==3) will blink 4 times etc.
 */
void rows() {
  for(int row=0;row<8;row++) {
    delay(delaytime2);
    lc.setRow(0,row,B10100000);
    delay(delaytime2);
    lc.setRow(0,row,(byte)0);
    for(int i=0;i<row;i++) {
      delay(delaytime2);
      lc.setRow(0,row,B10100000);
      delay(delaytime2);
      lc.setRow(0,row,(byte)0);
    }
  }
}

/*
  This function lights up a some Leds in a column.
 The pattern will be repeated on every column.
 The pattern will blink along with the column-number.
 column number 4 (index==3) will blink 4 times etc.
 */
void columns() {
  for(int col=0;col<8;col++) {
    delay(delaytime2);
    lc.setColumn(0,col,B10100000);
    delay(delaytime2);
    lc.setColumn(0,col,(byte)0);
    for(int i=0;i<col;i++) {
      delay(delaytime2);
      lc.setColumn(0,col,B10100000);
      delay(delaytime2);
      lc.setColumn(0,col,(byte)0);
    }
  }
}

/* 
 This function will light up every Led on the matrix.
 The led will blink along with the row-number.
 row number 4 (index==3) will blink 4 times etc.
 */
void single() {
  for(int row=0;row<8;row++) {
    for(int col=0;col<8;col++) {
      delay(delaytime2);
      lc.setLed(0,row,col,true);
      delay(delaytime2);
      for(int i=0;i<col;i++) {
        lc.setLed(0,row,col,false);
        delay(delaytime2);
        lc.setLed(0,row,col,true);
        delay(delaytime2);
      }
    }
  }
}


/* Funcion para Dibujar Numeros de 7 x 5 leds
 *  led : Display 0, 1, 2, 3
 *  numero : numero a pintar 0-9
 *  desplazamiento : columnas a desplazar desde la 0
 */
void PrintNumero (int led, char charnumero, int desplazamiento) {
  byte numeros [50] = {B01111100, B10001010, B10010010, B10100010, B01111100,   // 0
                       B00000000, B01000010, B11111110, B00000010, B00000000,   // 1
                       B01001110, B10010010, B10010010, B10010010, B01100010,   // 2 
                       B10000100, B10000010, B10010010, B10110010, B11001100,   // 3
                       B00011000, B00101000, B01001000, B11111110, B00001000,   // 4
                       B11100100, B10100010, B10100010, B10100010, B10011100,   // 5
                       B01111100, B10010010, B10010010, B10010010, B00001100,   // 6
                       B10000010, B10000100, B10001000, B10010000, B11100000,   // 7
                       B01101100, B10010010, B10010010, B10010010, B01101100,   // 8
                       B01100000, B10010010, B10010010, B10010010, B01111100};  // 9
  
  int numero;

  switch (charnumero) {
    case '0': numero = 0; break;
    case '1': numero = 1; break;
    case '2': numero = 2; break;
    case '3': numero = 3; break;
    case '4': numero = 4; break;
    case '5': numero = 5; break;
    case '6': numero = 6; break;
    case '7': numero = 7; break;
    case '8': numero = 8; break;
    case '9': numero = 9; break;
    default : numero = 0; break;
   }
  
  int pos = numero *5;
  
  /*Serial.print("Numero a imprimir :"); 
  Serial.print(charnumero);
  Serial.print(" posicion: ");
  Serial.println(pos);
  */
  
  lc.setColumn(led,0+desplazamiento,numeros[pos]);
  lc.setColumn(led,1+desplazamiento,numeros[pos+1]);
  lc.setColumn(led,2+desplazamiento,numeros[pos+2]);
  lc.setColumn(led,3+desplazamiento,numeros[pos+3]);
  lc.setColumn(led,4+desplazamiento,numeros[pos+4]);
  //lc.setColumn(led,5,numeros[pos+5]);
  //delay(delaytime1);

}

void Puntos () {
   byte a[2]={B00100100, B00000000};
   lc.setColumn(2 ,7, a[0]);
   delay(600);
   lc.setColumn(2 ,7, a[1]);
   delay(600);
}


void loop() { 
  /* 
   *  Muestra el Reloj 
   */
  
  dt = clock.getDateTime();
  char hora [3] ="";
  char minutos [3];
  
  sprintf (hora, "%s", clock.dateFormat("H", dt));
  sprintf (minutos, "%s", clock.dateFormat("i", dt));

  //DesplazamientoBarra();
  PrintNumero (3, hora[0], 1);
  PrintNumero (2, hora[1], 1);
  PrintNumero (1, minutos[0], 1);
  PrintNumero (0, minutos[1], 1);
  Puntos();  


  /* 
   *  Muestra la temperatura 
   */
  if (numMedidas == 10) {
     // Calculamos la media y mostramos
     temperature = acumTemperatura / 10;
     humidity = acumHumedad / 10;

//     Serial.print (" --> Calculo la media de: "); Serial.print ((int)numMedidas);
//     Serial.print (" medidas, suma temp: "); Serial.print ((int)acumTemperatura); Serial.print (", media temp :"); Serial.print ((int)temperature);
//     Serial.print (", suma hum: "); Serial.print ((int)acumHumedad); Serial.print (", media hum :"); Serial.println ((int)humidity);
     
     lcd.setCursor(0,0);
     String cadena = " grados";
     temp = (int)temperature  + cadena;
     lcd.print(temp);
  
     String cadena2 = " % humedad";
     humedad = (int)humidity + cadena2;
     lcd.setCursor(0,1);
     lcd.print(humedad);
     
     acumTemperatura = 0;
     acumHumedad = 0;
     numMedidas = 0;  
  }
  else {
     temperature = 0;
     humidity = 0;
     data[40] = {0};
    
     if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
        Serial.println("Read DHT11 failed");
        return;
     }

     /*
     for (int i = 0; i < 40; i++) {
        Serial.print((int)data[i]);
        if (i > 0 && ((i + 1) % 4) == 0) {
           Serial.print(' ');
        }
     }
     Serial.println("");
     */ 
//     Serial.print("Sample OK: ");
//     Serial.print((int)temperature); Serial.print(" *C, ");
//     Serial.print((int)humidity); Serial.print(" %");
//     
//     Serial.print("medida : ");Serial.print((int)numMedidas);
       
     acumTemperatura = acumTemperatura + temperature;
     acumHumedad = acumHumedad + humidity;
     numMedidas++;
//     Serial.print(", suma temp: "); Serial.print ((int)acumTemperatura);
//     Serial.print(", suma hum: "); Serial.println ((int)acumHumedad);
  }
}


// Pruebas

void DesplazamientoBarra () {
   byte encendido[1]={B01111111};
   byte apagado[1]={B00000000};

   for (int i = 0 ; i <= 3; i++) {
      for (int j= 7; j >= 0; j--) {
         lc.setColumn(i,j,encendido[0]); 
         delay (200);
         lc.setColumn(i,j,apagado[0]); 
          
      } 
   }
   for (int i = 3 ; i >= 0; i--) {
      for (int j= 0; j <= 7; j++) {
         lc.setColumn(i,j,encendido[0]); 
         delay (200);
         lc.setColumn(i,j,apagado[0]); 
          
      } 
   }
}

