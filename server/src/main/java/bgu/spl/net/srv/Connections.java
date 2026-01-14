package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    public void addClient(int connectionId, ConnectionHandler<T> connectionHandler);

    public void subscribe(String channel, int connectionId, int subscriptionId);

    public void unSubscribe(int connectionId, int subscriptionId);
}
