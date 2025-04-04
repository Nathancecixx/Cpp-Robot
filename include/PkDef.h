#pragma once
#include <iostream>
#include <cstring>

#define FORWARD    1
#define BACKWARD   2
#define RIGHT	   3
#define LEFT	   4

#define HEADERSIZE 30
#define CRCSIZE    1


struct Header{
	unsigned int PktCount : 16;
	unsigned int Drive : 1;
	unsigned int Status : 1;
	unsigned int Sleep : 1;
	unsigned int Ack : 1;
	unsigned int Padding : 4;
	unsigned int Length : 16;
};

struct DriveBody{
	unsigned char Direction;
	unsigned char Duration;
	unsigned char Speed;
};


enum CmdType { DRIVE, SLEEP, RESPONSE };

class PktDef {
private:
	struct CmdPacket{
		Header header;
		char* Data;
		char CRC;
	};
	CmdPacket packet;
	char* RawBuffer;

public:
	//C
	PktDef();
	PktDef(char*);

	//R
	CmdType GetCmd();
	bool GetAck();
	int GetLength();
	char* GetBodyData();
	int GetPktCount();

	//U
	void SetBodyData(char*, int);
	void SetPktCount(int);
	void SetCmd(CmdType);

	bool CheckCRC(char*, int);
	void CalcCRC();
	char* GenPacket();

	//D
	~PktDef();

private:
	int SumPacketBits(char*, int);

};