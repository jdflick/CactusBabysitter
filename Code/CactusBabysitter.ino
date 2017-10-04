// Cactus Babysitter
// Alex McMullen 
// 10-4-17

// NOTES
// I think the RGB LED color setting code is out of order
// it should be (RED, GREEN, BLUE)
// but it actually goes (BLUE, RED, GREEN) for whatever reason
// I'm not going to change it at the moment as I am away from my system to test

// OVERALL FUNCTION
// 1 light fixture that turns on at a specfic hour and off at a specific hour daily
// 1 pump that turns on weekly
// 2 RGB LEDs that are based on temperature ranges
// 4 buttons to change variables (light on time, light off time, milliliters, desired temp)

// COMPONENTS
// 1 Arduino Uno
// 1 LCD screen to display variables
// 1 internal clock to keep time independently
// 2 relays (light and pump)

// the specific devices I used and wiring strategies will be on my GitHub:
// github.com/AlexanderTheDecent/CactusBabysitter

// feel free to change the code to fit your needs, this was just for my specific build

//we need these libraries
  #include <Wire.h>
  #include <DS3231.h>
  #include <LiquidCrystal_I2C.h>

//initialize the DS3231 clock using the hardware interface
  DS3231  rtc(SDA, SCL);

//set the LCD address to 0x27 for a 20 char and 4 line display
  LiquidCrystal_I2C lcd(0x27, 20, 4);

//pin setup
//non sequential pin order is due to differences in pin output types
  const int tempLED1RedPin = 3;
  const int tempLED1GreenPin = 5;
  const int tempLED1BluePin = 6;
  const int tempLED2RedPin = 9;
  const int tempLED2GreenPin = 10;
  const int tempLED2BluePin = 11;
  const int relayPin = 7;
  const int pumpPin = 8;
  const int timeOnButtonPin = 13;
  const int timeOffButtonPin = 2;
  const int waterButtonPin = 4;
  const int tempButtonPin = 12;

  //defining light variables
    int lightHourOn = 9; //lights go on at 9am
    int lightHourOff = 21; //lights go off at 9pm
    boolean lightsOn = false;
 
  //code runs by watering on a given weekday, could be modded to water as often as needed
  //these variables don't change with buttons so you may have to change them here
    String wateringDay = "Sunday"; //watering every Sunday
    int wateringHour = 12; //at noon
  
  //defining water variables
    int milliliters = 0;
    int waterTime = milliliters*500; //this is just desired mL / ~6mL/sec flow * 3 pots * 1000ms/sec
    boolean gaveWater = false;
  
  //defining temp varaibles
  float desiredTemp = 75; //optimal plant temp in ºF
  float tempC = 0;
  float tempF = 0;
  
  //starting button statuses
  int timeOnButtonCurrentState = 0;     
  int timeOnButtonLastState = 0;
  int timeOffButtonCurrentState = 0;     
  int timeOffButtonLastState = 0;
  int waterButtonCurrentState = 0;     
  int waterButtonLastState = 0;
  int tempButtonCurrentState = 0;     
  int tempButtonLastState = 0;

void setup(){
  
  //initialize the rtc
  rtc.begin();
  
  //the following lines can be uncommented to set the date and time
  //rtc.setDOW(WEDNESDAY);      // Set Day-of-Week to WEDNESDAY
  //rtc.setTime(21, 47, 0);     // Set the time to 00:00:00 (24hr format)
  //rtc.setDate(9, 8, 2017);   // Set the date to January 1st, 2017
  
  //initialize the lcd
  lcd.init();
  
  //open the backlight
  lcd.backlight(); 
  
  //setup serial connection at 115200 baud
  Serial.begin(115200);
 
  //setting inputs
  pinMode(timeOnButtonPin, INPUT);
  pinMode(timeOffButtonPin, INPUT);
  pinMode(waterButtonPin, INPUT);
  pinMode(tempButtonPin, INPUT);
 
  //setting outputs
  pinMode(tempLED1RedPin, OUTPUT);
  pinMode(tempLED1GreenPin, OUTPUT);
  pinMode(tempLED1BluePin, OUTPUT);
  pinMode(tempLED2RedPin, OUTPUT);
  pinMode(tempLED2GreenPin, OUTPUT);
  pinMode(tempLED2BluePin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);

}


