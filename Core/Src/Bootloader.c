/*
 * Bootloader.c
 *
 *  Created on: April 26,2024
 *      Author: Abdelrahman Mohsen
 */
#include "Bootloader.h"

static void BL_Write_Data(uint8_t *Host_buffer);
static uint32_t BL_CRC_verfiy(uint8_t * pdata,uint32_t DataLen,uint32_t HosrCRC);
static uint8_t BL_Address_Varification(uint32_t Addresss);
static uint8_t FlashMemory_Paylaod_Write(uint16_t * pdata,uint32_t StartAddress,uint8_t Payloadlen);
static void BL_Send_ACK(uint8_t dataLen);
static void BL_Send_NACK();
static uint8_t Host_buffer[HOSTM_MAX_SIZE];

BL_status Bootloader_enumRecHostCommands()
{
	BL_status status = BL_NACK;
	HAL_StatusTypeDef Hal_status=HAL_ERROR;
	uint8_t DataLen=71;
	memset(Host_buffer,0,HOSTM_MAX_SIZE);
	Hal_status = HAL_UART_Receive(&huart2,Host_buffer,DataLen,1000);
	if(Hal_status != HAL_OK)
	{
		status = BL_NACK;
	}
	else{
		if(Host_buffer[1] == CBL_MEM_WRITE_CMD)
		{
			BL_Write_Data(Host_buffer);
		}
		else
		{

		}
	}
	return status;
}

uint16_t i=0;
static void BL_Write_Data(uint8_t *Host_buffer)
{
	uint8_t Adress_varfiy=ADDRESS_IS_INVALID;
	uint32_t Address_Host=0;
	uint8_t DataLen=0;
	uint8_t payload_status =FLASH_PAYLOAD_WRITE_FAILED;
	//uint16_t Host_Packet_Len=0;
	//uint32_t CRC_valu=0;
	//Host_Packet_Len =  Host_buffer[0]+1;
	//CRC_valu = *((uint32_t*)(Host_buffer+Host_Packet_Len -4));
	Address_Host = 0x08008000 + (64*i);
	i++;
	DataLen = Host_buffer[6];
	Adress_varfiy = BL_Address_Varification(Address_Host);
	if(Adress_varfiy == ADDRESS_IS_VALID)
	{

		uint16_t* pdata = (uint16_t*)(&Host_buffer[7]);
		payload_status = FlashMemory_Paylaod_Write(pdata,Address_Host,DataLen);
		HAL_UART_Transmit(&huart2,(uint8_t*)&payload_status,1,HAL_MAX_DELAY);
	}
	else
	{
		HAL_UART_Transmit(&huart2,(uint8_t*)&Adress_varfiy,1,HAL_MAX_DELAY);
	}
}

static uint8_t BL_Address_Varification(uint32_t Addresss)
{
	uint8_t Adress_varfiy=ADDRESS_IS_INVALID;
	if(Addresss>=FLASH_BASE &&Addresss<=STM32F103_FLASH_END)
	{
		Adress_varfiy=ADDRESS_IS_VALID;
	}
	else if(Addresss>=SRAM_BASE &&Addresss<=STM32F103_SRAM_END)
	{
		Adress_varfiy=ADDRESS_IS_VALID;
	}
	else{
		Adress_varfiy=ADDRESS_IS_INVALID;
	}
	return Adress_varfiy;
}
static uint32_t BL_CRC_verfiy(uint8_t * pdata,uint32_t DataLen,uint32_t HostCRC)
{
	uint8_t crc_status=CRC_VERIFING_FAILED;
	uint32_t MCU_CRC=0;
	uint32_t dataBuffer=0;
	for(uint8_t count =0;count<DataLen ;count++)
	{
		dataBuffer = (uint32_t)pdata[count];
		MCU_CRC = HAL_CRC_Accumulate(&hcrc,&dataBuffer,1);
	}
	__HAL_CRC_DR_RESET(&hcrc);
	if(HostCRC == MCU_CRC )
	{
		crc_status=CRC_VERIFING_PASS;
	}
	else{
		crc_status=CRC_VERIFING_FAILED;
	}
	return crc_status;
}

