#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// Initialize the LCD with I2C address 0x27 and size 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns

// Define the symbols on the buttons of the keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'c', '0', '#', '/'}
};

// Connect keypad ROW0, ROW1, ROW2, ROW3 to these Arduino pins
byte rowPins[ROWS] = {9, 8, 7, 6};

// Connect keypad COL0, COL1, COL2, COL3 to these Arduino pins
byte colPins[COLS] = {5, 4, 3, 2};

// Create the Keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputString = "";    // String to hold input from keypad
float firstNumber = 0;      // First operand
float secondNumber = 0;     // Second operand
char operatorChar;          // Operator (+, -, *, /)
boolean isSecondNumber = false; // Flag to check if we are entering the second number
boolean isCalculatorMode = true; // Flag to check if we are in calculator mode

const int switchPin = 10;   // Pin connected to the switch
boolean lastSwitchState = HIGH; // Last state of the switch
boolean currentSwitchState;     // Current state of the switch

void setup() {
  Serial.begin(9600);       // Start serial communication at 9600 baud
  lcd.init();              // Initialize the LCD
  lcd.backlight();          // Turn on the backlight
  lcd.clear();              // Clear any previous content on the LCD
  pinMode(switchPin, INPUT_PULLUP); // Initialize the switch pin with an internal pull-up resistor

  if (!rtc.begin()) {
    lcd.print("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    lcd.print("RTC lost power, set the time!");
    // Uncomment the following line to set the date and time
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  currentSwitchState = digitalRead(switchPin); // Read the state of the switch

  // Check if the switch state has changed
  if (currentSwitchState == LOW && lastSwitchState == HIGH) {
    delay(50); // Debounce delay
    isCalculatorMode = !isCalculatorMode; // Toggle mode
    lcd.clear(); // Clear the LCD
  }

  lastSwitchState = currentSwitchState; // Update the switch state

  if (isCalculatorMode) {
    runCalculatorMode();
  } else {
    displayDate();
  }
}

void runCalculatorMode() {
  char key = keypad.getKey(); // Get the key pressed

  if (key) {
    // If a key is pressed
    if ((key >= '0' && key <= '9') || key == '.') {
      // If the key is a number or a decimal point
      inputString += key;     // Append the key to the input string
      lcd.print(key);         // Print the key to the LCD
    } else if (key == '+' || key == '-' || key == '*' || key == '/') {
      // If the key is an operator
      if (inputString.length() > 0) {
        firstNumber = inputString.toFloat(); // Convert input string to float
        inputString = "";                    // Clear the input string
        operatorChar = key;                  // Store the operator
        isSecondNumber = true;               // Set the flag to true
        lcd.print(" ");                      // Print a space on the LCD
        lcd.print(key);                      // Print the operator on the LCD
        lcd.print(" ");                      // Print a space on the LCD
      }
    } else if (key == '#') {
      // If the key is the equals sign
      if (isSecondNumber && inputString.length() > 0) {
        secondNumber = inputString.toFloat(); // Convert input string to float
        inputString = "";                     // Clear the input string
        float result = 0;                     // Variable to store the result

        // Perform the calculation based on the operator
        switch (operatorChar) {
          case '+': result = firstNumber + secondNumber; break; // Addition
          case '-': result = firstNumber - secondNumber; break; // Subtraction
          case '*': result = firstNumber * secondNumber; break; // Multiplication
          case '/': 
            if (secondNumber != 0) {
              result = firstNumber / secondNumber; // Division
            } else {
              lcd.clear();
              lcd.print("Error: Div by 0"); // Display error for division by zero
              return;
            }
            break;
        }
        lcd.print("= ");       // Print equals sign on the LCD
        lcd.print(result, 4);  // Print the result with up to 4 decimal places
        firstNumber = 0;       // Reset the first number
        secondNumber = 0;      // Reset the second number
        isSecondNumber = false;// Reset the flag
      }
    } else if (key == 'c') {
      // If the key is clear (reset)
      inputString = "";         // Clear the input string
      firstNumber = 0;          // Reset the first number
      secondNumber = 0;         // Reset the second number
      isSecondNumber = false;   // Reset the flag
      lcd.clear();              // Clear the LCD
      lcd.print(" ");       // Print "Clear" on the LCD
    }
  }
}

void displayDate() {
  DateTime now = rtc.now(); // Get the current date and time

  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(now.year());
  lcd.print('/');
  lcd.print(now.month());
  lcd.print('/');
  lcd.print(now.day());

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(now.hour());
  lcd.print(':');
  lcd.print(now.minute());
  lcd.print(':');
  lcd.print(now.second());

  delay(1000); // Update the date and time every second
}
