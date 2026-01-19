#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_discription() const
{
    return this->description;
}

Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("")
{
    size_t posTeamA = frame_body.find("team a:");
    size_t posTeamB = frame_body.find("team b:");
    size_t posEventName = frame_body.find("event name:");
    size_t posTime = frame_body.find("time:");
    size_t posGeneralUp = frame_body.find("general game updates:");
    size_t posTeamAUp = frame_body.find("team a updates:");
    size_t posTeamBUp = frame_body.find("team b updates:");
    size_t posDesc = frame_body.find("description:");

    if (posTeamA != std::string::npos) {
        team_a_name = frame_body.substr(posTeamA + 7, frame_body.find('\n', posTeamA) - (posTeamA + 7));
        if (!team_a_name.empty() && team_a_name[0] == ' ') team_a_name.erase(0, 1);
        if (!team_a_name.empty() && team_a_name.back() == '\r') team_a_name.pop_back();
    }
    
    if (posTeamB != std::string::npos) {
        team_b_name = frame_body.substr(posTeamB + 7, frame_body.find('\n', posTeamB) - (posTeamB + 7));
        if (!team_b_name.empty() && team_b_name[0] == ' ') team_b_name.erase(0, 1);
        if (!team_b_name.empty() && team_b_name.back() == '\r') team_b_name.pop_back();
    }
    
    if (posEventName != std::string::npos) {
        name = frame_body.substr(posEventName + 11, frame_body.find('\n', posEventName) - (posEventName + 11));
        if (!name.empty() && name[0] == ' ') name.erase(0, 1);
        if (!name.empty() && name.back() == '\r') name.pop_back();
    }
    
    if (posTime != std::string::npos) {
        std::string timeStr = frame_body.substr(posTime + 5, frame_body.find('\n', posTime) - (posTime + 5));
        try {
            time = std::stoi(timeStr);
        } catch(...) {
            time = 0; 
        }
    }

    if (posGeneralUp != std::string::npos && posTeamAUp != std::string::npos) {
        std::string content = frame_body.substr(posGeneralUp + 21, posTeamAUp - (posGeneralUp + 21));
        game_updates = parseStringToMap(content);
    }

    if (posTeamAUp != std::string::npos && posTeamBUp != std::string::npos) {
        std::string content = frame_body.substr(posTeamAUp + 15, posTeamBUp - (posTeamAUp + 15));
        team_a_updates = parseStringToMap(content);
    }

    if (posTeamBUp != std::string::npos && posDesc != std::string::npos) {
        std::string content = frame_body.substr(posTeamBUp + 15, posDesc - (posTeamBUp + 15));
        team_b_updates = parseStringToMap(content);
    }

    if (posDesc != std::string::npos) {
        description = frame_body.substr(posDesc + 12); 
        while (!description.empty() && (description[0] == ' ' || description[0] == '\n' || description[0] == '\r')) {
            description.erase(0, 1);
        }
    }
}

std::map<std::string, std::string> Event::parseStringToMap(std::string map){
    std::map<std::string, std::string> hashMap;
    std::string remain = map;
    
    while (!remain.empty() && remain[0] == '\n') remain.erase(0, 1);

    while (remain.length() > 0){
        size_t posColon = remain.find(':');
        if (posColon == std::string::npos) break;

        std::string key = remain.substr(0, posColon); 
        
        size_t endLine = remain.find('\n');
        if (endLine == std::string::npos) endLine = remain.length();
        
        std::string value = remain.substr(posColon + 1, endLine - (posColon + 1));
        if (!value.empty() && value[0] == ' ') value.erase(0, 1);

        hashMap[key] = value;
        
        if (remain.find('\n') == std::string::npos) break;
        remain = remain.substr(remain.find('\n') + 1);
    }
    return hashMap;
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}