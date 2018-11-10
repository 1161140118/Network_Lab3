/*
* THIS FILE IS FOR IP TEST
*/
// system support
#include "sysInclude.h"
extern void ip_DiscardPkt(char* pBuffer,int type);
extern void ip_SendtoLower(char*pBuffer,int length);
extern void ip_SendtoUp(char *pBuffer,int length);
extern unsigned int getIpv4Address();
// implemented by students
int stud_ip_recv(char *pBuffer,unsigned short length)
{
	// get ip head message
    int version = pBuffer[0] >> 4;
    int headlength = pBuffer[0] & 0xf;
    int timetolive = (int)pBuffer[8];
    int headerChecksum = ntohs(*(short unsigned int*)(pBuffer+10));
    int destinationAddress = ntohl(*(unsigned int*)(pBuffer+16));
    
	// check version
    if (version != 4)						
    {
        ip_DiscardPkt(pBuffer,STUD_IP_TEST_VERSION_ERROR);
		return 1;
    }
    
	// check headIHL
    if (headlength < 5)						
    {
        ip_DiscardPkt(pBuffer,STUD_IP_TEST_HEADLEN_ERROR);
		return 1;
    }
    
	// check TTL
    if (timetolive == 0)					 
    {
        ip_DiscardPkt(pBuffer,STUD_IP_TEST_TTL_ERROR);
		return 1;
    }
    			
	// check target destination							 
    if (destinationAddress != getIpv4Address() && destinationAddress != 0xffffff)
    {
        ip_DiscardPkt(pBuffer,STUD_IP_TEST_DESTINATION_ERROR);
		return 1;
    }
    
	// check checksum
	unsigned short sum = 0; 
	unsigned short tempNum = 0; 
	for (int i = 0; i < headlength * 2; i++){
		tempNum = ((unsigned char)pBuffer[i*2]<<8) + (unsigned char)pBuffer[i*2 + 1];
		if (0xffff - sum < tempNum)
			sum = sum + tempNum + 1;
		else
			sum = sum + tempNum;
	}
	if (sum != 0xffff){
		ip_DiscardPkt(pBuffer, STUD_IP_TEST_CHECKSUM_ERROR);
		return 1;
	}
	
	ip_SendtoUp(pBuffer,length);
	return 0;
}
int stud_ip_Upsend(char *pBuffer,unsigned short len,unsigned int srcAddr,
				   unsigned int dstAddr,byte protocol,byte ttl)
{
    char *sendBuffer = new char(len + 20);
    memset(sendBuffer, 0, len+20);
    
    sendBuffer[0] = 0x45;
    unsigned short int totallen =  htons(len + 20);
    memcpy(sendBuffer + 2, &totallen, sizeof(unsigned short int));
    sendBuffer[8] = ttl;
    sendBuffer[9] = protocol;
    
   	unsigned int src = htonl(srcAddr);
    unsigned int dis = htonl(dstAddr);
    memcpy(sendBuffer + 12, &src, sizeof(unsigned int));
    memcpy(sendBuffer + 16, &dis, sizeof(unsigned int));
   
	// check sum
	unsigned short sum = 0; 
	unsigned short tempNum = 0; 
	unsigned short localCheckSum = 0;

	//checksum
	for (int i = 0; i < 10; i++){
		tempNum = ((unsigned char)sendBuffer[i*2]<<8) + (unsigned char)sendBuffer[i*2 + 1];
		if (0xffff - sum < tempNum)
			sum = sum + tempNum + 1;
		else
			sum = sum + tempNum;
	}
	localCheckSum = htons(0xffff - sum);  
	//localCheckSum = htons(0xffff - (unsigned short int)sum);
	
	memcpy(sendBuffer + 10, &localCheckSum, sizeof(unsigned short int));
	memcpy(sendBuffer + 20, pBuffer, len);	
	
	// send
	ip_SendtoLower(sendBuffer,len+20);
	return 0;
}
