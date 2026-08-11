#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =================================================
// OLED
// =================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);


// =================================================
// PIN DEFINITIONS
// =================================================

// Ultrasonic
const int TRIG_PIN = 3;
const int ECHO_PIN = 2;

// Servo
const int SERVO_PIN = 9;

// Motors
const int LEFT_MOTOR_PIN = 4;
const int RIGHT_MOTOR_PIN = 5;

// Push buttons
const int LEFT_BUTTON = 6;
const int RIGHT_BUTTON = 7;


// =================================================
// SETTINGS
// =================================================

const int SAFE_DISTANCE = 15;

// Servo angles
const int LEFT_ANGLE = 150;
const int CENTER_ANGLE = 90;
const int RIGHT_ANGLE = 30;


// =================================================
// SERVO OBJECT
// =================================================

Servo scannerServo;


// =================================================
// SETUP
// =================================================

void setup() {

    // -----------------------------
    // Ultrasonic
    // -----------------------------

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);


    // -----------------------------
    // Motors
    // -----------------------------

    pinMode(LEFT_MOTOR_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN, OUTPUT);


    // Motors OFF at startup
    stopRobot();


    // -----------------------------
    // Buttons
    // -----------------------------

    // INPUT_PULLUP means:
    // Button released = HIGH
    // Button pressed  = LOW

    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT_PULLUP);


    // -----------------------------
    // Serial
    // -----------------------------

    Serial.begin(9600);


    // -----------------------------
    // Servo
    // -----------------------------

    scannerServo.attach(SERVO_PIN);

    scannerServo.write(CENTER_ANGLE);


    // -----------------------------
    // OLED
    // -----------------------------

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            0x3C
        )) {

        Serial.println("OLED failed!");

        while (true);
    }


    // Clear OLED
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println("OBSTACLE ROBOT");

    display.println();
    display.println("System starting...");

    display.display();

    delay(1000);
}


// =================================================
// GET DISTANCE
// =================================================

long getDistance() {

    long duration;

    // Make sure trigger is LOW
    digitalWrite(TRIG_PIN, LOW);

    delayMicroseconds(2);


    // Send trigger pulse
    digitalWrite(TRIG_PIN, HIGH);

    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);


    // Read echo
    duration = pulseIn(
        ECHO_PIN,
        HIGH,
        30000
    );


    // No echo
    if (duration == 0) {

        return 400;
    }


    // Convert to centimeters
    return duration / 58;
}


// =================================================
// OLED DISPLAY
// =================================================

void showStatus(
    long distance,
    const char* status
) {

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);


    // Title
    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println("OBSTACLE ROBOT");

    display.drawLine(
        0, 10,
        127, 10,
        SSD1306_WHITE
    );


    // Distance
    display.setCursor(0, 16);

    display.print("Distance: ");

    display.print(distance);

    display.println(" cm");


    // Status
    display.setCursor(0, 32);

    display.print("Status: ");

    display.println(status);


    // Buttons
    display.setCursor(0, 48);

    display.println("L=LEFT   R=RIGHT");


    display.display();
}


// =================================================
// STOP ROBOT
// =================================================

void stopRobot() {

    digitalWrite(
        LEFT_MOTOR_PIN,
        LOW
    );

    digitalWrite(
        RIGHT_MOTOR_PIN,
        LOW
    );
}


// =================================================
// MOVE FORWARD
// =================================================

void moveForward() {

    digitalWrite(
        LEFT_MOTOR_PIN,
        HIGH
    );

    digitalWrite(
        RIGHT_MOTOR_PIN,
        HIGH
    );
}


// =================================================
// TURN LEFT
// =================================================

void turnLeft() {

    Serial.println("Turning LEFT");

    // Left motor OFF
    digitalWrite(
        LEFT_MOTOR_PIN,
        LOW
    );

    // Right motor ON
    digitalWrite(
        RIGHT_MOTOR_PIN,
        HIGH
    );

    delay(500);

    stopRobot();
}


// =================================================
// TURN RIGHT
// =================================================

void turnRight() {

    Serial.println("Turning RIGHT");

    // Left motor ON
    digitalWrite(
        LEFT_MOTOR_PIN,
        HIGH
    );

    // Right motor OFF
    digitalWrite(
        RIGHT_MOTOR_PIN,
        LOW
    );

    delay(500);

    stopRobot();
}


// =================================================
// SCAN LEFT AND RIGHT
// =================================================

void scanForPath() {

    Serial.println("Scanning...");


    // -----------------------------
    // Look LEFT
    // -----------------------------

    scannerServo.write(
        LEFT_ANGLE
    );

    delay(500);

    long leftDistance =
        getDistance();


    // -----------------------------
    // Look RIGHT
    // -----------------------------

    scannerServo.write(
        RIGHT_ANGLE
    );

    delay(500);

    long rightDistance =
        getDistance();


    // -----------------------------
    // Return CENTER
    // -----------------------------

    scannerServo.write(
        CENTER_ANGLE
    );

    delay(300);


    // -----------------------------
    // Decide
    // -----------------------------

    if (leftDistance > rightDistance) {

        Serial.println(
            "LEFT has more space"
        );

        showStatus(
            leftDistance,
            "TURN LEFT"
        );

        turnLeft();
    }

    else {

        Serial.println(
            "RIGHT has more space"
        );

        showStatus(
            rightDistance,
            "TURN RIGHT"
        );

        turnRight();
    }
}


// =================================================
// MANUAL BUTTON CONTROL
// =================================================

bool checkButtons() {

    // LEFT button
    if (digitalRead(LEFT_BUTTON) == LOW) {

        stopRobot();

        Serial.println(
            "LEFT BUTTON"
        );

        showStatus(
            getDistance(),
            "MANUAL LEFT"
        );

        turnLeft();

        return true;
    }


    // RIGHT button
    if (digitalRead(RIGHT_BUTTON) == LOW) {

        stopRobot();

        Serial.println(
            "RIGHT BUTTON"
        );

        showStatus(
            getDistance(),
            "MANUAL RIGHT"
        );

        turnRight();

        return true;
    }


    return false;
}


// =================================================
// OBSTACLE AVOIDANCE
// =================================================

void avoidObject() {

    // Check buttons first
    if (checkButtons()) {

        return;
    }


    // Measure front
    long distance =
        getDistance();


    Serial.print(
        "Distance: "
    );

    Serial.print(
        distance
    );

    Serial.println(
        " cm"
    );


    // -----------------------------
    // OBSTACLE
    // -----------------------------

    if (distance <= SAFE_DISTANCE) {

        // STOP FIRST
        stopRobot();

        showStatus(
            distance,
            "OBSTACLE!"
        );

        delay(300);


        // SCAN
        scanForPath();


        // Continue forward
        moveForward();
    }


    // -----------------------------
    // PATH CLEAR
    // -----------------------------

    else {

        moveForward();

        showStatus(
            distance,
            "MOVING"
        );
    }
}


// =================================================
// MAIN LOOP
// =================================================

void loop() {

    avoidObject();

    delay(100);
}