// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// Ezrah Machuca and Georgios Adamapolous
// 4/21/2025

#include <machine/_stdint.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

//uf0_1 for first wave, Ufo_6 for Boss

volatile const uint8_t *soundData;
volatile uint32_t soundCount;


void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this
  SysTick->CTRL = 0;
  SysTick->LOAD = period-1;
  SCB->SHP[1] = (SCB->SHP[1] & (~0xC0000000)) | (priority << 30);
  SysTick->VAL = 0;
  SysTick->CTRL = 7;
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
// write this
  DAC5_Init();
  soundData = 0;
  soundCount = 0; 
  // 7273 = 80,000,000/11,000 
  SysTick_IntArm(7273, 0);
  SysTick->LOAD = 0;  
}
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
  // output one value to DAC if a sound is active
  
  if(soundCount > 0){
    DAC5_Out(*soundData);
    soundData++;
    soundCount--;
  } else {
    SysTick->LOAD = 0;  // stop playback
  }

}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
// write this
  if(count == 0){
    return;
  } 
  soundData = pt;
  soundCount = count;
  SysTick->LOAD = 7273 - 1;
  SysTick->VAL = 0;
}
void Sound_Shoot(void){
  // write this
  Sound_Start(shoot, 4080);
}
void Sound_Killed(void){
  // write this
  Sound_Start(ufo_lowpitch, 25805);
}
void Sound_Explosion(void){
  // write this
  Sound_Start(explosion, 8731);//previously 3377
}

void Sound_Fastinvader1(void){
  Sound_Start(fastinvader1, 982); //PREVIOUSLY 2000

}

void Sound_Fastinvader2(void){
  Sound_Start(fastinvader2, 1042);
}

void Sound_Fastinvader3(void){

}
void Sound_Fastinvader4(void){

}
void Sound_Highpitch(void){

}