package bgu.spl.net.impl;

import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionImpl<T> implements Connections<T> {
    // conectionID -> connectionHandler
    ConcurrentHashMap<Integer, ConnectionHandler<T>> clients = new ConcurrentHashMap<>();
    // channel -> list of (connectionIDs, subscriptionIDs)
    ConcurrentHashMap<String, ConcurrentHashMap<Integer, Integer>> channelsToId = new ConcurrentHashMap<>();
    // connectionID -> list of channels
    ConcurrentHashMap<Integer, CopyOnWriteArrayList<String>> clientChannels = new ConcurrentHashMap<>();

    public boolean send(int connectionId, T msg){
        ConnectionHandler<T> connectionHandler = clients.get(connectionId);
        if(connectionHandler != null){
            connectionHandler.send(msg);
            return true;
        }
        return false;
    }

    public void send(String channel, T msg){
        ConcurrentHashMap<Integer, Integer> connectionIds = channelsToId.get(channel);
        if(connectionIds != null){
            for(Integer id : connectionIds.keySet()){
                send(id, msg);
            }
        }
    }

    public void disconnect(int connectionId){
        ConnectionHandler<T> handler = clients.remove(connectionId);
        if (handler != null) {
            try {
                handler.close(); 
            } catch (IOException e) {}
        }
        removeChannel(connectionId);
    }

    /**
     * Add a client to the clients map
     * @param connectionId
     * @param connectionHandler
     */
    public void addClient(int connectionId, ConnectionHandler<T> connectionHandler){
        clients.put(connectionId, connectionHandler);
        clientChannels.put(connectionId, new CopyOnWriteArrayList<>());
    }

    /**
     * Subscribe a client to a channel
     * @param channel
     * @param connectionId
     * @param subscriptionId
     */
    public void subscribe(String channel, int connectionId, int subscriptionId){
        ConcurrentHashMap<Integer, Integer> channelConnections = channelsToId.putIfAbsent(channel, new ConcurrentHashMap<Integer, Integer>());
        if (channelConnections != null) {
            channelConnections.put(connectionId, subscriptionId);
        } else {
            channelsToId.get(channel).put(connectionId, subscriptionId);
        }
        clientChannels.get(connectionId).add(channel);
    }
    
    /**
     * Remove all channels associated with a connectionId
     * @param connectionId
     */
    private void removeChannel(int connectionId){
        CopyOnWriteArrayList<String> clientChannelsList = clientChannels.get(connectionId);
        if(clientChannelsList != null){
            for(String channel : clientChannelsList){
                ConcurrentHashMap<Integer, Integer> channelConnections = channelsToId.get(channel);
                if(channelConnections != null){
                    channelConnections.remove(Integer.valueOf(connectionId));
                }
            }
        }
        clientChannels.remove(connectionId);
    }

    /**
     * Unsubscribe a client from a channel
     * @param connectionId
     * @param subscriptionId
     */
    public void unSubscribe(int connectionId, int subscriptionId) {
        CopyOnWriteArrayList<String> clientChannelsList = clientChannels.get(connectionId);
        if(clientChannelsList != null){
            for(String channel : clientChannelsList){
                ConcurrentHashMap<Integer, Integer> channelConnections = channelsToId.get(channel);
                if(channelConnections != null){
                    Integer subId = channelConnections.get(Integer.valueOf(connectionId));
                    if(subId != null && subId.equals(subscriptionId)){
                        channelConnections.remove(Integer.valueOf(connectionId));
                        clientChannelsList.remove(channel);
                        return;
                    }
                }
            }
        }
        
    }

    public boolean isSubscribed(int connectionId, String channel){
        return clientChannels.get(connectionId).contains(channel);
    }

    public ConcurrentHashMap<Integer, Integer> getChannelsToId(String channel){
        return channelsToId.get(channel);
    }
}