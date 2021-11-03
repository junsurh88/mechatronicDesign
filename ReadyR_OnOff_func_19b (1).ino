#include <Clock.h> //tone functions, allows output of square waves
#define INHIB_LFT 5 //ON-OFF left, when LOW, LFT motor inhibited   //4
#define INHIB_RGHT 4 //ON-OFF right, when LOW, RGHT motor inhibited  //5
#define DIR_LFT 6 //direction left, when HIGH, LFT motor reversed
#define DIR_RGHT 7 //direction right, when HIGH, RGHT motor reversed
#define CLK_LFT 10 //clock output left to motor driver CLK input thru 7406
#define CLK_RGHT 11 //clock output right
#define LVR_LFT 8 //bumper switch left, LO when pressed
#define LVR_RGHT 9 //bumper switch right, LO when pressed
#define PHOT_LFT 0 //phototransistor left, analog pin 1
#define PHOT_RGHT 1 //phototransistor right, analog pin 0
#define V_SWTCH 2 //vulnerability (kill) switch
//Change these two for different motor speeds
//#define FREQ_LFT 3800 //Frequency for left motor
//#define FREQ_RGHT 3800 //Frequency for right motor
#define GRAT 37 // 52  // 
#define PHOT_THRS 150
#define PHOT_DIF_Lft 22
#define PHOT_DIF 20
#define PHOT_DIF_Rgh 22
#define sped_dura 1200 // msec
//#define sighd 2
#define turn_dura 1500 
//define pp_max 256; 
//#define prev_vuln 0; 
int sighd;
  
//To disable a motor set INHIB_LFT or INHIB_RGHT to !ON_LFT or !ON_RGHT
#define ON_LFT LOW //Left motor is on when INHIB_LFT is set to this
#define ON_RGHT LOW//Right motor is on when INHIB_LFT is set to this
clock velocityL; //Clock for left motor
clock velocityR; //Clock for right motor
int FREQ_LFT; 
int FREQ_RGHT;



int PHOT_LFT_VAL;
int PHOT_RGHT_VAL;
int Phot_Diff_Left;
int pp_max; 
int vuln; 
int Vstop = 0; 
int prev_vuln = 0; // 1 means ON 0 means OFF for rover
volatile boolean enabled; 
//Determines if robot is enabled(do not change in main program)
char valu = 'X';
char prev_valu = 'Y'; // for debounce of remote switch

void setup() {
  Serial.print(" In Setup IO2S ");
  Serial.begin(9600);
  for(int ii =4; ii<=11; ii++) {
    pinMode(ii, OUTPUT);   //set up all of the inhibs, clock, etc as output
  }
  pinMode(V_SWTCH, INPUT); //setup inputs
  pinMode(LVR_LFT, INPUT);
  pinMode(LVR_RGHT, INPUT);

  int FREQval3 = 3800;
  int FREQval5 = 5400;
  if(GRAT == 52) {   // set base clock rate depending on gear ratio GRAT
    FREQ_LFT = FREQval5;
    FREQ_RGHT = FREQval5; 
    }
  else {
    FREQ_LFT = FREQval3;
    FREQ_RGHT = FREQval3;
  }    

  digitalWrite(V_SWTCH, HIGH);
  digitalWrite(LVR_LFT, HIGH); //enable internal pullup.
  digitalWrite(LVR_RGHT, HIGH);

  velocityL.begin(CLK_LFT); 
  //set up frequency output pins. don't need to do this again
  velocityR.begin(CLK_RGHT); //do not call this function in main program
  digitalWrite(INHIB_LFT, !ON_LFT); //inhibit both motors
  digitalWrite(INHIB_RGHT, !ON_RGHT);
  attachInterrupt(0, interruptroutine, CHANGE);  
  //when the V-switch is flicked to the side, call interrupt routine function
  interruptroutine();
  digitalWrite(DIR_LFT,HIGH);
  digitalWrite(DIR_RGHT, HIGH);//set direction to forwards

  //while(!photoactivate()){
  Serial.println(" Just before test remoteTactivate ");
  while(!remoteTactivate()){
    Serial.println(" In test remoteTactivate ");
    digitalWrite(INHIB_LFT, !ON_LFT); 
    digitalWrite(INHIB_RGHT, !ON_RGHT); //inhibit both motors
  }
  digitalWrite(DIR_RGHT, HIGH); // forward
  digitalWrite(DIR_LFT, HIGH); 
  
  int LightDetectReact();
  int RemoteDetectReact(); 
  delay(100);
}

