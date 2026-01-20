package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.impl.data.LoginStatus;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.data.Database;
import bgu.spl.net.srv.Connections;

public class StompProtocol implements StompMessagingProtocol<Frame>{
    private int connectionId;
    private Connections<Frame> connection;
    private boolean shouldTerminate = false;
    private static AtomicInteger counterMsgId = new AtomicInteger(1);

    private Database database;

    public void start(int connectionId, Connections<Frame> connections){
        this.connectionId=connectionId;
        this.connection=connections;
        this.database=Database.getInstance();
    }

    public void process(Frame message){
        try{
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
                    handleError(message, "message command not found", null);
                    break;
            }
        }
        catch (Exception e){
            handleError(message, "Exception was thrown", e.toString());
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
        if (version == null){
            handleError(msg, "malformed frame rceived", "Did not contain a accept-version header,\nwhich is REQUIRED for message propagation.");
        }
        String host = msg.getHeaders().get("host");
        if (host == null){
            handleError(msg, "malformed frame rceived", "Did not contain a host header,\nwhich is REQUIRED for message propagation.");
        }
        if (!version.equals("1.2") || !host.equals("stomp.cs.bgu.ac.il")){
            handleError(msg, "connected version not supported or host invalid", null);
            shouldTerminate = true;
            return;
        }

        String login = msg.getHeaders().get("login");
        if (login == null){
            handleError(msg, "malformed frame rceived", "Did not contain a login header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        String passcode = msg.getHeaders().get("passcode");
        if (passcode == null){
            handleError(msg, "malformed frame rceived", "Did not contain a passcode header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        LoginStatus status = database.login(connectionId, login, passcode);

        if (status == LoginStatus.LOGGED_IN_SUCCESSFULLY ||
            status == LoginStatus.ADDED_NEW_USER) {
            
            Frame connectedFrame = new Frame("CONNECTED\nversion:1.2\n\n\u0000");
            connection.send(connectionId, connectedFrame);
            
        } else if (status == LoginStatus.WRONG_PASSWORD) {
            handleError(msg, "Wrong password", "Password does not match the user");
            shouldTerminate = true;
            
        } else if (status == LoginStatus.ALREADY_LOGGED_IN) {
            handleError(msg, "User already logged in", "User already logged in");
            shouldTerminate = true;
            
        } else if (status == LoginStatus.CLIENT_ALREADY_CONNECTED) {
            handleError(msg, "Client already connected", "The client is already logged in, log out before trying again");
            shouldTerminate = true;
        }
    }

    private void handleSubscribe(Frame msg){
        String destination = msg.getHeaders().get("destination");
        if (destination == null){
            handleError(msg, "malformed frame received", "Did not contain a destination header,\nwhich is REQUIRED for message propagation.");
        }
        if (destination.startsWith("/")) {
            destination = destination.substring(1); 
        }
        String id = msg.getHeaders().get("id");
        if (id == null){
            handleError(msg, "malformed frame received", "Did not contain a id header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        connection.subscribe(destination, connectionId, Integer.parseInt(id));
        String receipt = msg.getHeaders().get("receipt");
        if (receipt != null) {
            Frame receiptFrame = new Frame("RECEIPT\nreceipt-id:" + receipt + "\n\n\u0000");
            connection.send(connectionId, receiptFrame);
        }
    }

    private void handleUnsubscribe(Frame msg){
        String id = msg.getHeaders().get("id");
        if (id == null){
            handleError(msg, "malformed frame received", "Did not contain a id header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        connection.unSubscribe(connectionId, Integer.parseInt(id));
        String receipt = msg.getHeaders().get("receipt");
        if (receipt != null) {
            Frame receiptFrame = new Frame("RECEIPT\nreceipt-id:" + receipt + "\n\n\u0000");
            connection.send(connectionId, receiptFrame);
        }
    }

    private void handleSend(Frame msg){
        String destination = msg.getHeaders().get("destination");
        if (destination == null){
            handleError(msg, "malformed frame received", "Did not contain a destination header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        if (destination.startsWith("/")) {
            destination = destination.substring(1); 
        }
        String fileName = msg.getHeaders().get("file");
        if (fileName == null){
            handleError(msg, "malformed frame received", "Did not contain a file header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        if (!connection.isSubscribed(connectionId, destination)){
            handleError(msg, "not subscribed", "The user try to send message to channel he not subscribed.");
            return;
        }
        String body = msg.getFrameBody();
        if (body == null){
            handleError(msg, "malformed frame received", "Did not contain a frame body,\nwhich is REQUIRED for message propagation.");
            return;
        }
        String userName = body.substring(body.indexOf(' ') + 1, body.indexOf('\n'));
        database.trackFileUpload(userName, fileName, destination);

        ConcurrentHashMap<Integer, Integer> conSubId = connection.getChannelsToId(destination);
        for (int conId : conSubId.keySet()){
            Frame frameToSend = createMsg(msg, conSubId.get(conId));
            connection.send(conId, frameToSend);
        }
    }

    private void handleDisconnect(Frame msg){
        String receipt = msg.getHeaders().get("receipt");
        if (receipt == null){
            handleError(msg, "malformed frame received", "Did not contain a receipt header,\nwhich is REQUIRED for message propagation.");
            return;
        }
        Frame receiptFrame = new Frame("RECEIPT\nreceipt-id:" + receipt + "\n\n\u0000");
        connection.send(connectionId, receiptFrame);
        shouldTerminate = true;
        database.logout(connectionId);
    }

    private void handleError(Frame msg, String errorMsg, String detailedErrorMsg) {
        StringBuilder builder = new StringBuilder();
        builder.append("ERROR\n");

        if (msg != null && msg.getHeaders().containsKey("receipt")) {
            builder.append("receipt-id: ").append(msg.getHeaders().get("receipt")).append("\n");
        }

        builder.append("message: ").append(errorMsg).append("\n");

        builder.append("\n");

        builder.append("The message:\n");
        builder.append("-----\n");
        builder.append(msg.toString());
        builder.append("\n-----\n");
        if (detailedErrorMsg != null){
            builder.append(detailedErrorMsg);
        }
        builder.append("\u0000");

        Frame errorFrame = new Frame(builder.toString());
        
        database.logout(connectionId);
        connection.send(connectionId, errorFrame); 
        shouldTerminate = true;
    }

    private Frame createMsg(Frame msg, int subId){
        String Id = "" + subId;
        String msgId = "" + counterMsgId.getAndIncrement();
        String destination = msg.getHeaders().get("destination");
        String body = msg.getFrameBody();
        Frame frameToSend = new Frame("MESSAGE\nsubscription:" + Id + "\nmessage-id:" + msgId + "\ndestination:" + destination + "\n\n" + body + "\u0000");
        return frameToSend;
    }

    public void getReport(){
        database.printReport();
    }
}

