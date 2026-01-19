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

    private void parse(String msg){
        int len = msg.length();
        int headerEnd = msg.indexOf("\n\n");

        if (headerEnd == -1) {
            headerEnd = len;
        }

        String headersPart = msg.substring(0, headerEnd);
        
        if (headerEnd + 2 < len) {
            frameBody = msg.substring(headerEnd + 2);
        }

        String[] lines = headersPart.split("\n");
        if (lines.length > 0) {
            command = lines[0];
            for (int i = 1; i < lines.length; i++) {
                String line = lines[i];
                int colonIndex = line.indexOf(':');
                if (colonIndex != -1) {
                    String key = line.substring(0, colonIndex);
                    String value = line.substring(colonIndex + 1);
                    headers.put(key, value);
                }
            }
        }
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


