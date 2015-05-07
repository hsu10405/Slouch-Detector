#define voltPin 3
#define voutPin 5
#define maxVolt 255
#define int2vin 0.019531
#define int2vout 0.00488281
void setup() {
    pinMode(voltPin, OUTPUT);
    pinMode(voutPin, INPUT);
    Serial.begin(9600);
}

void loop() {
    int intensity = 0;
    for(intensity; intensity <= maxVolt; intensity++){
        analogWrite(voltPin, intensity);
        delay(50);
        Serial.print("Currnet Vin is: ");
        Serial.print(float(intensity)*int2vin);
        Serial.print(" V");
        if(float(intensity)*int2vin <= 1.65){
            Serial.println(" (OFF)");
        }
        else{
            Serial.println(" (ON)");
        }
        Serial.print("Current Vout is: ");
        Serial.print(float(analogRead(voutPin)*int2vout));
        Serial.println(" V");
        Serial.println("");
        delay(1000);
    }
}