void loop(){
    
    //getting day and hour strings
    String currentDay = rtc.getDOWStr();
    String currentTimeString = rtc.getTimeStr();
    String currentHourString = currentTimeString;
    currentHourString.remove(2); 
    
    //converting hour strings into int
    int currentHour = currentHourString.toInt();
    
    //getting the temp in Celsius
    tempC = rtc.getTemp();
    
    //converting to Fahrenheit
    tempF = (tempC * 9/5) + 32;
    
    //reading timeOnButton
    timeOnButtonCurrentState = digitalRead(timeOnButtonPin);
    //if button is pushed in and also different from last read
    if(timeOnButtonCurrentState != timeOnButtonLastState && timeOnButtonCurrentState == HIGH){
      //up the time one hour or loop it back
      if(lightHourOn != 23){
        lightHourOn = lightHourOn + 1;
      }
      else{
         lightHourOn = 0;
      }
    }
    //set our state to our last state for the next check
    timeOnButtonLastState = timeOnButtonCurrentState;
    
    //reading timeOffButton
    timeOffButtonCurrentState = digitalRead(timeOffButtonPin);
    //if button is pushed in and also different from last read
    if(timeOffButtonCurrentState != timeOffButtonLastState && timeOffButtonCurrentState == HIGH){
      //up the time one hour or loop it back
      if(lightHourOff != 23){
        lightHourOff = lightHourOff + 1;
      }
      else{
         lightHourOff = 0;
      }
    }
    //set our state to our last state for the next check
    timeOffButtonLastState = timeOffButtonCurrentState;
    
    //reading waterButton
    waterButtonCurrentState = digitalRead(waterButtonPin);
    //if button is pushed in and also different from last read
    if(waterButtonCurrentState != waterButtonLastState && waterButtonCurrentState == HIGH){
      //up the water level 10ml
      if(milliliters != 200){
        milliliters = milliliters + 10;
      }
      else{
         milliliters = 0;
      }
    }
    //set our state to our last state for the next check
    waterButtonLastState = waterButtonCurrentState;
    
    //reading tempButton
    tempButtonCurrentState = digitalRead(tempButtonPin);
    //if button is pushed in and also different from last read
    if(tempButtonCurrentState != tempButtonLastState && tempButtonCurrentState == HIGH){
      //up the temp 5º
      if(desiredTemp != 100){
        desiredTemp = desiredTemp + 5;
      }
      else{
         desiredTemp = 50;
      }
    }
    //set our state to our last state for the next check
    tempButtonLastState = tempButtonCurrentState;
    
    //checking lights
    if(currentHour == lightHourOn && lightsOn == false){
      digitalWrite(relayPin, HIGH);
      lightsOn = true;
    }
    
    //resets the lights when lightHourOff comes
    else if(currentHour == lightHourOff && lightsOn == true){
      digitalWrite(relayPin, LOW);
      lightsOn = false;
    }
    
    //checking water
    if(currentDay == wateringDay && currentHour == wateringHour && gaveWater == false){
      digitalWrite(pumpPin, HIGH);
      //water for the time calculated based on the input mL (ratio is specific to my pump system)
      delay(waterTime);
      digitalWrite(pumpPin, LOW);
      gaveWater = true;
    }
    
    //resets the watering boolean the next hour
    else if(currentHour != wateringHour && gaveWater == true){
      gaveWater = false;
    }
    
    //checking if temperature is in tolerable 5º range 
    if(tempF >= (desiredTemp-5.0) && tempF < (desiredTemp+5.0)){
      //setting two green LEDs
      setColor1(0, 0, 20);
      setColor2(0, 0, 20);
    }
    
    //checking if temperature is 5-10º colder than desired
    else if(tempF > desiredTemp-10 && tempF <= desiredTemp-5){
      //setting one blue and one green LED
      setColor1(50, 0, 0);
      setColor2(0, 0, 20);
    }
    
    //checking if temperature is 5-10º hotter than desired
    else if(tempF < desiredTemp+10 && tempF >= desiredTemp+5){
      //setting one green and one red LED
      setColor1(0, 0, 20);
      setColor2(0, 100, 0);
    }

    //checking if temperature is 10º colder than desired or more
    else if(tempF <= desiredTemp-10){
      //setting two blue LEDs
      setColor1(50, 0, 0);
      setColor2(50, 0, 0);
    }
    
    //checking if temperature is 10º hotter than desired or more
    else if(tempF >= desiredTemp+10){
      //setting two red LEDs
      setColor1(0, 100, 0);
      setColor2(0, 100, 0);
    }
   
   //first line
   lcd.setCursor(0,0);
   lcd.print(lightHourOn);
   lcd.print(":00 lights on.....");
   
   //second line
   lcd.setCursor(0,1); 
   lcd.print(lightHourOff);
   lcd.print(":00 lights off....");
   
   //third line
   lcd.setCursor(0,2); 
   lcd.print(milliliters);
   lcd.print("mL per plant.....");
   
   //fourth line
   lcd.setCursor(0,3); 
   lcd.print(desiredTemp);
   //this is how you get the degrees (º) symbol
   lcd.print("F ideal temp..");
   
    //sending data to serial for debugging
    Serial.println("TIME DEBUG");
    Serial.print(rtc.getDOWStr());
    Serial.print(" ");
    Serial.print(rtc.getDateStr());
    Serial.print(" -- ");
    Serial.println(rtc.getTimeStr());    
    Serial.print("currentDay: ");
    Serial.println(currentDay);
    Serial.print("currentTime: ");
    Serial.println(currentTimeString);
    Serial.print("currentHour (from String): ");
    Serial.println(currentHourString);
    Serial.print("currentHour (from int): ");
    Serial.println(currentHour);
    Serial.println(" ");
    
    Serial.println("WATER DEBUG");
    Serial.print("wateringDay: ");
    Serial.println(wateringDay);
    Serial.print("wateringHour: ");
    Serial.println(wateringHour);
    Serial.print("milliliters: ");
    Serial.println(milliliters);
    Serial.print("waterTime: ");
    Serial.println(waterTime);
    Serial.print("gaveWater: ");
    Serial.println(gaveWater);
    Serial.println(" ");
    
    Serial.println("TEMP DEBUG");
    Serial.print("tempC: ");
    Serial.println(tempC);
    Serial.print("tempF: ");
    Serial.println(tempF);
    Serial.print("desiredTemp: ");
    Serial.println(desiredTemp);
    Serial.println(" ");
    
    Serial.println("LIGHT DEBUG");
    Serial.print("lightHourOn: ");
    Serial.println(lightHourOn);
    Serial.print("lightHourOff: ");
    Serial.println(lightHourOff);
    Serial.print("lightsOn: ");
    Serial.println(lightsOn);
    Serial.println(" ");
 
  
}
 
  //just a handy tool we use to help make the LED stuff easier 
  void setColor1(int red, int green, int blue){
    
  #ifdef COMMON_ANODE
    red = 255-red;
    green = 255-green;
    blue = 255-blue;
  #endif
  analogWrite(tempLED1RedPin, red);
  analogWrite(tempLED1GreenPin, green);
  analogWrite(tempLED1BluePin, blue);  
  
  }
  
  
  //just a handy tool we use to help make the LED stuff easier 
  void setColor2(int red, int green, int blue){
    
  #ifdef COMMON_ANODE
    red = 255-red;
    green = 255-green;
    blue = 255-blue;
  #endif
  analogWrite(tempLED2RedPin, red);
  analogWrite(tempLED2GreenPin, green);
  analogWrite(tempLED2BluePin, blue); 
  
  }
 

