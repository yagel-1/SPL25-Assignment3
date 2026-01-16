#pragma once

#include "../include/ConnectionHandler.h"
#include <event.h>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler &connectionHandler;
    std::map<std::string, int> topicToSubscriptionId;
    int subscriptionIdCounter = 1;
    int receiptIdCounter = 1;
    int disconnectReceiptId = -1;
    bool loggedOut = false;

    std::string user;
    std::map<std::string, std::vector<Event>> userToEvents;

    void handleJoin(const std::string & line);   
    void handleExit(const std::string & line);   
    void handleReport(const std::string & line); 
    void handleSummary(const std::string & line);  
    void handleLogOut();

public:
    StompProtocol(ConnectionHandler & connectionHandler);
    void handleLogin(const std::string & line);
    void readKeyBoard();
    void readSocket();

    const std::string &getUser() const;
};

