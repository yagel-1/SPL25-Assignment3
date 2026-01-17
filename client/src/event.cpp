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
    team_a_name = frame_body.substr(posTeamA + 2, frame_body.find('\n') - posTeamA + 2);

    size_t posTeamB = frame_body.find("team b:");
    team_b_name = frame_body.substr(posTeamB + 2, frame_body.find('\n', posTeamB) - posTeamB + 2);

    size_t posEventName = frame_body.find("event name:");
    name = frame_body.substr(posEventName + 2, frame_body.find('\n', posEventName) - posEventName + 2);

    size_t posTime = frame_body.find("time");
    time = std::stoi(frame_body.substr(posTime + 2, frame_body.find('\n', posTime) - posTime + 2));

    size_t posGeneralUp = frame_body.find("general game update:");
    size_t posTeamAUp = frame_body.find("team a updates:");
    std::string generalUp = frame_body.substr(posGeneralUp + 1, posTeamAUp - posGeneralUp + 1);
    game_updates = parseStringToMap(generalUp);

    size_t posTeamBUp = frame_body.find("team b updates:");
    std::string teamAUp = frame_body.substr(posTeamAUp + 1, posTeamBUp - posTeamAUp + 1);
    team_a_updates = parseStringToMap(teamAUp);

    size_t posDesc = frame_body.find("description:");
    std::string teamBUp = frame_body.substr(posTeamBUp + 1, posDesc - posTeamBUp + 1);
    team_b_updates = parseStringToMap(teamBUp);

    description = frame_body.substr(posDesc + 1, frame_body.find('\0') - posDesc + 1);
    
    
}

std::map<std::string, std::string> parseStringToMap(std::string map){
    std::map<std::string, std::string> hashMap;
    std::string remain = map;
    while (remain.length() != 0){
        size_t posKey = remain.find(':');
        std::string key = remain.substr(4, posKey - 4);
        std::string value = remain.substr(posKey + 2, remain.find('\n') - posKey + 2);
        hashMap[key] = value;
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