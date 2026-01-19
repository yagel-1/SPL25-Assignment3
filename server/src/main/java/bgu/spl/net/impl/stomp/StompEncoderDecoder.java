package bgu.spl.net.impl.stomp;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompEncoderDecoder implements MessageEncoderDecoder<Frame>{

    private byte[] bytes = new byte[1 << 10];;
    private int len = 0;
    
    public Frame decodeNextByte(byte nextByte){
        if (nextByte == '\u0000'){
            return popString();
        }
        pushByte(nextByte);
        return null;
    }

    public byte[] encode(Frame message){
        return message.toString().getBytes();
    }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len++] = nextByte;
    }

    private Frame popString() {
        String msg = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return new Frame(msg);
    }
}
