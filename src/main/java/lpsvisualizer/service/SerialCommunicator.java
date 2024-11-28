package lpsvisualizer.service;


import com.fazecast.jSerialComm.SerialPort;
import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.util.ByteChecker;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.springframework.scheduling.annotation.Async;

import java.io.IOException;
import java.io.InputStream;
import java.nio.BufferOverflowException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutionException;
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

    private final ExecutorService portListener = Executors.newSingleThreadExecutor();

    private final AtomicBoolean signal = new AtomicBoolean(false);

    public SerialCommunicator(PositionWebSocketHandler webSocketService) {
        this.webSocketService = webSocketService;
    }

    public void start() throws ExecutionException, InterruptedException {
        SerialPort com = SerialPort.getCommPort(COM_PORT);
        com.setBaudRate(115200);
        com.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
        signal.set(true);

        if (com.openPort()) {
            portListener.execute(() -> {
                int head = 0;
                int mark = -1;
                byte[] buffer = new byte[2048];
                boolean inBlock = false;

                byte[] positionBuffer = new byte[DisplayablePosition.SIZE];
                int positionBufferHead = 0;

                List<DisplayablePosition> positions = new ArrayList<>();

                try (InputStream inputStream = com.getInputStream()) {
                    while (signal.get()) {
                        if (head > buffer.length) {
                            throw new BufferOverflowException();
                        }

                        if (inputStream.available() > 0) {
                            buffer[head] = intToByte(inputStream.read());
                            if (inBlock) {
                                if (positionBufferHead == 0 && ByteChecker.checkSequence(SUFFIX, buffer, head)) {
                                    webSocketService.sendPositionsToClients(positions);
                                    positions.clear();

                                    inBlock = false;
                                    head = 0;
                                } else {
                                    // we are not at the end of a block. try parsing...
                                    positionBuffer[positionBufferHead++] = buffer[head];

                                    if (positionBufferHead == DisplayablePosition.SIZE) {
                                        positions.add(DisplayablePosition.fromBinaryData(positionBuffer));
                                        positionBufferHead = 0;
                                    }
                                }
                            } else {
                                if (ByteChecker.checkSequence(PREFIX, buffer, head)) {
                                    inBlock = true;
                                }
                            }

                            head++;
                        }
                    }

                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
        }
    }

    public int incCounter() {

        return counter++;
    }

    public void stop() {
        signal.set(false);
    }

    @Async
    public void serialRead() {

    }

    private byte intToByte(int i) {
        return (byte) (i & 0xFF);
    }


}
