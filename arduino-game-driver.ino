
struct plantData { //creates struct with all the information on the plants
 int waterTime;
 int spreadTime;
 bool alive;
 volatile bool needsWater;
 volatile bool hasVirus;
 unsigned long whenInfected;
 volatile unsigned long waterChange;
};


struct status_buffer { //creates a struct to store info to pass over serial
 unsigned short game_time;
 unsigned short plants_rem;
 unsigned short presses;
};


struct plantData plants[9]; //create array of plant structs
const int pins[4][9] = {{53, 51, 49, 47, 45, 43, 41, 39, 37}, {52, 50, 48, 46, 44, 42, 40, 38, 36}, {35, 34, 33, 32, 31, 30, 29, 28, 27}}; //array of pins for easy reference in loops
unsigned long virustime; //time before virus starts
bool infected; //is there an infection present
bool danger_temp; //whether or not the plants are at a dangerous temperature
int temp; //stores temporary random values
double heat; //how hot the plants are from 0-100
unsigned long previousTime; //last time heat was updated
unsigned long temp_timer; //timer for how long plants have been too hot or too cold
const int potPin = A0; //potentiometer pin
const int hot_pin = 24; //pin for heat led
const int cold_pin = 26; //pin for cold led
int total_alive; //plants left alive
unsigned short press_count; //counts how many times watering buttons are pressed
int waterDifficulty, spreadDifficulty, waterRandomDifficulty, waterDeathSpeed, virusSpeedDifficulty;


void setup() {
 Serial.begin(9600); //set baud rate
 for (int i = 27; i < 54; i++) { //initializes all the leds as outputs
   pinMode(i, OUTPUT);
 }
 attachInterrupt(digitalPinToInterrupt(2), isr_button_1, FALLING); //interrupts for buttons
 attachInterrupt(digitalPinToInterrupt(3), isr_button_2, FALLING);
 attachInterrupt(digitalPinToInterrupt(18), isr_button_3, FALLING);
 attachInterrupt(digitalPinToInterrupt(19), isr_button_4, FALLING);
 attachInterrupt(digitalPinToInterrupt(20), isr_button_5, FALLING);
 attachInterrupt(digitalPinToInterrupt(21), isr_button_6, FALLING);
 pinMode(potPin, INPUT); //initializes potentiometer as input
 pinMode(A2, INPUT);
 pinMode(7, INPUT); //initializes buttons as inputs
 pinMode(4, INPUT);
 pinMode(5, INPUT);
 pinMode(6, INPUT);
 pinMode(2, INPUT_PULLUP); //initializes other buttons as interrupt inputs
 pinMode(3, INPUT_PULLUP);
 pinMode(18, INPUT_PULLUP);
 pinMode(19, INPUT_PULLUP);
 pinMode(20, INPUT_PULLUP);
 pinMode(21, INPUT_PULLUP);
 pinMode(hot_pin, OUTPUT);
 pinMode(cold_pin, OUTPUT);
 double volt = 5 - analogRead(A2) * 0.004882814; //convert potentiometer input to voltage
 if (volt > 2 && volt < 3 ) { //if the difficulty voltage is between 2 and 3 V set difficulty to hardest
   waterDifficulty = 3000; //time until watering is minimum 3 seconds
   spreadDifficulty = 2000; //virus spreads after 2 seconds
   waterRandomDifficulty = 5000; //maximum water time is 8 seconds
   waterDeathSpeed = 3000; //plants die in 3 seconds without water
   virusSpeedDifficulty = 3000; //virus starts after minimum 3 seconds
 }
 else if (volt > 3) { //if difficulty voltage is above 3 set difficulty to medium
   waterDifficulty = 4000; //same as above w/ different numbers
   spreadDifficulty = 3000;
   waterRandomDifficulty = 8000;
   waterDeathSpeed = 4000;
   virusSpeedDifficulty = 5000;
 } else { //otherwise set difficulty to easy
   waterDifficulty = 5000; //same as above with different numbers
   spreadDifficulty = 4000;
   waterRandomDifficulty = 12000;
   waterDeathSpeed = 5000;
   virusSpeedDifficulty = 8000;
 }
 randomSeed(analogRead(A4)); //seed random to be differentevery time based on random analog port
 for (int i = 0; i < 9; i++) {
   plants[i].waterTime = waterDifficulty + random(0, 20000); //time until watering is based on watering difficulty plus 0-20 seconds
   plants[i].spreadTime = spreadDifficulty; //sets time until virus spreads to plant to spread difficulty
   plants[i].alive = true; //sets plant as alive
   plants[i].needsWater = false; //sets plant as not needing water
   plants[i].hasVirus = false; //sets plant as not having virus
   plants[i].waterChange = 0; //sets the last water changed time to 0
   digitalWrite(pins[1][i], HIGH); // turns off all watering lights
   digitalWrite(pins[2][i], HIGH); // turns off all infection lights
   digitalWrite(pins[0][i], LOW); // turns on healthy lights
 }
 heat = 50; //heat starts out in the middle
 previousTime = 0; //time hasn't been updated
 danger_temp = false; //plants are not in danger
 virustime = 15000; // sets time until virus to random numbr 2 - 10
 infected = false; // sets infected state
 temp_timer = 0; //plants are not in danger so it's set to 0
 press_count = 0; //watering buttons have been pressed 0 times
 digitalWrite(8, HIGH); //turns on reset button light
}


