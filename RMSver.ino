#include "MegunoLink.h"
#include <math.h> 
#include <RCSwitch.h>

TimePlot Plot, threshPlot, avgPlot;
RCSwitch mySwitch = RCSwitch();

#define groupIDlen  10
#define switchIDlen 10
char groupID[groupIDlen] = "11111";
char switchID[switchIDlen] = "10000";

int status=0;

unsigned long lastClap=millis();
unsigned long claps=0;

//see https://yaab-arduino.blogspot.com/2015/02/fast-sampling-from-analog-input.html
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define ARRSIZE 10

double sound=-1;
double soundlevels[ARRSIZE];
int arrpos=0;
int doubleRC=0; //doubleerrupt counter

double AVG=0.0;
double offset=100.0; 
double threshold; 
  

//DB values
double thresholdDB;
double offsetDB=5.0;
double avgDB=1.0;  
double soundDB;
  
void setup() {

  mySwitch.enableTransmit(7);
  mySwitch.switchOff(groupID, switchID);
  
  for(int i=0;i<ARRSIZE;i++){
    soundlevels[i]=analogRead(A0);
    delay(100);
  }

  //see https://www.instructables.com/Arduino-Timer-Interrupts/
  cli();//stop doubleerrupts
  //set timer1 doubleerrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare doubleerrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
  
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  
  //76khz with analogread
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  
  
  //initial avg (normal)
  AVG = analogRead(A0);
  delay(100);
  AVG += analogRead(A0);
  delay(100);
  AVG += analogRead(A0);
  delay(100);
  AVG += analogRead(A0);
  delay(100);
  AVG=AVG/4;
  threshold=AVG+offset;


  //DB versions 
  avgDB=mapDB(AVG);
  thresholdDB=avgDB+offsetDB;

}


void loop(){
  

  sound=analogRead(A0);
  soundDB=convertToDB(sound)+avgDB;

  //uncomment to see plot:
  //Plot.SendData("Clap Sensor", soundDB);
  //threshPlot.SendData("threshold", thresholdDB);
  //avgPlot.SendData("avg", avgDB);

  
    
    if(soundDB>thresholdDB){
      if(millis()-lastClap>10){ //time inbetween claps
        claps++;
        lastClap=millis();
      }else{
        claps=1;
        lastClap=millis();
      }
      
    }
  
    if(millis()-lastClap>1000){ 
      if(claps==2){
        if(status){
          mySwitch.switchOff(groupID, switchID);
          status=0;
        }
        else{
          mySwitch.switchOn(groupID, switchID);
          status=1;
        }     
      }
      claps=0;
    
    
  }
  //---------------------------------------------------------------------------------------------------


}


double calcRMS(){
  double sum=0;
  double amount=0;
  for(int i=0;i<ARRSIZE;i++){
    if(soundlevels[i]!=-1){
      double squared=sq(soundlevels[i]);
      sum+=squared;
      amount++;
    }
  }
  if(amount!=0){
    sum=sqrt((1.0/ARRSIZE)*sum);
  }
  return sum;
}

double convertToDB(double inp){
  double RMS=calcRMS();
  float value=inp/RMS;
  double soundLog=20*log10(value);
  return soundLog;
}


//doubleerrupt Service Routine
ISR(TIMER1_COMPA_vect){
  doubleRC++; //counts up every second
  if(doubleRC >= 30){ //after n seconds, change value in sound array
    //Serial.prdoubleln("one minute");
    calculateAverage();
    if(sound != -1){ //Sensor coulndt read any data yet if data is -1
      doubleRC =0;
      soundlevels[arrpos]=sound;
      arrpos++;
  
      if(arrpos>=ARRSIZE){
        arrpos=0;
      }
    }

  }

} 

void calculateAverage(){ //baseline (not DB)
    double sum=0.0;
    double amnt=0;
    for(int i=0;i<ARRSIZE;i++){
      if(soundlevels[i]!=-1){
          amnt++;
          sum+=soundlevels[i];
      }
    }
    if(amnt>0){
      AVG = sum / amnt; 
      avgDB=mapDB(AVG);
    }
    
    threshold=AVG+offset;
    thresholdDB=avgDB+offsetDB;

  }

double mapDB(double volt){
  double DBval=(volt+200)/10; //baseline 300+200=500 then divide by 10 for matching dB value
  return DBval;
}
