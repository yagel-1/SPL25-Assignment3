#include <StompProtocol.h>

StompProtocol::StompProtocol(ConnectionHandler & connectionHandler) : connectionHandler(connectionHandler) {}

void StompProtocol::readKeyBoard() {
    while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
		std::string line(buf);
        size_t pos = line.find(' ');
        std::string command = (pos == std::string::npos) ? line : line.substr(0, pos);
        if (command == "join") {
            handleJoin(line);
        } else if (command == "exit") {
            handleExit(line);
        } else if (command == "report") {
            handleReport(line);
        } else if (command == "summary") {
            handleSummary(line);
        }
    }
}



void StompProtocol::handleLogin(const std::string & line){
    std::string connectMsg = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il";
    std::string withoutLogin = line.substr(line.find(' '));
    std::string user = withoutLogin.substr(withoutLogin.find(' ') + 1);
    size_t startPos = user.find(' ');
    std::string login = user.substr(0, startPos);
    std::string passcode = user.substr(startPos + 1);
    connectMsg += "\nlogin:" + login;
    connectMsg += "\npasscode:" + passcode;
    connectMsg += "\n\n\0";
    connectionHandler.sendFrameAscii(connectMsg, '\0');
}

void StompProtocol::handleJoin(const std::string & line){
    std::string subscribeMsg = "SUBSCRIBE\n";
    size_t pos = line.find(' ');
    std::string topic = line.substr(pos + 1);
    subscribeMsg += "destination:" + topic + "\n";
    subscribeMsg += "id:" + std::to_string(subscriptionIdCounter) + "\n";
    subscribeMsg += "receipt:" + std::to_string(receiptIdCounter) + "\n\n\0";

    topicToSubscriptionId[topic] = subscriptionIdCounter;

    subscriptionIdCounter++;
    receiptIdCounter++;
    connectionHandler.sendFrameAscii(subscribeMsg, '\0');
}   

void StompProtocol::handleExit(const std::string & line){
    std::string unsubscribeMsg = "UNSUBSCRIBE\n";
    size_t pos = line.find(' ');
    std::string topic = line.substr(pos + 1);
    unsubscribeMsg += "id:" + std::to_string(topicToSubscriptionId[topic]) + "\n";
    unsubscribeMsg += "receipt:" + std::to_string(receiptIdCounter) + "\n\n\0";

    topicToSubscriptionId.erase(topic);

    receiptIdCounter++;
    connectionHandler.sendFrameAscii(unsubscribeMsg, '\0');
}

void StompProtocol::handleReport(const std::string & line){

}

void StompProtocol::handleSummary(const std::string & line){  
}