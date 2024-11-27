package lpsvisualizer.service;


import com.fazecast.jSerialComm.SerialPort;
import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.util.ByteChecker;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.springframework.scheduling.annotation.Async;

import java.io.IOException;
import java.io.InputStream;
import java.nio.BufferOverflowException;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * @since 25.11.2024
 */
public class SerialCommunicator {

    public static final String COM_PORT = "COM3";

    private static final byte[] PREFIX = {'L', 'P', 'S'};
    private static final byte[] SUFFIX = {(byte) 0xFF, (byte) 0xFF, (byte) 0xFF};

    private final PositionWebSocketHandler webSocketService;

    private int counter = 0;

    private ExecutorService portListener = Executors.newSingleThreadExecutor();

    private AtomicBoolean signal = new AtomicBoolean(false);

    CopyOnWriteArrayList<DisplayablePosition> positions = new CopyOnWriteArrayList<>();


    private SerialCommunicator(PositionWebSocketHandler webSocketService) {
        this.webSocketService = webSocketService;
        // start communicator thread
        System.err.println("Started");
    }

    public void start() {
        SerialPort com = SerialPort.getCommPort(COM_PORT);
        com.setBaudRate(115200);
        com.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);

        if (com.openPort()) {
            portListener.submit(() -> {
                int head = 0;
                int mark = -1;
                byte[] buf = new byte[1024];
                boolean inBlock = false;


                try (InputStream inputStream = com.getInputStream()) {
                    while (signal.get()) {
                        if (head > buf.length) {
                            throw new BufferOverflowException();
                        }

                        buf[head] = intToByte(inputStream.read());
                        if (inBlock) {
                            if (ByteChecker.checkSequence(SUFFIX, buf, head)) {
                                // todo send WS message
                                webSocketService.sendPositionsToClients(positions);
                                inBlock = false;
                            }
                        } else {
                            if (ByteChecker.checkSequence(PREFIX, buf, head)) {
                                inBlock = true;
                            }
                        }

                        head++;
                    }

                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
        }
        signal.set(true);
    }

    public int incCounter() {

        return counter++;
    }

    @Async
    public void serialRead() {

    }

    private byte intToByte(int i) {
        return (byte) (i & 0xFF);
    }


}
