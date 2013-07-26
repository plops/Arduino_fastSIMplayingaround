/*
  Trigger for the Hamamatsu C11440
  Trigger Mode: External Level Trigger
  Set exposure time and exposure length
 */
 
 // define all variables
 int slm_ext_run=10;    // starts hardware, activate running orders
 int slm_trigger=11;    // if high start showing image
 int slm_out=9;         // SLM LED enable signal, showing images when signal is High
 int cam_in=2;          // from arduino to camera_in (start exposure)
 int cam_out=4;         // from camera_out to arduino. signal is high if camera is ready for next exposure (output from camera to arduino)
 int cam_out_G=3;
 int aotf_enable=7;     // if high switch AOTF to on / blanking mode as well
 int aotf_signal=6;     // set amplitude of sound wave of AOTF
 int blinks=3;          // number of blinks at the beginning     
 int countblinks=0;
 int mindelay=10;       // time for delay
 
 int NumBlanks=0;    // number of cylces the SLM keeps displaying a pattern before the camera and AOTF are ready for next pattern 
 //int rep=2;          // number of repetitions of pos-switch-neg SLM displays and AOTF on-off-on sequences
 //int NumDirs=3;      // number of directions of grating
 //int NumPhases=3;    // number of phases of grating
 int rep=1;          // number of repetitions of pos-switch-neg SLM displays and AOTF on-off-on sequences
 int NumDirs=3;      // number of directions of grating
 int NumPhases=7;    // number of phases of grating
 unsigned long mytimeout=3000;  // Timeout in millisecs
 int SLMPhase=0;
 
/*
  while loops will loop continuously, and infinitely,
  until the expression inside the parenthesis, () becomes false. 
 */
//test if AOTF is working (try blinking) 
void TestAOTF() {
  countblinks=0;
 
   while(countblinks<blinks){           
      digitalWrite(aotf_enable,LOW);
      delay(200);  // millisecond 
      digitalWrite(aotf_enable,HIGH);
      delay(200);  
      countblinks=countblinks+1;
  }
  digitalWrite(aotf_enable,LOW);     // after 3 blinks, aotf set to low = off
}

void setup() {                
  // allocate names to pins on arduino
  //DIGITAL SIGNALS
    //SLM
    pinMode(slm_ext_run, OUTPUT);   // output signal from arduino to SLM
    pinMode(slm_trigger, OUTPUT);   // D2,output signal from arduino to SLM
    pinMode(slm_out, INPUT);        // D3, iutput signal from arduino to SLM; signal == high, if image is shown
      
    //CAMERA
    pinMode(cam_in, OUTPUT);   // output signal from arduino to camera; exposure while signal is high 
    pinMode(cam_out, INPUT);   // iutput signal from camera to arduino; Camera Trigger ready output  
    pinMode(cam_out_G, INPUT);         
    //AOTF             
    pinMode(aotf_enable, OUTPUT);  // output signal from arduino to AOTF; SLM is not illuminated but only for blink test
    pinMode(aotf_signal, OUTPUT);  // output signal from arduino to AOTF;  
    
    
  //SET SETTINGS
    digitalWrite(aotf_signal,HIGH); //sets maximum possible amplitude
                                    //of AOTF sound wave to maximum
    digitalWrite(slm_ext_run,HIGH);  //starts running order of SLM
    TestAOTF();
}



