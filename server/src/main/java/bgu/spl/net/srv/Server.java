package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;

import java.io.Closeable;
import java.util.function.Supplier;

public interface Server<T> extends Closeable {

    /**
     * The main loop of the server, Starts listening and handling new clients.
     */
    void serve();

    /**
     *This function returns a new instance of a thread per client pattern server
     * @param port The port for the server socket
     * @param protocolFactory A factory that creats new MessagingProtocols
     * @param encoderDecoderFactory A factory that creats new MessageEncoderDecoder
     * @param <T> The Message Object for the protocol
     * @return A new Thread per client server
     */
    public static <T> Server<T>  threadPerClient(
            int port,
            Supplier<MessagingProtocol<T> > protocolFactory,
            Supplier<MessageEncoderDecoder<T> > encoderDecoderFactory) {

        Supplier<StompMessagingProtocol<T>> stompFactory = () -> new ProtocolAdapter<>(protocolFactory.get());

        return new BaseServer<T>(port, stompFactory, encoderDecoderFactory) {
            @Override
            protected void execute(BlockingConnectionHandler<T>  handler) {
                new Thread(handler).start();
            }
        };

    }

    public static <T> Server<T> stompThreadPerClient(
            int port,
            Supplier<StompMessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> encoderDecoderFactory) {

        return new BaseServer<T>(port, protocolFactory, encoderDecoderFactory) {
            @Override
            protected void execute(BlockingConnectionHandler<T> handler) {
                new Thread(handler).start();
            }
        };
    }

    /**
     * This function returns a new instance of a reactor pattern server
     * @param nthreads Number of threads available for protocol processing
     * @param port The port for the server socket
     * @param protocolFactory A factory that creats new MessagingProtocols
     * @param encoderDecoderFactory A factory that creats new MessageEncoderDecoder
     * @param <T> The Message Object for the protocol
     * @return A new reactor server
     */
    public static <T> Server<T> reactor(
            int nthreads,
            int port,
            Supplier<MessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> encoderDecoderFactory) {
        Supplier<StompMessagingProtocol<T>> stompFactory = () -> new ProtocolAdapter<>(protocolFactory.get());
        return new Reactor<T>(nthreads, port, stompFactory, encoderDecoderFactory);
    }

    public static <T> Server<T> stompReactor(
            int nthreads,
            int port,
            Supplier<StompMessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> encoderDecoderFactory) {
        return new Reactor<T>(nthreads, port, protocolFactory, encoderDecoderFactory);
    }

    public static class ProtocolAdapter<T> implements StompMessagingProtocol<T> {
        private final MessagingProtocol<T> protocol;
        private int connectionId;
        private Connections<T> connections;

        public ProtocolAdapter(MessagingProtocol<T> protocol) {
            this.protocol = protocol;
        }

        @Override
        public void start(int connectionId, Connections<T> connections) {
            this.connectionId = connectionId;
            this.connections = connections;
        }

        @Override
        public void process(T message) {
            T response = protocol.process(message);
            
            if (response != null) {
                connections.send(connectionId, response);
            }
        }

        @Override
        public boolean shouldTerminate() {
            return protocol.shouldTerminate();
        }
    }
}
