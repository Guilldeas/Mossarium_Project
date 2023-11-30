/********************************************************************************
*                                                                               *
*                         Moss Terrarium Monitor/Control                        *
*                                                                               *
*                                Guillermo D.S.                                 *
*                                                                               *
*********************************************************************************
*                                                                               *
*   Description:                                                                *
*                                                                               *
*   This project aims to provide the user with the following                    *
*   measurements:                                                               *
*   ├── Temperature                                                             *
*   ├── Relative humidity                                                       *
*   └── Light intensity                                                         *
*                                                                               *
*   Additionaly the device should implement the following                       *
*   functionalities:                                                            *
*   ├── Lighting control through a relay box                                    *
*   ├── User input through a rotatory encoder                                   *
*   └── User output through an OLED display                                     *
*                                                                               *
*********************************************************************************
*                                                                               *
*   Dev Roadmap:                                                                *
*                                                                               *
*   1) Read sensor data  [v First pass]                                         *
*      └── Perform the averaging directly on loop() instead of using for,       *
*          averages will take 1h and we need to always listen for               *
*          serial commands.                                                     *
*                                                                               *
*   2) Operate lighting relay  [v First pass]                                   *
*                                                                               *
*   3) Send data to PC for storage when requested  [v First pass]               *
*                                                                               *
*   4) User input/display  [x]                                                  *
*                                                                               *
*   5) Handle PC interrupts/SD storage  [x]                                     *
*                                                                               *
********************************************************************************/


#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>




// -------------------------- Variable declaration ----------------------------- //

int N_Samples = 10;            // Average N samples for each measurements
int N_Failures = 0;            // Number of failed attempts to read data
float humd = 0;
float temp = 0;
float avg_humd = 0;
float avg_temp = 0;
char Python_Command;           // Carries an order received through serial comm
char Send_data = 's';          // Serial comm order to send data
bool reading_failed = false;   // Flag raised when readings fail for all averages

#define DHTPIN 20
#define DHTTYPE DHT22
const int Relay_Pin = 22;

DHT dht(DHTPIN, DHTTYPE);


void setup() {

  // Initialize DHT sensor.
  Serial.begin(9600);
  Serial.println(F("DHT22 Startup"));
  dht.begin();

  // Setup relay
    pinMode(Relay_Pin, OUTPUT);

}

void loop() {

  // -------------------------- Read sensor data ----------------------------- //

  // Average N Samples.
  for (int i = 0; i < N_Samples; ++i){
    // DHT22 samples at =< 0.5Hz. Reading temperature or humidity  takes about 
    // 250 milliseconds
    delay(2000);
    
    humd = dht.readHumidity();
    temp = dht.readTemperature();

    // Check if any readings failed and exit early loop early to try again.
    if (isnan(humd) || isnan(temp) ){

      //Serial.print(F("Failed to read from DHT sensor. Failed attempts: "));
      //Serial.println(N_Failures);
      N_Failures += 1;
      continue;
    }

    // Only if measurement was succesful add to average
    avg_humd += humd;
    avg_temp += temp;
  }

  // Compute average
  avg_humd = avg_humd / N_Samples;
  avg_temp = avg_temp / N_Samples;

  // If readings fail for all measurementes raise failure flag
  if (N_Failures == N_Samples){
    reading_failed = true;
    N_Failures = 0;

  }
  else {
    reading_failed = false;
    N_Failures = 0;
  }

  // ---------------------- Listen and answer commands ------------------------- //

  // Check for bytes on comm buffer
  if (Serial.available() > 0){
    Python_Command = Serial.read();

    if(Python_Command == Send_data){

      // Send a string if readings failed
      if(reading_failed){
        Serial.println(F("Readings failed, could not compute average"));
      }

      // Send data if all is good
      else{
        Serial.println(avg_humd);
        Serial.println(avg_temp);
      }
    }
  }

  // --------------------------- Relay Control -------------------------------- //
  // Output 5V (HIGH) for 1 second
  /*digitalWrite(Relay_Pin, HIGH);
  delay(2000);

  // Turn off the output (LOW) for 1 second
  digitalWrite(Relay_Pin, LOW);
  delay(2000);*/
}