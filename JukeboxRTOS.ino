/**
 * Jukebox interface for ARDUINO MEGA.
**/

#include <Arduino_FreeRTOS.h>

TaskHandle_t taskGetStatusLetterSwitchHandle;
TaskHandle_t taskGetStatusNumberSwitchHandle;
TaskHandle_t taskGetLetterHandle;
TaskHandle_t taskGetNumberHandle;
TaskHandle_t taskTransmitLetterAndNumberHandle;

#include <ctype.h>
// Relay to control Soleonid of jukebox, to obtain the real CLACK ;-)
const int RELAY = 5;
// any LETTER has been pushed
const int LETTERS = 52;
// any number has been pushed
const int NUMBERS = 53;
const int tasksDelay = 500;

boolean letterPressed = false;
boolean numberPressed = false;
boolean songTransmitted = false;
char letter = '?';
char number = '?';
boolean debug = false;
boolean blnDelay = true;
boolean simulate = false;

// Store the data about a button/GPIO pin
typedef struct {
  // which GPIO pin is this button hooked up to
  unsigned char pin;
  // which number/letter does this button represent
  char label;
} Button;

// These are the buttons in use
Button buttons[] = {
    {22,'A'}, //  0 tan     / brown
    {23,'B'}, //  1 red     / brown
    {24,'C'}, //  2 orange  / brown
    {25,'D'}, //  3 yellow  / brown
    {26,'E'},
    {27,'F'},
    {28,'G'},
    {29,'H'},
    {30,'J'},
    {31,'K'}, //  pink    / brown
    {32,'L'},
    {33,'M'}, //  red     / black
    {34,'N'}, //  orange  / black
    {35,'P'},
    {36,'Q'},
    {37,'R'},
    {38,'S'},
    {39,'T'},
    {40,'U'},
    {41,'V'}, // tan      / black
    {42,'1'}, // 20 grey
    {43,'2'},
    {44,'3'},
    {45,'4'},
    {46,'5'},
    {47,'6'},
    {48,'7'},
    {49,'8'},
    {50,'9'},
    {51,'0'},
};

// how many buttons there are
#define NUM_BUTTONS (sizeof(buttons) / sizeof(Button))

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  if (debug) Serial.print("Number of buttons :");
  if (debug) Serial.println (NUM_BUTTONS);
  // set up all the buttons for input with pullup enabled
  for (unsigned i = 0; i < NUM_BUTTONS; ++i) {
    const Button & btn(buttons[i]);
    pinMode (btn.pin, INPUT_PULLUP);
  }

  pinMode     (RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  pinMode     (LETTERS, INPUT_PULLUP);
  pinMode     (NUMBERS, INPUT_PULLUP);

  // Now set up 5 Tasks to run independently.
  // Task to check if a letter has been pressed  
  xTaskCreate(
    taskGetStatusLetterSwitch
    ,  "taskGetStatusLetterSwitch"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskGetStatusLetterSwitchHandle );

  // Task to check if a number has been pressed
  xTaskCreate(
    taskGetStatusNumberSwitch
    ,  "taskGetStatusNumberSwitch"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskGetStatusNumberSwitchHandle );

  // Task to read the letter that has been pressed
  xTaskCreate(
    taskGetLetter
    ,  "taskGetLetter"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskGetLetterHandle );

  // Task to read the number that has been pressed
  xTaskCreate(
    taskGetNumber
    ,  "taskGetNumber"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskGetNumberHandle );

  // Task to transmit the letter and number to Raspberry Pi
  xTaskCreate(
    taskTransmitLetterAndNumber
    ,  "taskTransmitLetterAndNumber"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &taskTransmitLetterAndNumberHandle );

  // Now the Task scheduler, which takes over control of scheduling individual Tasks, is automatically started.
  //vTaskSuspend(taskGetStatusLetterSwitchHandle);
  vTaskSuspend(taskGetStatusNumberSwitchHandle);
  vTaskSuspend(taskGetLetterHandle);
  vTaskSuspend(taskGetNumberHandle);
  vTaskSuspend(taskTransmitLetterAndNumberHandle);
}