void loop() {
  sighd = 1; // left  needed? 
  pp_max = 512; // debug term 
  int pp = 0;

   // frequencies for straight
  int FREQval3 = 3800;
  int FREQval5 = 5400;
 
  int FREQval3L_lo = 1600;  // 8 frequencies for curve CW CCW 
  int FREQval3R_lo = 1600;
  int FREQval5L_lo = 4400;
  int FREQval5R_lo = 4400;

  int FREQval3L_hi = 2400;
  int FREQval3R_hi = 2400;
  int FREQval5L_hi = 6600;
  int FREQval5R_hi = 6600;
  
  if (Vstop == 1) {
    digitalWrite(INHIB_LFT, !ON_LFT);
    digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop
  }
  while(enabled && pp < pp_max){
    Serial.println("IN remote while loop ");
    prev_valu = 'Y';  
    while ( Serial.available( ) ) {
      valu = Serial.read( );
      Serial.println(valu) ;
      if (valu == 'T' && prev_valu != 'T') { 
      prev_vuln = 1;
      Vstop = 0;
      prev_valu = 'T' ; 
      digitalWrite(DIR_RGHT, HIGH); // forward
      digitalWrite(DIR_LFT, HIGH);
      digitalWrite(INHIB_LFT, ON_LFT); 
      digitalWrite(INHIB_RGHT, ON_RGHT);//on on    
    } 
      if (valu == 'V') {
      Vstop = 1;
      digitalWrite(INHIB_LFT, !ON_LFT);
      digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop      
    }
      if (valu == 'T' || valu == 'V' )  {
        Serial.println("ON or OFF switched remote");
        //delay(1000) ;
      }    
      //delay(100) ;
      //valu = Serial.read( );
      if (valu == 'W' || prev_valu == 'U' ){
        digitalWrite(DIR_RGHT, LOW);
        digitalWrite(DIR_LFT, LOW); // reverse
      }
      if (valu == 'U' ) { //&& ( prev_valu == 'Y' || prev_valu == 'U' ) ) {
        Serial.println("Left BACK UP REMOTE");
        if (GRAT == 37) {
        velocityL.play(FREQval3L_lo); // reverse to the rover left
        velocityR.play(FREQval3R_hi);
        prev_valu = 'U';
        }
        if (GRAT == 52) {
        velocityL.play(FREQval5L_lo); // reverse to the rover left
        velocityR.play(FREQval5R_hi);
        prev_valu = 'U';
        }        
      }
      
      else if (valu == 'W' ) { //&& prev_valu == 'Y' || prev_valu == 'W') {
        Serial.println("Right BACK UP REMOTE");
        if (GRAT == 37) {
        velocityL.play(FREQval3L_hi); // reverse to the rover right
        velocityR.play(FREQval3R_lo); 
        prev_valu = 'W'; 
        }
        if (GRAT == 52) {
        velocityL.play(FREQval5L_hi); // reverse to the rover right
        velocityR.play(FREQval5R_lo); 
        prev_valu = 'W'; 
        }
      }   
 
      else {  
        valu = 'X'; // instead of blank... 
        Serial.println(valu) ;  
      }
      Serial.println(prev_valu) ;
    }
    
    Serial.println("out of loop remote while loop ");
    digitalWrite(DIR_RGHT, HIGH); // forward
    digitalWrite(DIR_LFT, HIGH);
    Serial.println(prev_valu) ;
    if (prev_valu == 'U' || prev_valu == 'W') {
      Serial.println("in U or W if");  
      digitalWrite(INHIB_LFT, !ON_LFT);
      digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop after letter U, W over  
      delay(100);
      digitalWrite(INHIB_LFT, ON_LFT); 
      digitalWrite(INHIB_RGHT, ON_RGHT);//go  go  
      digitalWrite(DIR_RGHT, HIGH); // forward
      digitalWrite(DIR_LFT, HIGH);

      if (GRAT == 37) {
      velocityL.play(FREQval3); // 
      velocityR.play(FREQval3); 
      prev_valu = 'Y';
      }
      if (GRAT == 52) {
      velocityL.play(FREQval5); // 
      velocityR.play(FREQval5); 
      prev_valu = 'Y';
      }      
    } 
  
   /*PHOT_LFT_VAL  = analogRead(PHOT_LFT); // crazy mix up in L-R naming 
    PHOT_RGHT_VAL = analogRead(PHOT_RGHT);

    /* Phot_Diff_Left = PHOT_LFT_VAL - PHOT_DIF_Lft;
    Serial.print(". PDL: ");
    Serial.print(Phot_Diff_Left);
    Serial.println(".");
    delay(250); */
    
    if(PHOT_LFT_VAL > PHOT_DIF_Lft) {
      LightDetectReact(1);  
      Serial.println("Left Photox Lit");
      Serial.print(PHOT_DIF_Lft); 
    }
    else if (PHOT_RGHT_VAL > PHOT_DIF_Rgh) {
      LightDetectReact(2);  
      Serial.println("RIGHT Photox Lit");
      Serial.print(PHOT_DIF_Rgh);
      }
    
    if( (digitalRead(LVR_LFT)==LOW)) { 
      //left STWITCH
      Serial.println("Left Lever"); 
      LeverDetectReact(1);   
    }
    else if( (digitalRead(LVR_RGHT)==LOW)) { 
      //RGHT SWITCH
      Serial.println("Right Lever");
      LeverDetectReact(2);
    }
    else if (Vstop == 0){
      digitalWrite(INHIB_LFT,ON_LFT);  //if nothing else but Vstop then
      digitalWrite(INHIB_RGHT,ON_RGHT); //on on
      digitalWrite(DIR_RGHT, HIGH); // forward
      digitalWrite(DIR_LFT, HIGH);      
    }

   /*if(PHOT_LFT_VAL + PHOT_RGHT_VAL > PHOT_THRS){
      digitalWrite(INHIB_LFT,ON_LFT);  // close
      digitalWrite(INHIB_RGHT,ON_RGHT); //move forwards no matter 
   } 
    
     if(PHOT_RGHT_VAL - PHOT_LFT_VAL > PHOT_DIF){
      digitalWrite(INHIB_RGHT,!ON_RGHT);
     } 
     
     if(PHOT_LFT_VAL - PHOT_RGHT_VAL > PHOT_DIF){
      digitalWrite(INHIB_LFT,!ON_LFT); 
     }*/
    
    Serial.print(". Phot Right: ");
    Serial.print(PHOT_LFT_VAL);
    //Serial.print(". Phot Dif Left: ");
    //Serial.print(PHOT_DIF_Lft);
    Serial.print(". Phot Left: ");
    Serial.print(PHOT_RGHT_VAL);
    Serial.println(".");
    Serial.print(". pp ");
    Serial.print( pp );
    Serial.println(".");
    //delay(200); // slow down debug
    pp++; 
  }
  if(digitalRead(V_SWTCH)==LOW) {
    digitalWrite(INHIB_LFT, !ON_LFT);//stop
    digitalWrite(INHIB_RGHT,!ON_RGHT);
   // belt and suspenders   
  }
}