void loop() {
 if (digitalRead(4) == HIGH) { //checks for button input for buttons that couldn't use interrupts
   read_button(6);
 }
 if (digitalRead(5) == HIGH) {
   read_button(7);
 }
 if (digitalRead(6) == HIGH) {
   read_button(8);
 }
 if (digitalRead(7) == HIGH) { //checks for input from lever
   read_lever();
 }
 for (int i = 0; i < 9; i++) {
   if (millis() - plants[i].waterChange >= plants[i].waterTime //time since last update exceeds watering time
       && plants[i].needsWater == false //watering light is not on
       && plants[i].alive == true //plant is alive
       && plants[i].hasVirus == false) { //plant is not infected
     digitalWrite(pins[1][i], LOW); //turn on watering led
     digitalWrite(pins[0][i], HIGH); //turn off healthy led
     plants[i].waterChange = millis(); //update led time to current time
     plants[i].needsWater = true; //needs water
   }
 }
 for (int i = 0; i < 9; i++) {
   if ((millis() - plants[i].waterChange) >= waterDeathSpeed    // time since last watered update exceeds death speed threshold
       && plants[i].needsWater == true // needs water
       && plants[i].alive == true) {   // plant is alive
     plants[i].alive = false; //kill it
     digitalWrite(pins[1][i], HIGH); //turn off water led
     plants[i].needsWater = false; //turn off flag signifying led is on
   }
 }
 if (!infected //there hasn't already been an infection
     && millis() >= virustime) { //time since last update exceeds virus time
   temp = random(1, 10); //chooses a random plant out of the 9
   if (plants[temp].needsWater == false //if the plant doesn't already need watering
       && plants[temp].alive == true) { //the plant is alive
     digitalWrite(pins[2][temp], LOW); //turn on the red light to signify infection
     digitalWrite(pins[0][temp], HIGH); //turn healthy led off
     plants[temp].hasVirus = true; //plant is infected
     plants[temp].whenInfected = millis(); //sets the time for the start of the infection in the current plant
     infected = true; //the plants are now in an infected state
   }
 }
 if (infected) { //if an infection is present
   for (int i = 0; i < 9; i++) {
     if (plants[i].hasVirus == true //if the current plant is infected
         && plants[i].alive == true) { //and the current plant is alive
       if (i - 1 >= 0 && i - 1 != 2 && i - 1 != 5) {//the array won't underflow
         spreadDisease(i - 1, i); //try to spread the disease to the left plant
       }
       if (i + 1 <= 8 && i + 1 != 3 && i + 1 != 6) {//the array won't underflow
         spreadDisease(i + 1, i); //try to spread the disease to the right plant
       }
       if (i + 3 <= 8) {//the array won't overflow
         spreadDisease(i + 3, i); //try to spread the disease to the plant below
       }
       if (i - 3 >= 0) {//the array won't underflow
         spreadDisease(i - 3, i); //try to spread the disease to the plant above
       }
     }
   }
   for (int i = 0; i < 9; i++) {
     if (millis() - plants[i].whenInfected >= 6000 //and a plant has been infected for more than 6 seconds
         && plants[i].hasVirus == true //and the led is on
         && plants[i].alive == true) { //and the plant is alive
       plants[i].alive = false; //kill the plant
       plants[i].hasVirus = false; //flag to show led is on no longer active
       digitalWrite(pins[2][i], HIGH); //turn off the led light
     }
   }
   if (!diseasePresent()) {   //if no plants are infected
     infected = false; //turn off infection flag
     virustime = millis() + 8000; //set the virus to start again at a random time
   }
 }
 if (millis() - previousTime >= 200) { //if the time since the last update is at least 200 ms
   changeTemp(); //change the temperature
 }
 total_alive = 9; //sets total alive plants to 9
 for (int i = 0; i < 9; i++) {
   if (!plants[i].alive) { //subtract a plant from the alive count for each plant that isn't alive
     total_alive--;
   }
 }
 if (total_alive < 6) { //if less than 6 plants are alive
   finish(); //end the game
 }
}


