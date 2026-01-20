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
			try {
				port = std::stoi(hostAndPort.substr(posHostAndPort + 1));
			}
			catch (...){
				std::cout << "Invalid input, can't pars to integer\n Please use: login <host:port> <user> <passcode>" << std::endl;
			}
		}
		else {
			return false;
		}
		
		size_t pos2 = line.find(' ', pos1 + 1);
		if (pos2 == std::string::npos) {
			return false;
		}
		return true;
	}
	return false;
}

int main (int argc, char *argv[]) {
	std::string nextLine = "";

	while (1) {
		std::string host;
		short port;
		std::string line;

		if (nextLine.empty()) {
			const short bufsize = 1024;
			char buf[bufsize];
			std::cin.getline(buf, bufsize);
			line = std::string(buf);
		} else {
			line = nextLine;
			nextLine = "";
		}

		if (!validLogin(line, host, port)) {
			std::cout << "Invalid login command. Please use: login <host:port> <user> <passcode>" << std::endl;
			continue;
		}
    
		ConnectionHandler connectionHandler(host, port);
		if (!connectionHandler.connect()) {
			std::cerr << "Could not connect to server" << std::endl;
			continue;
		}
		StompProtocol stompProtocol(connectionHandler);
		stompProtocol.handleLogin(line);
		std::thread threadKeyboard(&StompProtocol::readKeyBoard, std::ref(stompProtocol));
		std::thread threadSocket(&StompProtocol::readSocket, std::ref(stompProtocol));
		
		threadKeyboard.join();
		threadSocket.join();

		if (stompProtocol.hasUnUsedInput()) {
			nextLine = stompProtocol.getUnUsedInput();
		}
	}
	return 0;
}