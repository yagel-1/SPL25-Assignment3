package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.ConnectionImpl;
import bgu.spl.net.srv.Connections;

public class StompProtocol<String> implements StompMessagingProtocol<String>{

    public void start(int connectionId, Connections<String> connections){
        //connections.addClient(connectionId, new ConnectionImpl<String>());

    }

    public void process(String message){

    }
	
	/**
     * @return true if the connection should be terminated
     */
    public boolean shouldTerminate(){
        return false;
    }
}

