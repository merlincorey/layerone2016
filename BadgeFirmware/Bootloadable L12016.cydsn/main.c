/******************************************************************************
* Project Name		: Bootloadable Blinking LED
* File Name		: main.c
* Version 		: 1.0
* Device Used		: CY8C4245AXI-483
* Software Used		: PSoC Creator 3.3
* Compiler    		: ARMGCC 4.7.3, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-049-42xx PSoC 4 Prototyping Kit
*
********************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges.
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement.
*******************************************************************************/

/******************************************************************************
*                           THEORY OF OPERATION
* This is a blinking LED project. A PWM component drives the pin to blink the
* LED at regular intervals. This project contains a bootloadable component so
* that it can be bootloaded into PSoC 4 which has a bootloader already programmed
* into it.
*
* Default UART Port Configuration for bootloading the PSoC 4 in CY8CKIT-049-42xx
*   Baud Rate : 115200 bps
*   Data Bits : 8
*   Stop Bits : 1
*   Parity    : None
*
* Default MiniProg3 Configuration for programming the PSoC4 in CYBCKIT-049-42xx
*   Clock speed     : 24 MHz
*   Power           : External
*   Connector       : 5 pin
*   Acquire Mode    : Reset
*   Active Protocol : SWD
******************************************************************************/

/**
 ** Set CHARLIE to 1 to use charlie-x's original code
 **/
#ifndef CHARLIE
#define CHARLIE 0
#endif

/**
 ** main entry point
 **/
#if (CHARLIE)

#include "charlie.h"

#else

#include <project.h>

extern const uint32 StripLights_CLUT[];

void initialize_uart_bootloader() {
  UART_Start();
  UART_SpiUartClearRxBuffer();
  UART_SpiUartClearTxBuffer();  
}

void initialize_striplights() {
  StripLights_Start();
}

void initialize_wifi() {
  // Setting for ESP8266 enable */
  CH_PD_SetDriveMode ( CH_PD_DM_STRONG ) ;
  // ESP8266 off
  CH_PD_Write ( 0 );
  // start wifi for esp wifi
  uWIFI_Start();
  uWIFI_SpiUartClearRxBuffer();
  
}

void initialize() {
  initialize_uart_bootloader();
  initialize_striplights();
  initialize_wifi();
  // Enable global interrupts
  CyGlobalIntEnable;
  // LED drive on or off ?
  P1_6_SetDriveMode ( P1_6_DM_STRONG ) ;

  // LED on 
  P1_6_Write ( 1 );

  // diode is not bypassed for LED's
  // dim 2   40 - 60mA @ 3V
  // dim 3   30 - 40mA @ 3V
  // dim 4   20mA      @ 3V
  // dim 4   20 - 40mA @ 5V
}

int wifi_present() {
  // detect ESP8266  
  return (0 != GPIO2_Read());
}

void striplights_displayclear_delay(uint32 color, int microseconds) {
  StripLights_DisplayClear(color);
  CyDelay(microseconds);
}

int main() {
  initialize();

  if (wifi_present()) {
    // lower brightness when ESP is connected
    StripLights_Dim(4);
    // detected so different colour
    StripLights_DisplayClear(StripLights_ORANGE);
    CyDelay(500);
    StripLights_DisplayClear(StripLights_BLACK);
  } else {
    StripLights_Dim(3);
    // if gets to here WiFi didn't connect, or isn't present so turn it off.
    CH_PD_Write(0);
  }

  striplights_displayclear_delay(StripLights_RED_MASK,
				 500);
  striplights_displayclear_delay(StripLights_GREEN_MASK,
				 500);
  striplights_displayclear_delay(StripLights_BLUE_MASK,
				 500);

  StripLights_DisplayClear(0);
  CyDelay(2000);

  //reset board
  CySoftwareReset();  

  return 0;
}

#endif
  
