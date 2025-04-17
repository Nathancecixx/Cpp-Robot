#pragma once
#include <iostream>
#include <cstring>
#include <cstdint> 

#define FORWARD    1
#define BACKWARD   2
#define RIGHT	   3
#define LEFT	   4

#define HEADERSIZE sizeof(Header)
#define CRCSIZE    1

#pragma pack(push, 1) 
struct Header {
	uint16_t PktCount; //2 bytes (current count)

	uint8_t Flags;	//bit‑0 Drive
					//bit‑1 Status
					//bit‑2 Sleep
					//bit‑3 Ack
					//bit‑4..7 = 0

	uint8_t Length; //1 byte (total packet bytes)
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DriveBody {
	unsigned char Direction;
	unsigned char Duration;
	unsigned char Speed;
};
#pragma pack(pop)


constexpr uint8_t DRIVE_BIT  = 0x01;
constexpr uint8_t STATUS_BIT = 0x02;
constexpr uint8_t SLEEP_BIT  = 0x04;
constexpr uint8_t ACK_BIT    = 0x08;

enum CmdType { DRIVE, SLEEP, RESPONSE };

class PktDef {
private:
	struct CmdPacket {
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