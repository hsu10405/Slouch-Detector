#define fetPin 1;

void setup() {
    pinMode(fetPin, OUTPUT);
}

void loop() {
    /*
    digitalWrite(fetPin, HIGH);
    delay(50);
    digitalWrite(fetPin, LOW);
    delay(25);
    */
    
    for(int intensity = 0; intensity < 255; intensity++){
        analogWrite(fetPin, intensity);
        delay(100);
    }
}
