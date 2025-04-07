#define _CRT_SECURE_NO_WARNINGS 
#include "PktDef.h"



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
	this->packet.header.Ack = 0;
	this->packet.header.Drive = 0;
	this->packet.header.Length = 0;
	this->packet.header.Padding = 0;
	this->packet.header.PktCount = 0;
	this->packet.header.Sleep = 0;
	this->packet.header.Status = 0;
	this->packet.Data = nullptr;
	this->RawBuffer = nullptr;
	this->packet.CRC = 0;
}
PktDef::PktDef(char* src) {
	//Extract header from buffer
	std::memcpy(&this->packet.header, src, HEADERSIZE);

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
	if (this->packet.header.Drive)
		return DRIVE;
	else if (this->packet.header.Sleep)
		return SLEEP;
	else if (this->packet.header.Status)
		return RESPONSE;
	//Default return
	return RESPONSE;
}
bool PktDef::GetAck() {
	if (this->packet.header.Ack == 1)
		return true;
	return false;
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
	this->packet.header.Drive = 0;
	this->packet.header.Sleep = 0;
	this->packet.header.Status = 0;

	switch (type) {
	case DRIVE:
		this->packet.header.Drive = 1;
		break;
	case SLEEP:
		this->packet.header.Sleep = 1;
		break;
	case RESPONSE:
		this->packet.header.Status = 1;
		break;
	}
}
void PktDef::SetBodyData(char* source, int size) {
	//Delete old data if any
	if (this->packet.Data)
		delete[] this->packet.Data;

	//to avoid negatives
	if (size < 0)
		size = 0;
	//Allocate space
	this->packet.Data = new char[size + 1];
	//Copy data
	strcpy(this->packet.Data, source);
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
	this->packet.CRC = BitSum;
}


char* PktDef::GenPacket() {
	//Get the size of the body
	int BodySize = (this->packet.header.Drive) ? sizeof(DriveBody) : 0;
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