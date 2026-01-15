#pragma once

#include "../include/ConnectionHandler.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler &connectionHandler;
    std::map<std::string, int> topicToSubscriptionId;
    int subscriptionIdCounter = 1;
    int receiptIdCounter = 1;

    void handleJoin(const std::string & line);   
    void handleExit(const std::string & line);   
    void handleReport(const std::string & line); 
    void handleSummary(const std::string & line);  

public:
    StompProtocol(ConnectionHandler & connectionHandler);
    void handleLogin(const std::string & line);
    void readKeyBoard();
    void readSocket();
};