boolean photoactivate(){
  return max(analogRead(PHOT_LFT),analogRead(PHOT_RGHT))>PHOT_THRS ? true:false;
}
boolean remoteTactivate(){
  Serial.println(" Inside remoteTactivate waaaaaaaaaaay  ");
  delay(10); 
  prev_vuln = 0;
  if ( Serial.available( ) )  {
    Serial.println("In Setup IF") ; 
    vuln = Serial.read( );
    Serial.println(vuln) ;
    if (vuln == 'T') { 
      prev_vuln = 1; 
    }
    else {
     prev_vuln = 0; 
    }
    return prev_vuln == 1 ? true:false;
  }
  return prev_vuln == 1 ? true:false;
}

int LightDetectReact(int sighd) {
      if (sighd == 1) {
        Serial.println("Left Photox Lit");
        Serial.print(PHOT_DIF_Lft);
        digitalWrite(INHIB_LFT, !ON_LFT);//stop
        delay(turn_dura); // turn left
        digitalWrite(INHIB_LFT, ON_LFT); // start left
      }
      else if (sighd == 2) {
        Serial.println("Right Photox Lit");
        Serial.print(PHOT_DIF_Rgh);
        digitalWrite(INHIB_RGHT, !ON_RGHT);//stop
        delay(turn_dura); // turn left
        digitalWrite(INHIB_RGHT, ON_RGHT); // start left
      }
        
      if (GRAT == 52) {
         velocityL.play(7025); // speed up
         velocityR.play(7025);
         delay(sped_dura); // go straight
         velocityL.play(5430); // fin fixed action pattern
         velocityR.play(5430);
      Serial.println("GRreater CLK for 5.2 gear ratio");
      }
      else {
        velocityL.play(5500); // speed up 3.7 gear ratio
        velocityR.play(5500);
        delay(sped_dura); // go straight
        velocityL.play(3800); // fin fixed action pattern
        velocityR.play(3800);
        Serial.println("Photox L or R Lit_Fin");
      }       
      
      velocityL.play(5500); // speed up
      velocityR.play(5500);
      delay(sped_dura); // go straight
      velocityL.play(3800); // fin fixed action pattern
      velocityR.play(3800);
      Serial.println("Photox L or R Lit_Fin");
}  

