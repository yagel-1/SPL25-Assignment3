package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class Frame {
    String command;
    Map<String, String> headers = new HashMap<>();
    String frameBody;

    public Frame(String msg){
        parse(msg);
    }

    public void parse(String msg){
        String[] lines = msg.split("\n");
        command = lines[0];
        int i = 1;
        while (!lines[i].equals("")){
            String[] header = lines[i].split(":", 2);
            headers.put(header[0], header[1]);
            i++;
        }
        i++;

        StringBuilder body = new StringBuilder();
        while (i<lines.length){
            body.append(lines[i]);
            i++;
        }
        frameBody = (body.toString() == "") ? null : body.toString();
    }
    
    @Override
    public String toString() {
        StringBuilder ret = new StringBuilder();
        ret.append(command).append("\n");
        for (Map.Entry<String, String> header : headers.entrySet()) {
            ret.append(header.getKey())
            .append(":")
            .append(header.getValue())
            .append("\n");
        }
        ret.append("\n");
        if (frameBody != null){
            ret.append(frameBody);
        }
        ret.append("\u0000");
        return ret.toString();
    }

    public String getCommand() {
        return command;
    }

    public Map<String, String> getHeaders() {
        return headers;
    }

    public String getFrameBody() {
        return frameBody;
    }
}


