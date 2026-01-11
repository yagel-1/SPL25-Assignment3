package bgu.spl.net.impl;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionImpl<T> implements Connections<T> {
    ConcurrentHashMap<Integer, ConnectionHandler<T>> clients;
    ConcurrentHashMap<String, CopyOnWriteArrayList<Integer>> channels;
    ConcurrentHashMap<Integer, CopyOnWriteArrayList<String>> clientChannels;

    public boolean send(int connectionId, T msg){
        ConnectionHandler<T> connectionHandler = clients.get(connectionId);
        if(connectionHandler != null){
            connectionHandler.send(msg);
            return true;
        }
        return false;
    }

    public void send(String channel, T msg){
        CopyOnWriteArrayList<Integer> connectionIds = channels.get(channel);
        if(connectionIds != null){
            for(Integer id : connectionIds){
                send(id, msg);
            }
        }
    }

    public void disconnect(int connectionId){
        // add send disconnect
        // add close
        clients.remove(connectionId);
    }

    /**
     * Add a new client to the connections
     * @param connectionId
     * @param channel
     * @param connectionHandler
     */
    public void addClient(int connectionId, ConnectionHandler<T> connectionHandler){
        clients.put(connectionId, connectionHandler);
        clientChannels.put(connectionId, new CopyOnWriteArrayList<>());
    }

    /**
     * Add a channel to the channels map
     * @param channel
     * @param connectionId
     */
    public void addChannel(String channel, int connectionId){
        CopyOnWriteArrayList<Integer> channelConnections = channels.putIfAbsent(channel, new CopyOnWriteArrayList<Integer>());
        if (channelConnections != null) {
            channelConnections.add(connectionId);
        } else {
            channels.get(channel).add(connectionId);
        }

        clientChannels.get(connectionId).add(channel);
    }
    

    public void removeChannel(int connectionId){
        CopyOnWriteArrayList<String> clientChannelsList = clientChannels.get(connectionId);
        if(clientChannelsList != null){
            for(String channel : clientChannelsList){
                CopyOnWriteArrayList<Integer> channelConnections = channels.get(channel);
                if(channelConnections != null){
                    channelConnections.remove(Integer.valueOf(connectionId));
                }
            }
        }
        clientChannels.remove(connectionId);
    }
}