int LeverDetectReact(int sighd) {
      digitalWrite(INHIB_LFT, !ON_LFT);
      digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop after hit
      delay(400);
      digitalWrite(INHIB_LFT, ON_LFT); 
      digitalWrite(INHIB_RGHT, ON_RGHT);//on on
      digitalWrite(DIR_LFT, LOW);
      digitalWrite(DIR_RGHT, LOW); //reverse reverse
      delay(600);
      digitalWrite(INHIB_LFT, !ON_LFT);
      digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop after reverse
      delay(400);
      digitalWrite(INHIB_LFT, ON_LFT); 
      digitalWrite(INHIB_RGHT, ON_RGHT);//on on

      if (sighd == 1) {
       Serial.println("Left Lever Turn"); // about to turn 
       digitalWrite(DIR_RGHT, HIGH);
       delay(1280); // asymmetric turning angles for corner trap 
      } 
      else if (sighd == 2) {
        Serial.println("RIGHT Lever");    
        digitalWrite(DIR_LFT, HIGH); //in rev, make opp wheel forw
        delay(700); // asymmetric turning angles for corner trap
      }
      digitalWrite(INHIB_LFT, !ON_LFT);
      digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop after turn
      delay(400);
      digitalWrite(INHIB_LFT, ON_LFT); 
      digitalWrite(INHIB_RGHT, ON_RGHT);//on on     
         
      digitalWrite(DIR_LFT, HIGH); //turn  
      digitalWrite(DIR_RGHT, HIGH); // forward  forward 
}  
// below not currently called... 
int RemoteDetectReact(int sighd, char valu) { 
  digitalWrite(INHIB_LFT, !ON_LFT);
  digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop after letter U, W 
  //delay(50);
 
  while (valu == 'U' || valu == 'W') {
      digitalWrite(INHIB_LFT, ON_LFT); 
      digitalWrite(INHIB_RGHT, ON_RGHT);//on on
      digitalWrite(DIR_RGHT, LOW);
      digitalWrite(DIR_LFT, LOW); // reverse
    if (sighd == 1) {
      Serial.println("Left Remote Reverse"); // back up more R than L   
      velocityL.play(4300); // reverse to the rover left
      velocityR.play(1500);
      delay(100); // ballistic move 
    }
    else if (sighd == 2) {
      Serial.println("Right Remote Reverse"); // back up more L than R   
      velocityL.play(2500); // reverse to the rover right
      velocityR.play(4000);
      delay(100); // ballistic move     
    }
    if( Serial.available( ) > 0 ) {  // from example code 
      valu = Serial.read( );
      Serial.println("In valu read if");
      Serial.println(valu) ;  
    }
    else {
      valu = 'X'; 
    }
  }
  
  digitalWrite(INHIB_LFT, !ON_LFT);
  digitalWrite(INHIB_RGHT,!ON_RGHT); //stop stop after letter U, W over  
  delay(200);
  digitalWrite(INHIB_LFT, ON_LFT); 
  digitalWrite(INHIB_RGHT, ON_RGHT);//go  go  
  digitalWrite(DIR_RGHT, HIGH); // forward
  digitalWrite(DIR_LFT, HIGH);
  velocityL.play(3800); // 
  velocityR.play(3800);
}
    
//Vulnerability switch interrupt handler.
// void interruptroutine(int prev_vuln) 
void interruptroutine() {
  if(digitalRead(V_SWTCH)==LOW ) //&& prev_vuln == 1) 
  { //if vulnerability switch is pressed
    enabled = false; //disable robot
    digitalWrite(INHIB_LFT, !ON_LFT);//stop
    digitalWrite(INHIB_RGHT,!ON_RGHT);
    velocityL.stop(); //stop clocks
    velocityR.stop();
  }
  else{ //if vulnerability switch is released
    enabled = true; //enable robot
    velocityL.play(FREQ_LFT);//start clocks
    velocityR.play(FREQ_RGHT);
    digitalWrite(INHIB_LFT, !ON_LFT);//stop 
    digitalWrite(INHIB_RGHT,!ON_RGHT);
    digitalWrite(DIR_RGHT, HIGH); // forward
    digitalWrite(DIR_LFT, HIGH);
  }
}