#define rtDelay(v) {vTaskDelay(v/15);} // delay in ms

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
/* Check if any of the 20 letters has been pressed */
void taskGetStatusLetterSwitch( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  //vTaskSuspend(taskGetStatusLetterSwitchHandle);

  while (true) // A Task shall never return or exit. 
  {
    Serial.println("Checking if letter has been pressed.");
    if (blnDelay) rtDelay(tasksDelay);
    if ((digitalRead(LETTERS) == HIGH) && (letterPressed == false)) {
      if (debug) Serial.println("A letter has been pressed, relay activated.");
      digitalWrite (RELAY, HIGH);
      letterPressed = true;
      songTransmitted = false;
      vTaskResume(taskGetStatusNumberSwitchHandle);
      vTaskSuspend(taskGetStatusLetterSwitchHandle);
    }
    if ((digitalRead(LETTERS) == LOW) && (letterPressed == false)) {
      if (debug) Serial.println("Check if a letter has been pressed.");
    }
  }
}
/* Check if any of the 10 numbers has been pressed */
void taskGetStatusNumberSwitch( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  while (true) // A Task shall never return or exit. 
  {
    Serial.println("Checking if number has been pressed.");
    //if (blnDelay) rtDelay(tasksDelay);
    if ((digitalRead(NUMBERS) == HIGH)) {
      if (debug) Serial.println("A number has been pressed, relay activated.");
      numberPressed = true;
      vTaskResume(taskGetLetterHandle);
      vTaskSuspend(taskGetStatusNumberSwitchHandle);
    }
    if ((digitalRead(NUMBERS) == LOW) && (numberPressed == false)) {
      if (debug) Serial.println("Check if a number has been pressed.");
    }
  }
}
/* Get the pressed (selected) letter */
void taskGetLetter( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  while (true) // A Task shall never return or exit.
  {
    Serial.println("Reading pressed letter.");
    if (debug) Serial.println("Read pressed letter.");
    if (blnDelay) rtDelay(tasksDelay);
    for (unsigned i = 0; i <= 19; ++i)  {
      const Button & btn(buttons[i]);
      //if (!isdigit   (buttons[i].label)) {
      Serial.print("I:");Serial.print(i);Serial.print("Letter:");Serial.println(letter);
      if (letter == '?') {
        if (digitalRead(btn.pin) == LOW  ) {
          letter = btn.label;
          vTaskResume(taskGetNumberHandle);
          vTaskSuspend(taskGetLetterHandle);
        }
      }
    }
  }
}
/* Get the pressed (selected) number */
void taskGetNumber( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  while (true) // A Task shall never return or exit.
  {
    Serial.println("Reading pressed number.");
    if (debug) Serial.println("Read pressed number.");
    if (blnDelay) rtDelay(tasksDelay);
    for (unsigned i = 20; i < NUM_BUTTONS; ++i)  {
      const Button & btn(buttons[i]);
      //if (isdigit   (buttons[i].label)) {
      //Serial.print("I:");Serial.print(i);Serial.print("Number:");Serial.println(number);
      if (number == '?') {
        if (digitalRead(btn.pin) == LOW  ) {
          number = btn.label;
          vTaskResume(taskTransmitLetterAndNumberHandle);
          vTaskSuspend(taskGetNumberHandle);
        }
      }
    }
  }
}
/* Transmit the selected Letter-Number code to the Raspberry Pi (or PC or ..*/
void taskTransmitLetterAndNumber( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  while(true) // A Task shall never return or exit. 
  {
    if (debug) Serial.println("Transmit letter and number");
    if (debug) Serial.println("");
    // Simulate pressed keys
    if (simulate) {
      Serial.print("CODE:");
      Serial.print('A');
      Serial.print('-');
      Serial.println('0');
    }
    // If real keys have been pressed, send them to Raspberry Pi
    if ((letter != '?') && (number != '?')){
      // now print out the letter-space-number sequence for example CODE:A-8
      Serial.print("CODE:");
      Serial.print(letter);
      Serial.print('-');
      Serial.println(number);
    }
    letterPressed = false;
    numberPressed = false;
    letter = '?';
    number = '?';
    digitalWrite (RELAY, LOW);
    vTaskResume(taskGetStatusLetterSwitchHandle);
    vTaskSuspend(taskTransmitLetterAndNumberHandle);
  }
}
