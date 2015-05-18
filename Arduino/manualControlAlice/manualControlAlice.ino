#include <Smartcar.h>
#include <Smartcar_sensors.h>
#include <Wire.h>
#include <SoftwareSerial.h>

Smartcar alice;

Odometer encoder;
const int odo_pin = 19;

Sonar sonar;
const int trig_pin = 43;
const int echo_pin = 42;

volatile int counter;
boolean obstacle;
int index;
String mode;
  
void setup(){
   Serial.begin(9600);
   alice.begin();
   alice.setAutomationRotationSpeed(100);
   sonar.attach(trig_pin, echo_pin);
   encoder.attach(odo_pin);
}

void loop(){
  Serial.println("Car waiting");
  while(true){    
    if(Serial.available() > 0){
      if (Serial.peek() == '$'){
        mode = "Manual";
        break;
      }else{
        mode = "Auto";
        reset();
        break;
      }
    }
  }
  if(mode.equals("Auto")){
    autoMode();
  }else if(mode.equals("Manual")){
    manualMode();
  }   
}

void manualMode() {
  Serial.println("Manual mode entered, waiting for manual control.");
  String instr;
  
  while(true){
    if(Serial.available() > 0){
        
      if(Serial.peek() == '@'){
          Serial.readStringUntil('@');
          Serial.println("Manual mode end, turn back into idle mode.");
          break;
        }else if(Serial.peek() == '$'){
          Serial.readStringUntil('$');
          instr = Serial.readStringUntil('*');
        }else{
          Serial.read();
        }
      }
      if(instr.equals("goForward")){
        Serial.println("Going forward.");
        alice.goForward();
      }else if(instr.equals("goBackward")){
        Serial.println("Going backward.");
        alice.goBackward();
      }else if(instr.equals("rotateClockwise")){
        Serial.println("Rotating clockwise.");
        alice.rotateClockwise();
      }else if(instr.equals("rotateCounterClockwise")){
        Serial.println("Rotating counterclockwise");
        alice.rotateCounterClockwise();
      }else{
        Serial.println("Stop.");
        alice.stop();
      }
    }
  }
  
void autoMode(){
  Serial.println("Auto mode entered.");
  if(Serial.available() > 0){ 
    //initialize a queue as accommodation
    int arraylength = Serial.readStringUntil('!').toInt();
    String queue[arraylength];
    int k = 0;
        
    //store instructions in queue
    while(k<arraylength){
      if(Serial.available() > 0){
        queue[k] = Serial.readStringUntil('*');
        k++;
      }
    }
    
    //interpret and excute instructions
    for(index = 0; index<arraylength && !obstacle; index++){
        String instr = "";
        String parameter = "";
        boolean spaceFound = false;
        
        for(int j = 0; j<queue[index].length(); j++){
          if(queue[index].charAt(j)==' '){
            spaceFound = true;
          }else if(spaceFound){
            parameter += queue[index].charAt(j);
          }else{
            instr += queue[index].charAt(j); 
          }
        }
      
      //excute instruction
      int value = parameter.toInt();
      
      if(instr.equals("goForward")){
          goForwardSafe(value);
      }else if(instr=="rotateClockwise"){
          alice.rotateClockwise(value);
      }else if(instr=="rotateCounterClockwise"){
          alice.rotateCounterClockwise(value);
      }
    }
  }
}

void goForwardSafe(int desiredDistance){
  encoder.begin();
  
  while(encoder.getDistance() < desiredDistance && isFrontClear()){
    alice.goForward();
  }
  
  brake();
}

boolean isFrontClear(){
  int distance = sonar.getDistance();
  
  if(distance < 25 && distance != 0){
    counter++;
  }
  
  if(counter >= 3){
    String str = "Obstacle ";
    str += index;
    Serial.println(str);
    
    obstacle = true;
    return false;
  }
  
  return true;
}

void brake(){
  alice.stop();
  delay(50);
  alice.goBackward();
  delay(75);
  alice.stop();
}

void reset(){
  counter = 0;
  obstacle = false;
}