void SLMCycle(int DoAOTF) {  // processes one SLM cycle on-switch-off-switch, DoAOTF==0 mean Laser always off, DoAOTF==2 means Laser on Frame and Antiframe, DoAOTF==1 means Laser on only Frame
       SLMPhase++;  // To keept track of SLM position
       while(digitalRead(slm_out)==LOW) {
       delayMicroseconds(mindelay);} // while the signal of slm_out still Low, then do the delay in micorseconds untill the signal goes to High; 
                                     // this delay is for checking digitalRead not so often
                                     // at the moment, there is no image on SLM, and wait that slm displays next image
    
      // Now the positive image of grating is shown -> turn on the laser
      if (DoAOTF>0) digitalWrite(aotf_enable,HIGH);     // illuminate SLM
      
      while(digitalRead(slm_out)==HIGH){
       delayMicroseconds(mindelay);}  // while the signal of slm_out still High, then do the delay in micorseconds untill the signal goes to Low;
                                      // at the moment, there is a positive image on SLM, wait for SLM switching process

      // Now SLM is under switching process the laser should be off, then the next step of SLM will be to change to negative image
      if (DoAOTF>0) digitalWrite(aotf_enable,LOW);     // stop illumination the SLM
                                                     // camera still integrating!
      while(digitalRead(slm_out)==LOW) {
       delayMicroseconds(mindelay);}   // wait that SLM shows negative image      
      // SLM shows negative image
      if (DoAOTF>1) digitalWrite(aotf_enable,HIGH);      // illuminate the SLM again     

      while(digitalRead(slm_out)==HIGH){
       delayMicroseconds(mindelay);}   // wait that SLM switches to next grating     
      // SLM is now switching to the next grating     
      if (DoAOTF>1) digitalWrite(aotf_enable,LOW) ;          // stop illuminating the SLM
}

void CheckSLMPos(void)   // Checks the state of the SLM and introduces SLM dummy cycles if necessary  
                         // void (does not return anything)
                         // the void inside the main()means that no return value go into main()
{
  long TotalPhases=NumDirs*NumPhases;
  while (SLMPhase%TotalPhases != 0)   // != (not equal to)    //If SLMPhase not the same as TotalPhases
    SLMCycle(0);
  SLMPhase=0;
}

void loop() {
  unsigned long mytime;  int wasreset; // 'unsigned' means no negative, show everything only in positive
  digitalWrite(slm_trigger,LOW);
  digitalWrite(aotf_enable,LOW);  
     
  CheckSLMPos();
  while(digitalRead(cam_out)==LOW) {  // Wait for Camera ready signal -> high
     delayMicroseconds(mindelay);  
   } 
 
  mytime=millis();
  wasreset=0;

  //delay(2);  // unit in msec
  cli();   // disable the interrupts for the time that pricise timing is needed
                                      //camera is reading out at the moment
                                      //cam ready for exposure                                      
 
  for(int d=0;d<NumDirs;d++)    // replications of the pattern // 'd++' means ' d=d+1'   
  for(int p=0;p<NumPhases;p++)    // replications of the pattern    
  {
    //if (~wasreset)
      digitalWrite(cam_in,HIGH);   //start camera intergration 

      while(!wasreset && digitalRead(cam_out_G)==LOW) {  // Wait for global exposute to go high, // && means 'and'
         if (millis() > mytime + mytimeout)  // time out reached
             wasreset=1;
       } 
      if (wasreset) {delay(100);}
  
      digitalWrite(slm_trigger,HIGH);     

      for(int i=0;i<rep;i++){    //1 replications of the pattern, but only run SLMCycle once in the first time (i=0)
         SLMCycle(2);  // 1 shutters only positive frames, 2 shutters both positive and negative frames
         digitalWrite(slm_trigger,LOW);  //here the arduino has time - so we set the sending trigger signal back to low 
       }


      //if (~wasreset)  // ~changes each bit to its opposite, means 'either 1 to 0 or 0 to 1'
      digitalWrite(cam_in,LOW)  ;    //end integration of Camera --> readout starts  
  
      while(!wasreset && digitalRead(cam_out)==LOW){  // wait for camera ready signal to go high
         delayMicroseconds(mindelay);  
         if (millis() > mytime + mytimeout)  // time out reached
             wasreset=1;                     // The brackets may be omitted after an if statement. If this is done, the next line (defined by the semicolon) becomes the only conditional statement.
       }
      if (wasreset) {delay(100);}
  
      //for(int i=0;i<NumBlanks;i++){    // listen to the SLM for timing purposes
      //      SLMCycle(0);
      //    } 
  }
  
  sei();   // allow the arduino to perform housekeeping stuff
  delayMicroseconds(mindelay);  //wait for SLM switching process
  
  // remove the following part if you want to run Life mode!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  
  while(digitalRead(cam_out)==LOW){ // wait for camera ready to go high
     delayMicroseconds(mindelay);  
   }

  delay(300);  //wait for SLM switching process
  while(digitalRead(cam_out)==HIGH){  // wait for camera ready to go low by itself (only for fixed number acquisition)
     delayMicroseconds(mindelay); 
   } 
}
