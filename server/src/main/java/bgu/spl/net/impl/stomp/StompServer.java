package bgu.spl.net.impl.stomp;

import bgu.spl.net.impl.data.Database;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        Database.getInstance().resetAllLogins(); // Initialize the singleton Database instance
        
        if (args.length < 2) {
            System.out.println("Usage: StompServer <port> <tpc/reactor>");
            return;
        }

        int port = Integer.parseInt(args[0]);
        String mode = args[1];

        if (mode.equals("tpc")) {
            Server.threadPerClient(
                    port,
                    () -> new StompProtocol(),
                    () -> new StompEncoderDecoder()
            ).serve();
        } else if (mode.equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port,
                    () -> new StompProtocol(),
                    () -> new StompEncoderDecoder()
            ).serve();
        } else {
            System.out.println("Server mode must be 'tpc' or 'reactor'");
        }
    }
}
