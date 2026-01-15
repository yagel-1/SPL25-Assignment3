package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompProtocol implements StompMessagingProtocol<Frame>{
    private int connectionId;
    private Connections<Frame> connection;
    private boolean shouldTerminate = false;
    private int counterMsgId = 1;

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
                handleError(message, "message command not found");
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
            handleError(msg, "CONNECTED version not supported or host invalid");
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
            handleError(msg, "SUBSCRIBE missing destination or id");
            return;
        }
        connection.subscribe(destination, connectionId, Integer.parseInt(id));

    }

    private void handleUnsubscribe(Frame msg){
        String id = msg.getHeaders().get("id");
        if (id == null){
            handleError(msg, "SUBSCRIBE missing destination or id");
            return;
        }
        connection.unSubscribe(connectionId, Integer.parseInt(id));
    }

    private void handleSend(Frame msg){
        String destination = msg.getHeaders().get("destination");
        if (destination == null){
            handleError(msg, "SEND missing destination");
            return;
        }
        Frame frameToSend = createMsg(msg);
        connection.send(destination, frameToSend);
    }

    private void handleDisconnect(Frame msg){
        Frame receiptFrame = new Frame("RECEIPT\nreceipt-id:" + msg.getHeaders().get("receipt") + "\n\n\u0000");
        connection.send(connectionId, receiptFrame);
        connection.disconnect(connectionId);
        shouldTerminate = true;
    }

    private void handleError(Frame msg, String errorMsg){
        Frame errorFrame = new Frame("ERROR\nmessage:" + errorMsg + "\n\n" + msg.toString() + "\u0000");
        connection.send(connectionId, errorFrame);
        shouldTerminate = true;
    }

    private Frame createMsg(Frame msg){
        String Id = "" + connectionId;
        String msgId = "" + counterMsgId;
        counterMsgId++;
        String destination = msg.getHeaders().get("destination");
        String body = msg.getFrameBody();
        Frame frameToSend = new Frame("MESSAGE\nsubscription:" + Id + "\nmessage-id:" + msgId + "\ndestination:" + destination + "\n\n" + body + "\u0000");
        return frameToSend;
    }
}

