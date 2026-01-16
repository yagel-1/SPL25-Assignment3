#include <StompProtocol.h>
#include <event.h>

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

const std::string &StompProtocol::getUser() const{
    return this->user;
}



void StompProtocol::handleLogin(const std::string & line){
    std::string connectMsg = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il";
    std::string withoutLogin = line.substr(line.find(' '));
    std::string user = withoutLogin.substr(withoutLogin.find(' ') + 1);
    size_t startPos = user.find(' ');
    std::string login = user.substr(0, startPos);
    std::string passcode = user.substr(startPos + 1);

    // check against the DB later

    this->user = login;

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
    names_and_events namesAndEvents = parseEventsFile(line.substr(line.find(' ') + 1));
    std::string teamAName = namesAndEvents.team_a_name;
    std::string teamBName = namesAndEvents.team_b_name;
    events = namesAndEvents.events;

    std::sort(events.begin(), events.end(), [](const Event & a, const Event & b) {
        return a.get_time() < b.get_time();
    });

    for (const Event & event : namesAndEvents.events) {
        std::string sendEventMsg = "SEND\n";
        sendEventMsg += "destination:/" + teamAName + "_" + teamBName + "\n\n";
        sendEventMsg += "user: " + getUser() + "\n";
        sendEventMsg += "team a: " + event.get_team_a_name() + "\n";
        sendEventMsg += "team b: " + event.get_team_b_name() + "\n";
        sendEventMsg += "event name: " + event.get_name() + "\n";
        sendEventMsg += "time: " + std::to_string(event.get_time()) + "\n";

        sendEventMsg += "general game updates:\n";
        for (const auto & pair : event.get_game_updates()) {
            sendEventMsg += pair.first + ": " + pair.second + "\n";
        }

        sendEventMsg += "team a updates:\n";
        for (const auto & pair : event.get_team_a_updates()) {
            sendEventMsg += pair.first + ": " + pair.second + "\n";
        }

        sendEventMsg += "team b updates:\n";
        for (const auto & pair : event.get_team_b_updates()) {
            sendEventMsg += pair.first + ": " + pair.second + "\n";
        }

        sendEventMsg += "description: " + event.get_discription() + "\n\0";

        connectionHandler.sendFrameAscii(sendEventMsg, '\0');
    }
}

void StompProtocol::handleSummary(const std::string & line){  
}