void spreadDisease(int plantNum, int rootPlant) { //spreads disease to surrounding plants
 if (plants[plantNum].alive == true //if the plant is alive
     && plants[plantNum].needsWater == false //and the plant doesn't need water
     && plants[plantNum].hasVirus == false //and the plant isn't already infected
     && millis() - plants[rootPlant].whenInfected >= plants[plantNum].spreadTime) { //and the plant's resistance period is over
   digitalWrite(pins[2][plantNum], LOW); //turn red led on
   digitalWrite(pins[0][plantNum], HIGH); //turn healthy led off
   plants[plantNum].hasVirus = true; //turn flag on for infection
   plants[plantNum].whenInfected = millis(); //set the time of infection of current plant
 }
}


bool diseasePresent() { //checks if a disease is still present
 for (int i = 0; i < 9; i++) {
   if (plants[i].hasVirus == true) { //if a disease is still present return true
     return true;
   }
 }
 return false; //otherwise return false
}



void changeTemp() { //controls temperature-based operations
 double volt = 5 - analogRead(potPin) * 0.004882814; //convert potentiometer input to voltage
 if (volt < 2.5) { //if the potentiometer is turned to less than halfway
   heat = heat - 3.5 + volt; //heat goes down faster the farther the potentiometer is turned
   previousTime = millis(); //updates update time
   if (heat < 0) { //if the heat goes below 0
     heat = 0; //the heat is set back to 0
   }
 } else { //if the potentiometer is turned to more than halfway
   heat = heat + volt - 1.5; //heat goes up faster the farther the potentiometer is turned
   previousTime = millis(); //updates update time
   if (heat > 100) { //if the heat goes above 100
     heat = 100; //the heat is set back to 100
   }
 }
 if (heat > 90 || heat < 10) { //if the heat is too high or too low
   if (heat > 90) { //if it's above 90
     digitalWrite(hot_pin, LOW); //turn on the led signifying it's too hot
   } else { //otherwise
     digitalWrite(cold_pin, LOW); //turn on the led signifying it's too cold
   }
   if (!danger_temp) { //if this is the first time it's executing this
     temp_timer = millis() + 5000; //the time before a plant dies is 5 seconds
     danger_temp = true; //it'll no longer run through this
   }
 } else { //if the heat is not too high or too low
   digitalWrite(hot_pin, HIGH); //turn off heat led
   digitalWrite(cold_pin, HIGH); //turn off cold led
   temp_timer = 0; //reset temperature timer
   danger_temp = false; //will run through the temp_timer setter again
 }
 if (temp_timer != 0 && millis() >= temp_timer) { //if the temp_timer is not 0 and the time until a plant overheats/ freezes has passed
   temp = random(0, 9); //pick a random plant
   if (plants[temp].alive == true) { //if the plant is alive
     plants[temp].alive = false; //kill it
     digitalWrite(pins[0][temp], HIGH); //turn off all colors of the led
     digitalWrite(pins[1][temp], HIGH);
     digitalWrite(pins[2][temp], HIGH);
     temp_timer = millis() + 2000; //time until next plant dies is 2 seconds
   }
 }
}
void isr_button_1() {
 if (plants[0].needsWater == true //if the plant needs water
     && plants[0].alive == true) { //and the plant is alive
   digitalWrite(pins[1][0], HIGH); //turn off water light
   digitalWrite(pins[0][0], LOW); //turn back on green led
   plants[0].waterChange = millis(); //updates time value for led to be current program time
   plants[0].needsWater = false; //changes needsWater to reflect off state of led
   plants[0].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5)); //time until next watering determined by difficulty + random
   if (plants[0].waterTime < 1000) { //if waterTime is below 1 second
     plants[0].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}