static void BL_Send_ACK(uint8_t dataLen)
{
	uint8_t ACK_value[2]={0};
	ACK_value[0]=SEND_ACK;
	ACK_value[1]=dataLen;
	HAL_UART_Transmit(&huart2,(uint8_t*)ACK_value,2,HAL_MAX_DELAY);
}
static void BL_Send_NACK()
{
	uint8_t ACk_value=SEND_NACK;
	HAL_UART_Transmit(&huart2,&ACk_value,sizeof(ACk_value),HAL_MAX_DELAY);
}
uint8_t Bootloader_uint8FlashErase(uint32_t PageAddress, uint8_t page_Number)
{
	FLASH_EraseInitTypeDef pEraseInit;
	HAL_StatusTypeDef Hal_status  = HAL_ERROR;
	uint32_t PageError =0;
	uint8_t PageStatus=INVALID_PAGE_NUMBER;
	if(page_Number>CBL_FLASH_MAX_PAGE_NUMBER)
	{
		PageStatus=INVALID_PAGE_NUMBER;
	}
	else
	{
		PageStatus=VALID_PAGE_NUMBER;
		if(page_Number<= (CBL_FLASH_MAX_PAGE_NUMBER - 1) || PageAddress == CBL_FLASH_MASS_ERASE)
		{
			if(PageAddress ==CBL_FLASH_MASS_ERASE )
			{
				pEraseInit.TypeErase =FLASH_TYPEERASE_PAGES;
				pEraseInit.Banks = FLASH_BANK_1;
				pEraseInit.PageAddress = 0x8008000;
				pEraseInit.NbPages =12;
			}
			else{
				pEraseInit.TypeErase =FLASH_TYPEERASE_PAGES;
				pEraseInit.Banks = FLASH_BANK_1;
				pEraseInit.PageAddress = PageAddress;
				pEraseInit.NbPages =page_Number;
			}
			HAL_FLASH_Unlock();
			Hal_status = HAL_FLASHEx_Erase(&pEraseInit,&PageError);
			HAL_FLASH_Lock();
			if(PageError == HAL_SUCCESSFUL_ERASE)
			{
				PageStatus=SUCCESSFUL_ERASE;
			}
			else{
				PageStatus=UNSUCCESSFUL_ERASE;
			}
		}
		else
		{
			PageStatus=INVALID_PAGE_NUMBER;
		}
	}
	return PageStatus;

}
uint8_t UpdataAdress=0;
static uint8_t FlashMemory_Paylaod_Write(uint16_t * pdata,uint32_t StartAddress,uint8_t Payloadlen)
{
	uint32_t Address=0;
	UpdataAdress=0;
	HAL_StatusTypeDef Hal_status=HAL_ERROR;
	uint8_t payload_status =FLASH_PAYLOAD_WRITE_FAILED;
	//Perform_Flash_Erase(0xff,12);


	for(uint8_t payload_count=0,UpdataAdress=0;payload_count<Payloadlen/2;payload_count++,UpdataAdress+=2)
	{
		Address =  StartAddress +UpdataAdress;
		HAL_FLASH_Unlock();
		Hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address,pdata[payload_count]);
		HAL_FLASH_Lock();
		//HAL_FLASH_Lock();
		if(Hal_status != HAL_OK)
		{
			payload_status =FLASH_PAYLOAD_WRITE_FAILED;
		}
		else{
			payload_status =FLASH_PAYLOAD_WRITE_PASSED;
		}
	}

	return payload_status;

}

#define FLASH_SECTOR2_BASE_ADDRESS   0x08008000U
typedef void (*pMainApp)(void);
void Bootloader_voidJumpToApp()
{
	uint32_t MSP_Value = *((volatile uint32_t*)FLASH_SECTOR2_BASE_ADDRESS);
	uint32_t MainAppAdd = *((volatile uint32_t*)(FLASH_SECTOR2_BASE_ADDRESS+4));

	pMainApp ResetHandler_Address=(pMainApp)MainAppAdd;
	HAL_RCC_DeInit();
	__set_MSP(MSP_Value);
	ResetHandler_Address();
}

uint8_t check[16]="Bootloader Start";
uint8_t Erase[16]="Bootloader Ereas";
uint8_t start[16]={0};
uint8_t flag=0;
uint8_t Bootloader_voidCheckStartCommand()
{
	uint8_t count=0;
	HAL_UART_Receive(&huart2, start, sizeof(check), 100);

	while(check[count]!='\0')
	{
		if(start[count] != check[count])
			break;
		count++;
	}
	if(count == 16)
		return 1;

	else{
		count=0;
		while(Erase[count]!='\0')
		{
			if(start[count] != Erase[count])
				break;
			count++;
		}
		if(count == 16)
			return 2;
	}
	return 0;
}


