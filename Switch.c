/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author: George Adamopoulos and Ezrah Machuca
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
  // write this
  IOMUX->SECCFG.PINCM[PA24INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PA25INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PA26INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00040081;

}
// return current state of switches
uint32_t Switch_In(void){
  return ((GPIOA->DIN31_0 & 0x0F000000) >> 24) & 0x0F; // shifts value of inputs to the first 4 digits and makes sure its just the 4 digits
}
