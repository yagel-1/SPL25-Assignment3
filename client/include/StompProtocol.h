#pragma once

#include "../include/ConnectionHandler.h"
#include <event.h>
#include <mutex>
#include <atomic>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler &connectionHandler;
    mutable std::mutex mtx;
    std::map<std::string, int> topicToSubscriptionId;
    int subscriptionIdCounter;
    int receiptIdCounter;
    int disconnectReceiptId;
    std::atomic<bool> loggedIn;
    std::atomic<bool> socketClose;
    std::string user;
    std::map<std::string, std::vector<Event>> userToEvents;
    std::string unUsedInput;

    void handleJoin(const std::string & line);   
    void handleExit(const std::string & line);   
    void handleReport(const std::string & line); 
    void handleSummary(const std::string & line);  
    void handleLogOut();
    void safePrint(const std::string& msg);
    const std::vector<Event> getEventsList(const std::string & userName) const;
    void putEvent(const std::string & userName, const Event & event);
    bool sendToHandler(std::string msg);

public:
    StompProtocol(ConnectionHandler & connectionHandler);
    void handleLogin(const std::string & line);
    void readKeyBoard();
    void readSocket();
    const std::string &getUser() const;
    bool hasUnUsedInput() const;
    const std::string &getUnUsedInput() const;
};

