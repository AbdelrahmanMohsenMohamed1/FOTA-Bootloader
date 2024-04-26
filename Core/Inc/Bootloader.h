/*
 * Bootloader.h
 *
 *  Created on: Apr 23, 2024
 *      Author: Abdelrahman Mohsen
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

#include <string.h>
#include <stdio.h>
#include "main.h"

// External references to hardware peripherals
extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart2;

// Bootloader command definitions
#define CBL_GO_TO_ADDR_CMD           0x14
#define CBL_FLASH_ERASE_CMD          0x15
#define CBL_MEM_WRITE_CMD            0x16

// CRC verification results
#define CRC_VERIFING_FAILED  0x00
#define CRC_VERIFING_PASS    0x01

// Response codes
#define SEND_NACK        0xAB
#define SEND_ACK         0xCD

// Maximum payload size for host communication
#define HOSTM_MAX_SIZE    200

// Flash-related constants
#define CBL_FLASH_MAX_PAGE_NUMBER    16
#define CBL_FLASH_MASS_ERASE         0xFF
#define INVALID_PAGE_NUMBER          0x00
#define VALID_PAGE_NUMBER            0x01
#define UNSUCCESSFUL_ERASE           0x02
#define SUCCESSFUL_ERASE             0x03
#define HAL_SUCCESSFUL_ERASE         0xFFFFFFFFU

// Address validation constants
#define ADDRESS_IS_INVALID           0x00
#define ADDRESS_IS_VALID             0x01

// STM32F103 memory layout definitions
#define STM32F103_SRAM_SIZE          (20 * 1024)
#define STM32F103_FLASH_SIZE         (64 * 1024)
#define STM32F103_SRAM_END           (SRAM_BASE + STM32F103_SRAM_SIZE)
#define STM32F103_FLASH_END          (FLASH_BASE + STM32F103_FLASH_SIZE)

// Flash write status
#define FLASH_PAYLOAD_WRITE_FAILED   0x00
#define FLASH_PAYLOAD_WRITE_PASSED   0x01


// Bootloader status enumeration
typedef enum {
    BL_NACK = 0,
    BL_ACK
} BL_status;

// Function prototypes
uint8_t Bootloader_uint8FlashErase(uint32_t PageAddress, uint8_t page_Number);
BL_status Bootloader_enumRecHostCommands();
void Bootloader_voidJumpToApp();
uint8_t Bootloader_voidCheckStartCommand();

#endif /* INC_BOOTLOADER_H_ */