void isr_button_2() {
 if (plants[1].needsWater == true //if the plant needs water
     && plants[1].alive == true) { //and the plant is alive
   digitalWrite(pins[1][1], HIGH); //turn off water light
   digitalWrite(pins[0][1], LOW); //turn back on green led
   plants[1].waterChange = millis(); //updates time value for led to be current program time
   plants[1].needsWater = false; //changes needsWater to reflect off state of led
   plants[1].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5); //time until next watering determined by difficulty + random
   if (plants[1].waterTime < 1000) { //if waterTime is below 1 second
     plants[1].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}
void isr_button_3() {
 if (plants[2].needsWater == true //if the plant needs water
     && plants[2].alive == true) { //and the plant is alive
   digitalWrite(pins[1][2], HIGH); //turn off water light
   digitalWrite(pins[0][2], LOW); //turn back on green led
   plants[2].waterChange = millis(); //updates time value for led to be current program time
   plants[2].needsWater = false; //changes needsWater to reflect off state of led
   plants[2].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5); //time until next watering determined by difficulty + random
   if (plants[0].waterTime < 1000) { //if waterTime is below 1 second
     plants[0].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}g
void isr_button_4() {
 if (plants[3].needsWater == true //if the plant needs water
     && plants[3].alive == true) { //and the plant is alive
   digitalWrite(pins[1][3], HIGH); //turn off water light
   digitalWrite(pins[0][3], LOW); //turn back on green led
   plants[3].waterChange = millis(); //updates time value for led to be current program time
   plants[3].needsWater = false; //changes needsWater to reflect off state of led
   plants[3].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5); //time until next watering determined by difficulty + random
   if (plants[0].waterTime < 1000) { //if waterTime is below 1 second
     plants[0].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}
void isr_button_5() {
 if (plants[4].needsWater == true //if the plant needs water
     && plants[4].alive == true) { //and the plant is alive
   digitalWrite(pins[1][4], HIGH); //turn off water light
   digitalWrite(pins[0][4], LOW); //turn back on green led
   plants[4].waterChange = millis(); //updates time value for led to be current program time
   plants[4].needsWater = false; //changes needsWater to reflect off state of led
   plants[4].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5); //time until next watering determined by difficulty + random
   if (plants[0].waterTime < 1000) { //if waterTime is below 1 second
     plants[0].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}
void isr_button_6() {
 if (plants[5].needsWater == true //if the plant needs water
     && plants[5].alive == true) { //and the plant is alive
   digitalWrite(pins[1][5], HIGH); //turn off water light
   digitalWrite(pins[0][5], LOW); //turn back on green led
   plants[5].waterChange = millis(); //updates time value for led to be current program time
   plants[5].needsWater = false; //changes needsWater to reflect off state of led
   plants[5].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5); //time until next watering determined by difficulty + random
   if (plants[0].waterTime < 1000) { //if waterTime is below 1 second
     plants[0].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}


void read_button(int i) {
 if (plants[i].needsWater == true //if the plant needs water
     && plants[i].alive == true) { //and the plant is alive
   digitalWrite(pins[1][i], HIGH); //turn off water light
   digitalWrite(pins[0][i], LOW); //turn back on green led
   plants[i].waterChange = millis(); //updates time value for led to be current program time
   plants[i].needsWater = false; //changes needsWater to reflect off state of led
   plants[i].waterTime = waterDifficulty + random(0, waterRandomDifficulty) - (press_count * 5); //time until next watering determined by difficulty + random
   if (plants[0].waterTime < 1000) { //if waterTime is below 1 second
     plants[0].waterTime = 1000; //set it to 1 second
   }
   press_count++; //add 1 to button press count
 }
}


void read_lever() {
 for (int i = 0; i < 9; i++) { // loops through the 9 plants
   if (plants[i].hasVirus == true) { // if the plant is infected
     digitalWrite(pins[2][i], HIGH); // turn off infection led
     digitalWrite(pins[0][i], LOW); // turn on healthy led
     infected = false; // infection is no longer active
     virustime = millis() + virusSpeedDifficulty - (millis() / 50); // sets time until virus starts again and gets harder as time goes on
     if (virustime < 500) { //if time until virus is under half a second
       virustime = 500; //set back to half a second
     }
     plants[i].hasVirus = false; // flag for infection is set back to off
   }
 }
}


void finish() {
 struct status_buffer st_buf; //creates buffer to pass to serial write
 st_buf.game_time = millis() / 1000; //game time transposed from milliseconds to seconds
 st_buf.plants_rem = total_alive; //adds plants remaining to the buffer
 st_buf.presses = press_count; //adds number of plants watered to the buffer
 Serial.write((unsigned char*) &st_buf, sizeof(struct status_buffer)); //writes the data address of the struct to the serial port in lieu of an array, along with the size of the struct
 while (true) { //loops so no actions can be performed until game is reset
 }
}

