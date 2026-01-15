#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include <iostream>
#include <thread>

/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/

bool validLogin(const std::string & line, std::string & host, short & port) {
	size_t pos = line.find(' ');
	std::string command = (pos == std::string::npos) ? line : line.substr(0, pos);
	if (command == "login") {
		size_t pos1 = line.find(' ', pos + 1);
		if (pos1 == std::string::npos) {
			return false;
		}
		std::string hostAndPort = line.substr(pos + 1, pos1 - pos - 1);
		size_t posHostAndPort = hostAndPort.find(':');
		if (posHostAndPort != std::string::npos) {
			host = hostAndPort.substr(0, posHostAndPort);
			port = std::stoi(hostAndPort.substr(posHostAndPort + 1));
		}
		size_t pos2 = line.find(' ', pos1 + 1);
		if (pos2 != std::string::npos) {
			return false;
		}
		size_t pos3 = line.find(' ', pos2 + 1);
		if (pos3 != std::string::npos) {
			return false;
		}
		return true;
	}
	return false;
}

int main (int argc, char *argv[]) {
	std::string host;
	short port;
	std::string line;
    while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
		line = std::string(buf);
		if (validLogin(line, host, port)) {
			break;
		}
		else{
			std::cout << "Invalid login command. Please use: login <host> <port>" << std::endl;
		}
	}
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
	StompProtocol stompProtocol(connectionHandler);
	stompProtocol.handleLogin(line);
	std::thread threadKeyboard(&StompProtocol::readKeyBoard, stompProtocol, std::ref(connectionHandler));
	std::thread threadSocket(&StompProtocol::readSocket, stompProtocol, std::ref(connectionHandler));
	
    threadKeyboard.join();
	threadSocket.join();

	return 0;
}