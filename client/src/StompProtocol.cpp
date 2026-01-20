#include <StompProtocol.h>
#include <event.h>
#include <fstream>

StompProtocol::StompProtocol(ConnectionHandler & connectionHandler) :
      connectionHandler(connectionHandler),
      mtx(),
      topicToSubscriptionId(), 
      subscriptionIdCounter(1), 
      receiptIdCounter(1), 
      disconnectReceiptId(-1), 
      loggedIn(true),
      socketClose(false),
      user(""), 
      userToEvents(),
      unUsedInput("")
{}

void StompProtocol::readKeyBoard() {
    while (1) {
        if (socketClose){
            break;
        }
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
		std::string line(buf);

        if (socketClose){
            unUsedInput = line;
            break;
        }

        size_t pos = line.find(' ');
        std::string command = (pos == std::string::npos) ? line : line.substr(0, pos);
        if (command == "login"){
            if (loggedIn){
                safePrint("The client is already logged in, log out before trying again");
            }
            else{
                handleLogin(line);
            }
        }
        else if (command == "join" && loggedIn) {
            handleJoin(line);
        } 
        else if (command == "exit" && loggedIn) {
            handleExit(line);
        } 
        else if (command == "report" && loggedIn) {
            handleReport(line);
        }
        else if (command == "summary" && loggedIn) {
            handleSummary(line);
        }
        else if (command == "logout" && loggedIn){
            handleLogOut();
            return;
        }
        else{
            if(!loggedIn){
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
    std::string withoutLogin = line.substr(line.find(' ') + 1);
    std::string userAndPass = withoutLogin.substr(withoutLogin.find(' ') + 1);
    size_t startPos = userAndPass.find(' ');
    std::string login = userAndPass.substr(0, startPos);
    std::string passcode = userAndPass.substr(startPos + 1);

    this->user = login;

    connectMsg += "\nlogin:" + login;
    connectMsg += "\npasscode:" + passcode;
    connectMsg += "\n\n";
    bool sent = sendToHandler(connectMsg);
    if (!sent) {
        loggedIn = false;
        return;
    }
}

void StompProtocol::handleJoin(const std::string & line){
    std::string subscribeMsg = "SUBSCRIBE\n";
    size_t pos = line.find(' ');
    std::string topic = line.substr(pos + 1);
    subscribeMsg += "destination:" + topic + "\n";
    subscribeMsg += "id:" + std::to_string(subscriptionIdCounter) + "\n";
    subscribeMsg += "receipt:" + std::to_string(receiptIdCounter) + "\n\n";

    topicToSubscriptionId[topic] = subscriptionIdCounter;

    subscriptionIdCounter++;
    receiptIdCounter++;
    bool sent = sendToHandler(subscribeMsg);
    if (!sent) {
        loggedIn = false;
        return;
    }
}   

void StompProtocol::handleExit(const std::string & line){
    std::string unsubscribeMsg = "UNSUBSCRIBE\n";
    size_t pos = line.find(' ');
    std::string topic = line.substr(pos + 1);

    if (topicToSubscriptionId.find(topic) == topicToSubscriptionId.end()) {
        safePrint("Error: You are not subscribed to " + topic);
        return;
    }

    unsubscribeMsg += "id:" + std::to_string(topicToSubscriptionId[topic]) + "\n";
    unsubscribeMsg += "receipt:" + std::to_string(receiptIdCounter) + "\n\n";

    topicToSubscriptionId.erase(topic);

    receiptIdCounter++;
    bool sent = sendToHandler(unsubscribeMsg);
    if (!sent) {
        loggedIn = false;
        return;
    }
}

void StompProtocol::handleReport(const std::string & line){
    try{
        names_and_events namesAndEvents = parseEventsFile(line.substr(line.find(' ') + 1));
        std::string teamAName = namesAndEvents.team_a_name;
        std::string teamBName = namesAndEvents.team_b_name;
        std::vector<Event> events = namesAndEvents.events;

        std::sort(events.begin(), events.end(), [](const Event & a, const Event & b) {
            return a.get_time() < b.get_time();
        });

        for (const Event & event : namesAndEvents.events) {
            std::string sendEventMsg = "SEND\n";
            sendEventMsg += "destination:/" + teamAName + "_" + teamBName + "\n";
            sendEventMsg += "file: " + line.substr(line.find_last_of('/') + 1, line.find('.') - line.find_last_of('/') - 1) + "\n\n";
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

            sendEventMsg += "description: " + event.get_discription() + "\n";

            bool sent = sendToHandler(sendEventMsg);
            if (!sent) {
                loggedIn = false;
                break;
            }
        }
    }
    catch (const std::exception& e) {
        safePrint("Error: Failed to parse events file. " + std::string(e.what()));
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

    std::vector<Event> allEvents = getEventsList(userName);    
    
    std::vector<Event> events;
    for (const Event& ev : allEvents) {
        if (ev.get_team_a_name() == teamAName && ev.get_team_b_name() == teamBName) {
            events.push_back(ev);
        }
    }
    if (events.empty()) {
        safePrint("No events found for user " + userName + " in game " + gameName);
        outFile.close();
        return;
    }

    std::sort(events.begin(), events.end(), [](const Event & a, const Event & b) {
        return a.get_time() < b.get_time();
    });
    std::map<std::string, std::string> game_stats_list;
    std::map<std::string, std::string> team_a_stats_list;
    std::map<std::string, std::string> team_b_stats_list;

    for (const Event & event : events) {
        for (const auto& pair : event.get_game_updates()) {
            game_stats_list[pair.first] = pair.second;
        }
        for (const auto& pair : event.get_team_a_updates()) {
            team_a_stats_list[pair.first] = pair.second;
        }
        for (const auto& pair : event.get_team_b_updates()) {
            team_b_stats_list[pair.first] = pair.second;
        }
    }

    outFile << teamAName << " vs " << teamBName << "\n";
    outFile << "Game stats:\n";
    outFile << "General stats:\n";
    for (const auto & pair : game_stats_list) {
        outFile << pair.first << ": " << pair.second << "\n";
    }
    outFile << teamAName << " stats:\n";
    for (const auto & pair : team_a_stats_list) {
        outFile << pair.first << ": " << pair.second << "\n";
    }
    outFile << teamBName << " stats:\n";
    for (const auto & pair : team_b_stats_list) {
        outFile << pair.first << ": " << pair.second << "\n";
    }
    outFile << "Game event reports:\n";
    for (const Event & event : events) {
        outFile << event.get_time() << " - " << event.get_name() << ":\n\n";
        outFile << event.get_discription() << "\n\n";
    }
    outFile.close();
}

void StompProtocol::handleLogOut(){
    loggedIn = false;
    std::string disconnectMsg = "DISCONNECT\nreceipt:" + std::to_string(receiptIdCounter) + "\n\n";
    disconnectReceiptId = receiptIdCounter;
    receiptIdCounter++;
    bool sent = sendToHandler(disconnectMsg);
    if (!sent) {
        loggedIn = false;
        socketClose = true;
        return;
    }
}

void StompProtocol::readSocket() {
    while (loggedIn) {
        std::string frame;
        if (!connectionHandler.getFrameAscii(frame, '\0')) {
            socketClose = true;
            loggedIn = false;
            break;
        }
        if (frame.empty()){
            continue;
        }
        
        if(frame.substr(0,frame.find('\n')) == "CONNECTED"){
            safePrint("Login successful");
            continue;
        }
        if(frame.substr(0,frame.find('\n')) == "RECEIPT"){
            size_t posColon = frame.find(":");
            int receiptId = std::stoi(frame.substr(posColon+1, frame.find('\n') - (posColon+1)));
            if (receiptId == disconnectReceiptId) {
                std::lock_guard<std::mutex> lock(mtx);
                connectionHandler.close();
                socketClose = true;
                break;
            }   
        }
        if(frame.substr(0,frame.find('\n')) == "ERROR"){
            safePrint(frame);
            std::lock_guard<std::mutex> lock(mtx);
            loggedIn = false;
            connectionHandler.close();
            socketClose = true;
            break;
        }
        if(frame.substr(0,frame.find('\n')) == "MESSAGE"){
            std::string body = frame.substr(frame.find("\n\n") + 2); 
            std::string user = body.substr(body.find(' ') +1, body.find("\n") - body.find(' ') -1);
            body = body.substr(body.find("\n") +1);
            Event event(body);
            putEvent(user, event);
        }
    }
}

const std::vector<Event> StompProtocol::getEventsList(const std::string& userName) const{
    std::lock_guard<std::mutex> lock(mtx);
    if (userToEvents.find(userName) != userToEvents.end()) {
        return userToEvents.at(userName);
    }
    return std::vector<Event>();
}

void StompProtocol::putEvent(const std::string & userName, const Event & event){
    std::lock_guard<std::mutex> lock(mtx);
    userToEvents[userName].push_back(event);
}

bool StompProtocol::sendToHandler(std::string msg){
    std::lock_guard<std::mutex> lock(mtx);
    if (connectionHandler.isOpen()){
        if (!connectionHandler.sendFrameAscii(msg, '\0')) {
            socketClose = true;
            return false;
        }
        return true;
    }
    else{
        socketClose = true;
        return false;
    }
}

bool StompProtocol::hasUnUsedInput() const {
    return !unUsedInput.empty();
}

const std::string &StompProtocol::getUnUsedInput() const {
    return unUsedInput;
}