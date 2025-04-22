/*
 * LED.c
 *
 *  Created on: Nov 5, 2023
 *      Author: Georgios Adamopoulos and Ezrah Machuca
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table

// initialize your LEDs
void LED_Init(void){
  IOMUX->SECCFG.PINCM[PA15INDEX] = 0x81;  
  IOMUX->SECCFG.PINCM[PA16INDEX] = 0x81; 
  IOMUX->SECCFG.PINCM[PA17INDEX] = 0x81; 
  GPIOB->DOE31_0 |= (1 << 15) | (1 << 16) | (1 << 17);  
}
// data specifies which LED to turn on
/*
    Data goes from 0 to 2 (0 being PA15, 1 being PA16, 1 being PA17)
*/
void LED_On(uint32_t data){
    
    if(data <= 2){
        GPIOA->DOUTSET31_0 = (1 << (15 + data));
    }
}

// data specifies which LED to turn off
/*
    Data goes from 0 to 2 (0 being PA15, 1 being PA16, 1 being PA17)
*/
void LED_Off(uint32_t data){
   
    if(data <= 2){
        GPIOA->DOUTCLR31_0 = (1 << (15 + data));
    }


}

// data specifies which LED to toggle
/*
    Data goes from 0 to 2 (0 being PA15, 1 being PA16, 1 being PA17)
*/
void LED_Toggle(uint32_t data){
    
    if(data <= 2){
        GPIOA->DOUTTGL31_0 = (1 << (15 + data));
    }
}
