#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "crow_all.h"
#include "PktDef.h"
#include "MySocket.h"

MySocket* udpSocket = nullptr;
std::string robotIP = "";
int robotPort = 0;

int pktCount = 0;

//Struct to hold telem data
#pragma pack(push,1)
struct TelemData {
    uint16_t LastPktCounter;
    uint16_t CurrentGrade;
    uint16_t HitCount;
    uint8_t LastCmd;
    uint8_t LastCmdValue;
    uint8_t LastCmdSpeed;
};
#pragma pack(pop) 

//Helper function to read files into string buffers
std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        return "404 Not Found";
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([](){
        //Return the robot control UI
        crow::response res(readFile("../public/robot_control_gui.html"));
        res.set_header("Content-Type", "text/html");
        return res;

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
        std::string cmd = json["direction"].s();
        int durationVal = json.has("duration") ? json["duration"].i() : 0;
        int speedVal    = json.has("speed")    ? json["speed"].i()    : 90;

        PktDef pkt;
        pkt.SetPktCount(++pktCount);
        //Set the correcy body
        DriveBody body;
        if (cmd == "SLEEP") {
            pkt.SetCmd(SLEEP);
            pkt.SetBodyData(nullptr, 0);
        } else {
            DriveBody body{};
            if      (cmd == "FORWARD")  body.Direction = FORWARD;
            else if (cmd == "BACKWARD") body.Direction = BACKWARD;
            else if (cmd == "LEFT")     body.Direction = LEFT;
            else if (cmd == "RIGHT")    body.Direction = RIGHT;
            else
                return crow::response(400, "Unknown direction");
    
            body.Duration = static_cast<uint8_t>(durationVal);
            body.Speed    = static_cast<uint8_t>(speedVal);
    
            pkt.SetCmd(DRIVE);
            pkt.SetBodyData(reinterpret_cast<char*>(&body), sizeof(body));
        }
        body.Duration = durationVal;
        body.Speed = speedVal;

        //Create packet to send
        char* buffer = pkt.GenPacket();
        pkt.CalcCRC();
        udpSocket->SendData(buffer, pkt.GetLength());

        return crow::response(200, "Command sent");
    });

    CROW_ROUTE(app, "/telementry_request/").methods("GET"_method)
    ([&](const crow::request&, crow::response& res) {
        if (!udpSocket) { res.code = 500; return res.end(); }

        //Send status Request
        PktDef req;
        req.SetCmd(RESPONSE);
        req.SetPktCount(++pktCount);
        req.SetBodyData(nullptr, 0);
        char* out = req.GenPacket();
        req.CalcCRC();
        udpSocket->SendData(out, req.GetLength());
    
        //Read packets until we get response
        char rxBuf[1024];
        int  nBytes;
        std::unique_ptr<PktDef> pkt;
        while (true) {
            nBytes = udpSocket->GetData(rxBuf);
            if (nBytes < HEADERSIZE + CRCSIZE) { 
                res.code = 500; 
                return res.end(); 
            }
            pkt.reset(new PktDef(rxBuf));
            if (!pkt->GetAck()) break;
        }
    
        //Ensure command is response
        if (pkt->GetCmd() != RESPONSE) { 
            res.code = 400; 
            return res.end(); 
        }
    
        //Copy packet data into telem struct
        int bodyBytes = pkt->GetLength() - (HEADERSIZE + CRCSIZE);
        if (bodyBytes < static_cast<int>(sizeof(TelemData))) {
            res.code = 500; return res.end();
        }
        TelemData telem;
        std::memcpy(&telem, pkt->GetBodyData(), sizeof(TelemData));
    
        //Parse telem data
        crow::json::wvalue j{
            {"lastPktCounter", telem.LastPktCounter},
            {"currentGrade",   telem.CurrentGrade},
            {"hitCount",       telem.HitCount},
            {"lastCmd",        telem.LastCmd},
            {"lastCmdValue",   telem.LastCmdValue},
            {"lastCmdSpeed",   telem.LastCmdSpeed}
        };

        //Return json with data
        res.set_header("Content-Type", "application/json");
        res.write(j.dump());
        res.end();
    });

    //Route for getting images from server
    CROW_ROUTE(app, "/images/<string>")
    ([](const crow::request& req, crow::response& res, std::string filename) {
        res.set_header("Content-Type", "image/png");
        res.write(readFile("../public/resources/" + filename));
        res.end();
    });
    

    app.port(18080).multithreaded().run();
}
