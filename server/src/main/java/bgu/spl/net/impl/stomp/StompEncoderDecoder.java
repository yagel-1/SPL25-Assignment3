package bgu.spl.net.impl.stomp;

import java.util.LinkedList;
import java.util.List;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompEncoderDecoder implements MessageEncoderDecoder<Frame>{

    private List<Byte> bytes = new LinkedList<>();
    
    public Frame decodeNextByte(byte nextByte){
        if (nextByte == '\u0000'){
            String msg = new String(bytes.toString())
            Frame ret = new Frame(msg);
            bytes.clear();
            return ret;
        }
        bytes.add(nextByte);
        return null;
    }

    public byte[] encode(Frame message){
        return (message.toString() + "\u0000").getBytes();
    }
}
