package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.ConnectionImpl;
import bgu.spl.net.srv.Connections;

public class StompProtocol implements StompMessagingProtocol<Frame>{
    private int connectionId;
    private Connections<Frame> connection;
    private boolean shouldTerminate = false;

    public void start(int connectionId, Connections<Frame> connections){
        this.connectionId=connectionId;
        this.connection=connections;
    }

    public void process(Frame message){
        switch(message.getCommand()){
            case "CONNECT":
                handleConnect(message);
                break;
            case "SUBSCRIBE":
                handleSubscribe(message);
                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(message);
                break;
            case "SEND":
                handleSend(message);
                break;
            case "DISCONNECT":
                handleDisconnect(message);
                break;
            default:
                handleError("message command not found");
                break;
        }
    }
	
	/**
     * @return true if the connection should be terminated
     */
    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    private void handleConnect(Frame msg){
        String version = msg.getHeaders().get("accept-version");
        String host = msg.getHeaders().get("host");
        if (!version.equals("1.2") || !host.equals("stomp.cs.bgu.ac.il")){
            handleError("CONNECTED version not supported or host invalid");
            shouldTerminate = true;
            return;
        }
        // verifyClient(msg.getHeaders().get("login"), msg.getHeaders().get("passcode"));

        Frame connectedFrame = new Frame("CONNECTED\nversion:1.2\n\n\u0000");
        connection.send(connectionId, connectedFrame);
    }

    private void handleSubscribe(Frame msg){
        String destination = msg.getHeaders().get("destination");
        String id = msg.getHeaders().get("id");
        if (destination == null || id == null){
            handleError("SUBSCRIBE missing destination or id");
            return;
        }
        connection.subscribe(destination, connectionId, Integer.parseInt(id));

    }

    private void handleUnsubscribe(Frame msg){
        String id = msg.getHeaders().get("id");
        if (id == null){
            handleError("SUBSCRIBE missing destination or id");
            return;
        }
        connection.unSubscribe(connectionId, Integer.parseInt(id));
    }

    private void handleSend(Frame msg){}

    private void handleDisconnect(Frame msg){}

    private void handleError(String errorMsg){

    }
}

