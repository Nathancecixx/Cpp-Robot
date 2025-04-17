#define _CRT_SECURE_NO_WARNINGS 
#include "PktDef.h"


//Helper function to sum bits in a buffer
int PktDef::SumPacketBits(char* src, int size) {
	int BitCount = 0;
	for (int i = 0; i < size; i++) {
		unsigned char byte = src[i];
		while (byte) {
			BitCount += byte & 1;
			byte >>= 1;
		}
	}
	return BitCount;
}


//C
PktDef::PktDef() {
	this->packet.header.PktCount = 0;
	this->packet.header.Flags = 0;
	this->packet.header.Length = 0;
	this->packet.Data = nullptr;
	this->RawBuffer = nullptr;
	this->packet.CRC = 0;
}

PktDef::PktDef(char* src) : RawBuffer(nullptr) {
	//Extract header from buffer
	std::memcpy(&this->packet.header, src, HEADERSIZE);

	//Copy body into buffer
	int BodySize = this->packet.header.Length - HEADERSIZE - CRCSIZE;
	if (BodySize > 0) {
		this->packet.Data = new char[BodySize];
		std::memcpy(this->packet.Data, src + HEADERSIZE, BodySize);
	}
	else {
		this->packet.Data = nullptr;
	}

	//Copy Crc into buffer
	std::memcpy(&this->packet.CRC, src + HEADERSIZE + BodySize, CRCSIZE);
}

//R
CmdType PktDef::GetCmd() {
	return  (this->packet.header.Flags & DRIVE_BIT)  ? DRIVE   :
		    (this->packet.header.Flags & SLEEP_BIT)  ? SLEEP   :
													  RESPONSE ;
}
bool PktDef::GetAck() {
	return packet.header.Flags & ACK_BIT;
}
int PktDef::GetLength() {
	return this->packet.header.Length;
}
char* PktDef::GetBodyData() {
	return this->packet.Data;
}
int PktDef::GetPktCount() {
	return this->packet.header.PktCount;
}

//U
void PktDef::SetCmd(CmdType type) {
	//Reset current header command type
	this->packet.header.Flags &= ~(DRIVE_BIT|STATUS_BIT|SLEEP_BIT);

	//Set selected command type
    if 		(type == DRIVE)    	packet.header.Flags |= DRIVE_BIT;
    else if (type == SLEEP) 	packet.header.Flags |= SLEEP_BIT;
    else                		packet.header.Flags |= STATUS_BIT;
}

void PktDef::SetBodyData(char* source, int size) {
	//Delete old data if any
	if (this->packet.Data)
		delete[] this->packet.Data;

	//to avoid negatives
	if (size < 0)
		size = 0;
	//Allocate space
	this->packet.Data = new char[size];
	//Copy data
	std::memcpy(this->packet.Data, source, size);
	//Set Packet Length
	this->packet.header.Length = HEADERSIZE + size + CRCSIZE;
}

void PktDef::SetPktCount(int count) {
	this->packet.header.PktCount = count;
}

bool PktDef::CheckCRC(char* source, int size) {
	int BitSum = this->SumPacketBits(source, size);

	if (BitSum == this->packet.CRC)
		return true;
	return false;

}

void PktDef::CalcCRC() {
	int PacketSize = this->packet.header.Length - CRCSIZE;
	int BitSum = this->SumPacketBits(this->RawBuffer, PacketSize);
	//Store as 1 byte value, wrap if >255
    packet.CRC = static_cast<unsigned char>(BitSum & 0xFF);
    //Write it into the already serialised buffer
    RawBuffer[PacketSize] = packet.CRC;
}

char* PktDef::GenPacket() {
	//Get the size of the body
    int BodySize = (packet.header.Flags & DRIVE_BIT) ? sizeof(DriveBody) : 0;
	int TotalSize = HEADERSIZE + BodySize + CRCSIZE;

	//Allocate space for full packet
	if (this->RawBuffer)
		delete[] this->RawBuffer;
	this->RawBuffer = new char[TotalSize];

	//Copy Header into buffer
	std::memcpy(this->RawBuffer, &this->packet.header, HEADERSIZE);

	//Copy Body into buffer
	if (BodySize > 0 && this->packet.Data)
		std::memcpy(this->RawBuffer + HEADERSIZE, this->packet.Data, BodySize);

	//Copy Crc into buffer
	std::memcpy(this->RawBuffer + HEADERSIZE + BodySize, &(this->packet.CRC), CRCSIZE);

	return this->RawBuffer;
}

//D
PktDef::~PktDef() {
	if (this->RawBuffer)
		delete[] this->RawBuffer;
	if (this->packet.Data)
		delete[] this->packet.Data;
}