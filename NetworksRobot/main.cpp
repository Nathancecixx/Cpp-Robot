#include <iostream>

#include "crow_all.h"
#include "PktDef.h"
#include "MySocket.h"

MySocket* udpSocket = nullptr;
std::string robotIP = "";
int robotPort = 0;

int pktCount = 0;


struct TelemData {
    uint16_t LastPktCounter;
    uint16_t CurrentGrade;
    uint16_t HitCount;
    uint8_t LastCmd;
    uint8_t LastCmdValue;
    uint8_t LastCmdSpeed;
};


int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        //Return the robot control UI
        std::ifstream file("../public/robot_control_gui.html");
        if (!file.is_open()) 
            return crow::response(500, "Failed to open robot_control_gui.html");
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return crow::response{buffer.str()};
    });

    CROW_ROUTE(app, "/connect").methods("POST"_method)
    ([](const crow::request& req){
        //Load ip and port from body
        auto json = crow::json::load(req.body);
        robotIP = json["ip"].s();
        robotPort = json["port"].i();
        //delete old socket and create new one
        if (udpSocket) delete udpSocket;
        udpSocket = new MySocket(CLIENT, robotIP, robotPort, UDP, 1024);
        return crow::response(200, "Connected");
    });

    CROW_ROUTE(app, "/telecommand/").methods("PUT"_method)
    ([](const crow::request& req){
        //Ensure socket is setup
        if (!udpSocket) {
            return crow::response(500, "UDP socket not initialized. Call /connect first.");
        }

        //Get the direction and duration from body
        auto json = crow::json::load(req.body);
        std::string direction = json["direction"].s();
        int duration = json["duration"].i();

        //Set the correcy body
        DriveBody body;
        if (direction == "FORWARD") body.Direction = FORWARD;
        else if (direction == "BACKWARD") body.Direction = BACKWARD;
        else if (direction == "LEFT") body.Direction = LEFT;
        else if (direction == "RIGHT") body.Direction = RIGHT;
        body.Duration = duration;
        body.Speed = 90;

        //Create packet to send
        PktDef pkt;
        pkt.SetCmd(DRIVE);
        pkt.SetPktCount(++pktCount);
        pkt.SetBodyData(reinterpret_cast<char*>(&body), sizeof(body));
        char* buffer = pkt.GenPacket();
        pkt.CalcCRC();
        udpSocket->SendData(buffer, pkt.GetLength());

        return crow::response(200, "Command sent");
    });

    CROW_ROUTE(app, "/telementry_request/").methods("GET"_method)
    ([](const crow::request& req, crow::response& res){
        //Send response packet
        PktDef pkt;
        pkt.SetCmd(RESPONSE);
        pkt.SetPktCount(++pktCount);
        pkt.SetBodyData(nullptr, 0);
        char* buffer = pkt.GenPacket();
        pkt.CalcCRC();
        udpSocket->SendData(buffer, pkt.GetLength());


        //Create recieve buffer
        char recvBuffer[1024];
        int bytes = udpSocket->GetData(recvBuffer);
        PktDef response;

        //Skip over any ACK 
        do {
            bytes = udpSocket->GetData(recvBuffer);
            if (bytes < HEADERSIZE + CRCSIZE){
                res.code = 500;
                return res.end();
            }
            response = PktDef(recvBuffer);
        } while(response.GetAck());

        //Check command type of incoming packet
        if(response.GetCmd() == RESPONSE){
            std::cout << "Status Recieved!!!" << std::endl;
        }

        //Make sure recieved packet is correct size
        int needed = response.GetLength();
        if(bytes < needed){
            res.code = 500;
            res.write(R"({"error":"Incomplete packet"})");
            res.set_header("Content-Type","application/json");
            res.end();
            return;
        }

        //Make sure body data is there and the correct size
        auto body = response.GetBodyData();
        int bodySize = needed - (HEADERSIZE + CRCSIZE);
        if(!body || bodySize < sizeof(TelemData)){
            res.code = 500;
            res.write(R"({"error":"Incomplete packet"})");
            res.set_header("Content-Type","application/json");
            res.end();
            return;
        }
        
        //Copy data into telem struct
        TelemData data;
        std::memcpy(&data, body, sizeof(TelemData));
        std::cout << "Got telemetry: lastPkt=" << data.LastPktCounter << std::endl;

        res.end();
    });

    app.port(18080).multithreaded().run();
}
