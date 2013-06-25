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
 
 int NumBlanks=1;    // number of cylces the SLM keeps displaying a pattern before the camera and AOTF are ready for next pattern 
 //int rep=2;          // number of repetitions of pos-switch-neg SLM displays and AOTF on-off-on sequences
 //int NumDirs=3;      // number of directions of grating
 //int NumPhases=3;    // number of phases of grating
 int rep=2;          // number of repetitions of pos-switch-neg SLM displays and AOTF on-off-on sequences
 int NumDirs=2;      // number of directions of grating
 int NumPhases=3;    // number of phases of grating
 
 
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



void SLMIlluminateFrame(int illum_mode)   
// processes one SLM cycle on-switch-off-switch, illum_mode==0 mean
// Laser always off, illum_mode==2 means Laser on Frame and Antiframe,
// illum_mode==1 means Laser on only Frame
{
  while(digitalRead(slm_out)==LOW)
    delayMicroseconds(mindelay);
  
  if (illum_mode>0)
    digitalWrite(aotf_enable,HIGH);     // illuminate SLM
  
  while(digitalRead(slm_out)==HIGH)
    delayMicroseconds(mindelay);
  
  if (illum_mode>0)
    digitalWrite(aotf_enable,LOW); // prevent illumination while SLM in undefined state

  while(digitalRead(slm_out)==LOW) // wait until SLM shows negative image      
    delayMicroseconds(mindelay);

  if (illum_mode>1)
    digitalWrite(aotf_enable,HIGH); // illuminate the SLM again     

  while(digitalRead(slm_out)==HIGH) // wait that SLM switches to next image
    delayMicroseconds(mindelay);
 
  if (illum_mode>1) 
    digitalWrite(aotf_enable,LOW) ;          // stop illuminating the SLM
}

void loop() {
  digitalWrite(slm_trigger,LOW);
  digitalWrite(aotf_enable,LOW);  
    
  while(digitalRead(cam_out)==LOW) {  // Wait for Camera ready signal -> high
    delayMicroseconds(mindelay);  
  } //wait for cam_out==HIGH
  
  delay(2);  // unit in msec
  cli();   
 
  for(int d=0;d<NumDirs;d++)   
    for(int p=0;p<NumPhases;p++){
      digitalWrite(cam_in,HIGH);   //start camera intergration 
      while(digitalRead(cam_out_G)==LOW) {  // wait for global exposure to start
	delayMicroseconds(mindelay); 
      }
      
      digitalWrite(slm_trigger,HIGH); // start slm     
      for(int i=0;i<rep;i++){ 
        SLMIlluminateFrame(2);  
	digitalWrite(slm_trigger,LOW);  
      }
      
      digitalWrite(cam_in,LOW)  ;         //end integration of Camera --> readout starts    
      for(int i=0;i<NumBlanks;i++){    // listen to the SLM for timing purposes
        SLMIlluminateFrame(0);
      } 
    }

  sei();
  delayMicroseconds(mindelay);  //wait for SLM switching process
}
