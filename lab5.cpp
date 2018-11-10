/*
* THIS FILE IS FOR IP FORWARD TEST
*/
#include "sysInclude.h"
#include<map>
 
// system support
extern void fwd_LocalRcv(char *pBuffer, int length);
 
extern void fwd_SendtoLower(char *pBuffer, int length, unsigned int nexthop);
 
extern void fwd_DiscardPkt(char *pBuffer, int type);
 
extern unsigned int getIpv4Address( );
 
// implemented by students
 
// structure route table by map
map<unsigned int,unsigned int> route_table;
 
 
//initialize route table
void stud_Route_Init()
{
	route_table.clear();
	return;
}

// add a new item
void stud_route_add(stud_route_msg *proute)
{
	unsigned int dest = (ntohl(proute->dest))&(0xffffffff<<(32-htonl(proute->masklen)));
	unsigned int next = (ntohl(proute->nexthop));
	route_table.insert(map<unsigned int, unsigned int>::value_type(dest,next));	
	return;
}
 
// routing forward
int stud_fwd_deal(char *pBuffer, int length)
{
	int headlength = pBuffer[0]&0xf;
	int TTL = (int)pBuffer[8];
	int destIp = ntohl(*(unsigned int*)(pBuffer+16));
 
	// send to upper layer protocol
	if(destIp == getIpv4Address())
	{
		fwd_LocalRcv(pBuffer,length);
		return 0;
	}
 	
	// check TTL
	if(TTL<=0)
	{
		fwd_DiscardPkt(pBuffer,STUD_FORWARD_TEST_TTLERROR);
		return 1;
	}
 
	// routing forward
	map<unsigned int,unsigned int>::iterator iter;
	iter = route_table.find(destIp);
	if(iter!=route_table.end())
	{
		char *sendBuffer = new char[length];
		memcpy(sendBuffer,pBuffer,length);

		//TTL - 1
		sendBuffer[8]--;

		// update checksum
		unsigned int sum = 0;
		unsigned short int newCheckSum = 0;
		for(int i = 0; i < 2*headlength; i++)
		{
			if(i!=5)
			{
				sum += ((unsigned char)sendBuffer[2*i]<<8)+((unsigned char)sendBuffer[i*2+1]);
			}
		}
		while((sum&0xffff0000)!=0){
			sum = (sum&0xffff)+((sum>>16)&0xffff);
		}
		newCheckSum = (short unsigned int)sum;
		newCheckSum = htons(~newCheckSum);
		memcpy(sendBuffer+10,&newCheckSum,sizeof(unsigned short int));
		fwd_SendtoLower(sendBuffer,length,iter->second);
		return 0;
	}
	else
	{	// there is no such item.
		fwd_DiscardPkt(pBuffer,STUD_FORWARD_TEST_NOROUTE);
		return 1;
	}
	return 0;
}
