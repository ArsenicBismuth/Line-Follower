/* The average rotary encoder has three pins, seen from front: A C B
   Clockwise rotation A(on)->B(on)->A(off)->B(off)
   CounterCW rotation B(on)->A(on)->B(off)->A(off)
   
   Rotary Enc Ref : https://playground.arduino.cc/Main/RotaryEncoders
*/

// usually the rotary encoders three pins have the ground pin in the middle
enum PinAssignments {
  encPinA = 2,   // right
  encPinB = 3   // left
};

volatile unsigned long encPos = 0;   // a counter for the dial
unsigned long lastReportedPos = 1;   // change management
static boolean rotating = false;    // debounce management
float ang = 0.0;
int spd = 0;

unsigned long cmillis = 0;          // Current time
unsigned long pmillis = 0;          // Previous time

// interrupt service routine vars
boolean A_set = false;
boolean B_set = false;

void setup() {
    pinMode(encPinA, INPUT);
    pinMode(encPinB, INPUT);
    
    // turn on pullup resistors
    digitalWrite(encPinA, HIGH);
    digitalWrite(encPinB, HIGH);
    
    // encoder pin on interrupt 0 (pin 2)
    attachInterrupt(0, doencA, CHANGE);
    // encoder pin on interrupt 1 (pin 3)
    attachInterrupt(1, doencB, CHANGE);
    
    Serial.begin(9600);  // output
    Serial.println("Angle(n) \tAngle(deg) \tSpeed(RPM)");
}

// main loop, work is done by interrupt service routines, this one only prints stuff
void loop() {
    cmillis = millis();
    rotating = true;  // reset the debouncer

    if (lastReportedPos != encPos) {
        Serial.print(encPos, DEC);
        Serial.print("\t");
        
        ang = encPos * 0.15;    // Max 9830.25 deg or 27.30625 rev
        Serial.print(ang, 2);
        Serial.print("\t");

        spd = (encPos - lastReportedPos) * 1000 * 0.15 / (cmillis - pmillis) * 360 / 60;
        Serial.println(spd);
        
        lastReportedPos = encPos;
        pmillis = cmillis;
    }
}

// Interrupt on A changing state
void doencA() {
    // debounce
    if ( rotating ) delay (1);  // wait a little until the bouncing is done
    // Test transition, did things really change?
    if ( digitalRead(encPinA) != A_set ) { // debounce once more
        A_set = !A_set;
        if ( A_set && !B_set ) encPos += 1; // adjust counter + if A leads B
        rotating = false;  // no more debouncing until loop() hits again
    }
}

// Interrupt on B changing state, same as A above
void doencB() {
    if ( rotating ) delay (1);
    if ( digitalRead(encPinB) != B_set ) {
        B_set = !B_set;
        if ( B_set && !A_set ) encPos -= 1; //  adjust counter - 1 if B leads A
        rotating = false;
    }
}
