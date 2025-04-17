Authors: Mohammad Aljabrery & Nathan Ceci
CSCN72050 Term Project - Milestone #1: PktDef Class
CSCN72050 Term Project - Milestone #2: MyPacket Class
CSCN72050 Term Project - Milestone #3: WebServer


Building and Running Unit Tests:
1. Open the solution in Visual Studio.
2. Make sure the NetworksRobot project is configured as a library.
	(NetworkRobot Properties -> Configuration -> general -> configuration type -> static library)
3. Build the solution (Build > Build Solution).
4. Open the Test Explorer (Test > Run all tests) and run all tests.
5. All tests should pass, confirming that the PktDef, and MyScoket class meets the project requirements.


Building and Running Web Server:
1. Start your docker container
	- docker run -ti -p 18080:18080 -v $(PWD)\Cpp-Robot\NetworksRobot:/RobotServer webserver_dev bash
2. Cd into the CPP-Robot/build
3. Create make files
	- cmake ..
4. Run build command
	- make
5. Start Server 
	- ./WebServer
6. Open URL in browser
	- localhost:18080

