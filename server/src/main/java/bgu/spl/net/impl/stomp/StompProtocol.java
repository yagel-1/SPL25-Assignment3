package bgu.spl.net.impl.stomp;

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
                HandleError(message);
                break;
        }
    }
	
	/**
     * @return true if the connection should be terminated
     */
    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    private void handleConnect(Frame message){
        // String version = message.getHeaders().get("accept-version");
        // String host = message.getHeaders().get("host");
        // if (!version.equals("1.2") || !host.equals("stomp.cs.bgu.ac.il")){
        //     Frame errorFrame = new Frame("ERROR\nmessage:malformed CONNECT frame\n\nThe CONNECT frame is missing required headers.\u0000");
        //     connection.send(connectionId, errorFrame);
        //     shouldTerminate = true;
        //     return;
        // }
        // // verifyClient(message.getHeaders().get("login"), message.getHeaders().get("passcode"));

        // connection.addClient(connectionId, ((ConnectionImpl<Frame>)connection).clients.get(connectionId));

        // Frame connectedFrame = new Frame("CONNECTED\nversion:1.2\n\n\u0000");
        // connection.send(connectionId, connectedFrame);
    }

    private void handleSubscribe(Frame message){}

    private void handleUnsubscribe(Frame message){}

    private void handleSend(Frame message){}

    private void handleDisconnect(Frame message){}

    private void HandleError(Frame message){}

}

