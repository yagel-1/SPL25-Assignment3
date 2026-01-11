package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    public void addClient(int connectionId, ConnectionHandler<T> connectionHandler);

    public void addChannel(String channel, int connectionId);

    public void removeChannel(int connectionId);
}
