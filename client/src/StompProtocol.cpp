#include <StompProtocol.h>
#include <event.h>
#include <fstream>

StompProtocol::StompProtocol(ConnectionHandler & connectionHandler) : connectionHandler(connectionHandler), 
      topicToSubscriptionId(), 
      subscriptionIdCounter(1), 
      receiptIdCounter(1), 
      disconnectReceiptId(-1), 
      loggedOut(false), 
      user(""), 
      userToEvents() 
{}

void StompProtocol::readKeyBoard() {
    while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
		std::string line(buf);
        size_t pos = line.find(' ');
        std::string command = (pos == std::string::npos) ? line : line.substr(0, pos);
        if (command == "join" && !loggedOut) {
            handleJoin(line);
        } 
        else if (command == "exit" && !loggedOut) {
            handleExit(line);
        } 
        else if (command == "report" && !loggedOut) {
            handleReport(line);
        }
        else if (command == "summary" && !loggedOut) {
            handleSummary(line);
        }
        else if (command == "logout" && !loggedOut){
            handleLogOut();
        }
        else{
            if(loggedOut){
                safePrint("You are logged out. No further commands can be processed.");
                break;
            }
            else
               safePrint("Invalid command");
        }
    }
}

void StompProtocol::safePrint(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << msg << std::endl;
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
    std::vector<Event> events = namesAndEvents.events;

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

    std::stringstream ss(line); 
    std::string command, gameName, userName, fileName;
    ss >> command;  
    ss >> gameName;  
    ss >> userName;  
    ss >> fileName;
    std::string teamAName = gameName.substr(0, gameName.find('_'));
    std::string teamBName = gameName.substr(gameName.find('_') + 1);
    std::ofstream outFile(fileName);
    if (!outFile.is_open()) {
        safePrint("Error opening file: " + fileName);
        return;
    }

    std::vector<Event> events = getEventsList(userName);

    std::sort(events.begin(), events.end(), [](const Event & a, const Event & b) {
        return a.get_time() < b.get_time();
    });
    std::vector<std::map<std::string, std::string>> game_stats_list;
    std::vector<std::map<std::string, std::string>> team_a_stats_list;
    std::vector<std::map<std::string, std::string>> team_b_stats_list;
    for (const Event & event : events) {
        game_stats_list.push_back(event.get_game_updates());
        team_a_stats_list.push_back(event.get_team_a_updates());
        team_b_stats_list.push_back(event.get_team_b_updates());
    }
    std::sort(game_stats_list.begin(), game_stats_list.end(), [](const std::map<std::string, std::string> & a, const std::map<std::string, std::string> & b) {
        return a.begin()->first < b.begin()->first;
    });
    std::sort(team_a_stats_list.begin(), team_a_stats_list.end(), [](const std::map<std::string, std::string> & a, const std::map<std::string, std::string> & b) {
        return a.begin()->first < b.begin()->first;
    });
    std::sort(team_b_stats_list.begin(), team_b_stats_list.end(), [](const std::map<std::string, std::string> & a, const std::map<std::string, std::string> & b) {
        return a.begin()->first < b.begin()->first;
    });
    outFile << teamAName << " VS " << teamBName << "\n";
    outFile << "Game stats :\n";
    outFile << "General stats:\n";
    for (const auto & stats : game_stats_list) {
        for (const auto & pair : stats) {
            outFile << pair.first << ": " << pair.second << "\n";
        }
    }
    outFile << teamAName << " stats:\n";
    for (const auto & stats : team_a_stats_list) {
        for (const auto & pair : stats) {
            outFile << pair.first << ": " << pair.second << "\n";
        }
    }
    outFile << teamBName << " stats:\n";
    for (const auto & stats : team_b_stats_list) {
        for (const auto & pair : stats) {
            outFile << pair.first << ": " << pair.second << "\n";
        }
    }
    outFile << "Game event reports:\n";
    for (const Event & event : events) {
        outFile << event.get_time() << " - " << event.get_name() << ":\n\n";
        outFile << event.get_discription() << "\n\n";
    }
    outFile.close();
}

void StompProtocol::handleLogOut(){
    loggedOut = true;
    std::string disconnectMsg = "DISCONNECT\nreceipt:" + std::to_string(receiptIdCounter) + "\n\n\0";
    disconnectReceiptId = receiptIdCounter;
    receiptIdCounter++;
    connectionHandler.sendFrameAscii(disconnectMsg, '\0');
}

void StompProtocol::readSocket() {
    while (!loggedOut) {
        std::string frame;
        if (!connectionHandler.getFrameAscii(frame, '\0')) {
            break;
        }
        safePrint(frame);
        if(frame.substr(0,frame.find('\n')) == "RECEIPT"){
            int receiptId = std::stoi(frame.substr(frame.find(":")+1, frame.find('\n') - frame.find(":")+1));
            if (receiptId == disconnectReceiptId) {
                connectionHandler.close();
                break;
            }   
        }
        if(frame.substr(0,frame.find('\n')) == "ERROR"){
            connectionHandler.close();
            break;
        }
        if(frame.substr(0,frame.find('\n')) == "MESSAGE"){
            std::string body = frame.substr(frame.find("\n\n") +1); 
            std::string user = body.substr(body.find(' ') +1, body.find("\n") - body.find(' ') +1);
            body = body.substr(body.find("\n") +1);
            Event event(body);
            putEvent(user, event);
        }
    }
}

const std::vector<Event> StompProtocol::getEventsList(const std::string& userName) const{
    std::lock_guard<std::mutex> lock(mtx);
    return userToEvents.at(userName);
}

void StompProtocol::putEvent(const std::string & userName, const Event & event){
    std::lock_guard<std::mutex> lock(mtx);
    userToEvents[userName].push_back(event);
}