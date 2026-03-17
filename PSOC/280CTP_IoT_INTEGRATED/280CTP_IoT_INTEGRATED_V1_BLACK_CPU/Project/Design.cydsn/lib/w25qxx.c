
#include "w25qxx.h"

#define DELAY_W25QXX        (10)
#define W25QXX_DUMMY_BYTE         0xA5

w25qxx_t	w25qxx;

//###################################################################################################################
uint32_t W25qxx_ReadID(void)
{
    uint8 readDataNO = 0;
    uint8 len;
    uint32_t Temp;
    uint8_t data[4] = {0x9F,0xA5,0xA5,0xA5};
    
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_PutArray(data, 4);
    
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    
    while(SPIM_FLASH_GetRxBufferSize() > 0)
    {
        data[readDataNO++] = SPIM_FLASH_ReadByte();
    }
    
    Temp = (data[1] << 16) | (data[2] << 8) | data[3];
    return Temp;
}
//###################################################################################################################
void W25qxx_ReadUniqID(void)
{
    uint8 i = 0;
    uint8_t data[8] = {0x4B,0xA5,0xA5,0xA5,0xA5,};
    
    // Command    
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_PutArray(data, 5);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));  
    SPIM_FLASH_ClearRxBuffer();
    SPIM_FLASH_PutArray(data, 8);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
	while(SPIM_FLASH_GetRxBufferSize() > 0)
    {
        w25qxx.UniqID[i++] = SPIM_FLASH_ReadByte();
    }
}
//###################################################################################################################
void W25qxx_WriteEnable(void)
{
    CyDelay(1);
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0x06);	
    //SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE); 
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));  
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    SPIM_FLASH_ClearRxBuffer();
    CyDelay(1);
}
//###################################################################################################################
void W25qxx_WriteDisable(void)
{
    CyDelay(1);
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0x04);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));  
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    SPIM_FLASH_ClearRxBuffer();
    CyDelay(1);
}
//###################################################################################################################
uint8_t W25qxx_ReadStatusRegister(uint8_t	SelectStatusRegister_1_2_3)
{
    uint8 readDataNO =0; 
	uint8_t	status[2]={0,};
    
    Ctrl_MEM_SS_Write(0);
	if(SelectStatusRegister_1_2_3==1)
	{
		SPIM_FLASH_WriteByte(0x05);
        SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
        while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
        CyDelayUs(DELAY_W25QXX);
        while(SPIM_FLASH_GetRxBufferSize() > 0)
        {
            status[readDataNO++] = SPIM_FLASH_ReadByte();
        }
		w25qxx.StatusRegister1 = status[1];
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
		SPIM_FLASH_WriteByte(0x35);
        SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
        while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE)); 
        CyDelayUs(DELAY_W25QXX);
        while(SPIM_FLASH_GetRxBufferSize() > 0)
        {
            status[readDataNO++] = SPIM_FLASH_ReadByte();
        }
		w25qxx.StatusRegister2 = status[1];
	}
	else
	{
		SPIM_FLASH_WriteByte(0x15);
        SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
        while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE)); 
        CyDelayUs(DELAY_W25QXX);
        while(SPIM_FLASH_GetRxBufferSize() > 0)
        {
            status[readDataNO++] = SPIM_FLASH_ReadByte();
        }
		w25qxx.StatusRegister3 = status[1];
	}
    Ctrl_MEM_SS_Write(1);
    CyDelayUs(DELAY_W25QXX);
	return status[1];
}
//###################################################################################################################
void W25qxx_WriteStatusRegister(uint8_t	SelectStatusRegister_1_2_3,uint8_t Data)
{
	uint8_t	status=0;
    Ctrl_MEM_SS_Write(0);
	if(SelectStatusRegister_1_2_3==1)
	{
        SPIM_FLASH_WriteByte(0x01);
		w25qxx.StatusRegister1 = Data;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
        SPIM_FLASH_WriteByte(0x31);
		w25qxx.StatusRegister2 = Data;
	}
	else
	{
        SPIM_FLASH_WriteByte(0x11);
		w25qxx.StatusRegister3 = Data;
	}
	SPIM_FLASH_WriteByte(Data);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE)); 
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    CyDelay(20);
}
//###################################################################################################################
void W25qxx_WaitForWriteEnd(void)
{
    Ctrl_MEM_SS_Write(0);
	SPIM_FLASH_WriteByte(0x05);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    SPIM_FLASH_ClearRxBuffer();
    do
    {
        SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
        while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
        w25qxx.StatusRegister1 = SPIM_FLASH_ReadByte();
    }
    while ((w25qxx.StatusRegister1 & 0x01) == 0x01);
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    CyDelay(1);
}
//###################################################################################################################
bool	W25qxx_Init(void)
{
    w25qxx.Lock=1;
    CyDelay(100);
    
    uint32_t	id;
    
    id=W25qxx_ReadID();

    switch(id & 0x000000FF)
    {
    	case 0x20:	// 	w25q512
    		w25qxx.ID=W25Q512;
    		w25qxx.BlockCount=1024;
    	break;
    	case 0x19:	// 	w25q256
    		w25qxx.ID=W25Q256;
    		w25qxx.BlockCount=512;
    	break;
    	case 0x18:	// 	w25q128
    		w25qxx.ID=W25Q128;
    		w25qxx.BlockCount=256;
    	break;
    	case 0x17:	//	w25q64
    		w25qxx.ID=W25Q64;
    		w25qxx.BlockCount=128;
    	break;
    	case 0x16:	//	w25q32
    		w25qxx.ID=W25Q32;
    		w25qxx.BlockCount=64;
    	break;
    	case 0x15:	//	w25q16
    		w25qxx.ID=W25Q16;
    		w25qxx.BlockCount=32;
    	break;
    	case 0x14:	//	w25q80
    		w25qxx.ID=W25Q80;
    		w25qxx.BlockCount=16;
    	break;
    	case 0x13:	//	w25q40
    		w25qxx.ID=W25Q40;
    		w25qxx.BlockCount=8;
    	break;
    	case 0x12:	//	w25q20
    		w25qxx.ID=W25Q20;
    		w25qxx.BlockCount=4;
    	break;
    	case 0x11:	//	w25q10
    		w25qxx.ID=W25Q10;
    		w25qxx.BlockCount=2;
    	break;
    	default:
    		w25qxx.Lock=0;	
    		return false;
    			
    }		
    w25qxx.PageSize=256;
    w25qxx.SectorSize=0x1000;
    w25qxx.SectorCount=w25qxx.BlockCount*16;
    w25qxx.PageCount=(w25qxx.SectorCount*w25qxx.SectorSize)/w25qxx.PageSize;
    w25qxx.BlockSize=w25qxx.SectorSize*16;
    w25qxx.CapacityInKiloByte=(w25qxx.SectorCount*w25qxx.SectorSize)/1024;
    W25qxx_ReadUniqID();
    W25qxx_ReadStatusRegister(1);
    W25qxx_ReadStatusRegister(2);
    W25qxx_ReadStatusRegister(3);
    w25qxx.Lock=0;	
    return true;
}	
//###################################################################################################################
void	W25qxx_EraseChip(void)
{
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;	
	W25qxx_WriteEnable();
    
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0xC7);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);       
    
	W25qxx_WaitForWriteEnd();
    
	w25qxx.Lock=0;	
}
//###################################################################################################################
void W25qxx_EraseSector(uint32_t SectorAddr)
{
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;
	//W25qxx_WaitForWriteEnd();
	SectorAddr = SectorAddr * w25qxx.SectorSize;
    W25qxx_WriteEnable();
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0x20);
    SPIM_FLASH_WriteByte((SectorAddr & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((SectorAddr & 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(SectorAddr & 0xFF);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    CyDelay(200);
    W25qxx_WaitForWriteEnd();
	CyDelay(1);
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;	
	W25qxx_WaitForWriteEnd();
	BlockAddr = BlockAddr * w25qxx.SectorSize*16;
    W25qxx_WriteEnable();
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0xD8);
	if(w25qxx.ID>=W25Q256)
		SPIM_FLASH_WriteByte((BlockAddr & 0xFF000000) >> 24);
    SPIM_FLASH_WriteByte((BlockAddr & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((BlockAddr & 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(BlockAddr & 0xFF);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    W25qxx_WaitForWriteEnd();
	CyDelay(1);
	w25qxx.Lock=0;
}
//###################################################################################################################
uint32_t	W25qxx_PageToSector(uint32_t	PageAddress)
{
	return ((PageAddress*w25qxx.PageSize)/w25qxx.SectorSize);
}
//###################################################################################################################
uint32_t	W25qxx_PageToBlock(uint32_t	PageAddress)
{
	return ((PageAddress*w25qxx.PageSize)/w25qxx.BlockSize);
}
//###################################################################################################################
uint32_t	W25qxx_SectorToBlock(uint32_t	SectorAddress)
{
	return ((SectorAddress*w25qxx.SectorSize)/w25qxx.BlockSize);
}
//###################################################################################################################
uint32_t	W25qxx_SectorToPage(uint32_t	SectorAddress)
{
	return (SectorAddress*w25qxx.SectorSize)/w25qxx.PageSize;
}
//###################################################################################################################
uint32_t	W25qxx_BlockToPage(uint32_t	BlockAddress)
{
	return (BlockAddress*w25qxx.BlockSize)/w25qxx.PageSize;
}

//###################################################################################################################
void W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;
	W25qxx_WaitForWriteEnd();
    
    W25qxx_WriteEnable();
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0x02);
	if(w25qxx.ID>=W25Q256)
		SPIM_FLASH_WriteByte((WriteAddr_inBytes & 0xFF000000) >> 24);
    SPIM_FLASH_WriteByte((WriteAddr_inBytes & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((WriteAddr_inBytes & 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(WriteAddr_inBytes & 0xFF);
    SPIM_FLASH_WriteByte(pBuffer);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    W25qxx_WaitForWriteEnd();
	w25qxx.Lock=0;
}
//###################################################################################################################
void 	W25qxx_WritePage(uint8_t *pBuffer	,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToWrite_up_to_PageSize)
{
    uint32_t i;
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;
	if(((NumByteToWrite_up_to_PageSize+OffsetInByte)>w25qxx.PageSize)||(NumByteToWrite_up_to_PageSize==0))
		NumByteToWrite_up_to_PageSize=w25qxx.PageSize-OffsetInByte;
	if((OffsetInByte+NumByteToWrite_up_to_PageSize) > w25qxx.PageSize)
		NumByteToWrite_up_to_PageSize = w25qxx.PageSize-OffsetInByte;
	W25qxx_WaitForWriteEnd();
    W25qxx_WriteEnable();
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0x02);
	Page_Address = (Page_Address*w25qxx.PageSize)+OffsetInByte;	
	if(w25qxx.ID>=W25Q256)
		SPIM_FLASH_WriteByte((Page_Address & 0xFF000000) >> 24);
    SPIM_FLASH_WriteByte((Page_Address & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((Page_Address & 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(Page_Address&0xFF);
	//HAL_SPI_Transmit(&_W25QXX_SPI,pBuffer,NumByteToWrite_up_to_PageSize,100);
    for(i = 0; i < NumByteToWrite_up_to_PageSize; i++)
    {
        SPIM_FLASH_WriteByte(*pBuffer++);
    }
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    W25qxx_WaitForWriteEnd();
	CyDelay(1);
	w25qxx.Lock=0;
}
//###################################################################################################################
void 	W25qxx_WriteSector(uint8_t *pBuffer	,uint32_t Sector_Address,uint32_t OffsetInByte	,uint32_t NumByteToWrite_up_to_SectorSize)
{
	if((NumByteToWrite_up_to_SectorSize>w25qxx.SectorSize)||(NumByteToWrite_up_to_SectorSize==0))
		NumByteToWrite_up_to_SectorSize=w25qxx.SectorSize;
	if(OffsetInByte>=w25qxx.SectorSize)
	{
		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToWrite;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToWrite_up_to_SectorSize) > w25qxx.SectorSize)
		BytesToWrite = w25qxx.SectorSize-OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_SectorSize;	
	StartPage = W25qxx_SectorToPage(Sector_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_WritePage(pBuffer,StartPage,LocalOffset,BytesToWrite);
		StartPage++;
		BytesToWrite-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToWrite>0);	
}
//###################################################################################################################
void 	W25qxx_WriteBlock	(uint8_t* pBuffer ,uint32_t Block_Address	,uint32_t OffsetInByte	,uint32_t	NumByteToWrite_up_to_BlockSize)
{
	if((NumByteToWrite_up_to_BlockSize>w25qxx.BlockSize)||(NumByteToWrite_up_to_BlockSize==0))
		NumByteToWrite_up_to_BlockSize=w25qxx.BlockSize;
	if(OffsetInByte>=w25qxx.BlockSize)
	{
		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToWrite;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToWrite_up_to_BlockSize) > w25qxx.BlockSize)
		BytesToWrite = w25qxx.BlockSize-OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_BlockSize;	
	StartPage = W25qxx_BlockToPage(Block_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_WritePage(pBuffer,StartPage,LocalOffset,BytesToWrite);
		StartPage++;
		BytesToWrite-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToWrite>0);	
}
//###################################################################################################################
void 	W25qxx_ReadByte(uint8_t *pBuffer,uint32_t Bytes_Address)
{
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;
    Ctrl_MEM_SS_Write(0);
    SPIM_FLASH_WriteByte(0x0B);
	if(w25qxx.ID>=W25Q256)
		SPIM_FLASH_WriteByte((Bytes_Address & 0xFF000000) >> 24);
    SPIM_FLASH_WriteByte((Bytes_Address & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((Bytes_Address& 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(Bytes_Address & 0xFF);
	SPIM_FLASH_WriteByte(0);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    SPIM_FLASH_ClearRxBuffer();
    SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));  
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    while(SPIM_FLASH_GetRxBufferSize() > 0)
    {
        *pBuffer = SPIM_FLASH_ReadByte();
    }
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_ReadBytes(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    uint32_t i;
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;
    
    Ctrl_MEM_SS_Write(0);
	SPIM_FLASH_WriteByte(0x0B);
	if(w25qxx.ID>=W25Q256)
	SPIM_FLASH_WriteByte((ReadAddr & 0xFF000000) >> 24);
    SPIM_FLASH_WriteByte((ReadAddr & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((ReadAddr& 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(ReadAddr & 0xFF);
	SPIM_FLASH_WriteByte(0);
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    SPIM_FLASH_ClearRxBuffer();
    for(i=0; i < NumByteToRead; i++)
    {
        SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
        while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    	*pBuffer++ = SPIM_FLASH_ReadByte();
    }
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);	
	w25qxx.Lock=0;
}
//###################################################################################################################
void 	W25qxx_ReadPage(uint8_t *pBuffer,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_PageSize)
{
    uint32_t i;
	while(w25qxx.Lock==1)
		CyDelay(1);
	w25qxx.Lock=1;
	if((NumByteToRead_up_to_PageSize>w25qxx.PageSize)||(NumByteToRead_up_to_PageSize==0))
		NumByteToRead_up_to_PageSize=w25qxx.PageSize;
	if((OffsetInByte+NumByteToRead_up_to_PageSize) > w25qxx.PageSize)
		NumByteToRead_up_to_PageSize = w25qxx.PageSize-OffsetInByte;	
	Page_Address = Page_Address*w25qxx.PageSize+OffsetInByte;
    
    Ctrl_MEM_SS_Write(0);
	SPIM_FLASH_WriteByte(0x0B);
	if(w25qxx.ID>=W25Q256)
		SPIM_FLASH_WriteByte((Page_Address & 0xFF000000) >> 24);
    SPIM_FLASH_WriteByte((Page_Address & 0xFF0000) >> 16);
    SPIM_FLASH_WriteByte((Page_Address& 0xFF00) >> 8);
    SPIM_FLASH_WriteByte(Page_Address & 0xFF);
	SPIM_FLASH_WriteByte(0);
    for(i=0; i < NumByteToRead_up_to_PageSize; i++)
    {
        SPIM_FLASH_WriteByte(W25QXX_DUMMY_BYTE);
    	*pBuffer++ = SPIM_FLASH_ReadByte();
    }
    
    while(!(SPIM_FLASH_ReadTxStatus() & SPIM_FLASH_STS_SPI_DONE));
    CyDelayUs(DELAY_W25QXX);
    Ctrl_MEM_SS_Write(1);
    
	w25qxx.Lock=0;
}
//###################################################################################################################
void 	W25qxx_ReadSector(uint8_t *pBuffer,uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_SectorSize)
{	
	if((NumByteToRead_up_to_SectorSize>w25qxx.SectorSize)||(NumByteToRead_up_to_SectorSize==0))
		NumByteToRead_up_to_SectorSize=w25qxx.SectorSize;
	if(OffsetInByte>=w25qxx.SectorSize)
	{
		#if (_W25QXX_DEBUG==1)
		printf("---w25qxx ReadSector Faild!\r\n");
		CyDelay(100);
		#endif	
		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToRead;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToRead_up_to_SectorSize) > w25qxx.SectorSize)
		BytesToRead = w25qxx.SectorSize-OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_SectorSize;	
	StartPage = W25qxx_SectorToPage(Sector_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_ReadPage(pBuffer,StartPage,LocalOffset,BytesToRead);
		StartPage++;
		BytesToRead-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToRead>0);
}
//###################################################################################################################
void 	W25qxx_ReadBlock(uint8_t* pBuffer,uint32_t Block_Address,uint32_t OffsetInByte,uint32_t	NumByteToRead_up_to_BlockSize)
{
	if((NumByteToRead_up_to_BlockSize>w25qxx.BlockSize)||(NumByteToRead_up_to_BlockSize==0))
		NumByteToRead_up_to_BlockSize=w25qxx.BlockSize;
	if(OffsetInByte>=w25qxx.BlockSize)
	{	
		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToRead;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToRead_up_to_BlockSize) > w25qxx.BlockSize)
		BytesToRead = w25qxx.BlockSize-OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_BlockSize;	
	StartPage = W25qxx_BlockToPage(Block_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_ReadPage(pBuffer,StartPage,LocalOffset,BytesToRead);
		StartPage++;
		BytesToRead-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToRead>0);	
}
//###################################################################################################################
