package lpsvisualizer.service;


import com.fazecast.jSerialComm.SerialPort;
import lpsvisualizer.dto.PositionUpdate;
import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.entity.LPSRoom;
import lpsvisualizer.util.ByteChecker;
import lpsvisualizer.websocket.PositionWebSocketHandler;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.BufferOverflowException;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * @since 25.11.2024
 */
public class SerialCommunicator {

    public static final String COM_PORT = "COM3";

    public static final byte[] ESP_CONFIG_START_PREFIX = "ESP_CONFIG_START".getBytes(StandardCharsets.UTF_8);
    public static final byte[] ESP_CONFIG_REQ_PREFIX = "ESP_CONFIG_REQ".getBytes(StandardCharsets.UTF_8);
    public static final byte[] ESP_POS_DATA_START_PREFIX = "ESP_POS_DATA_START".getBytes(StandardCharsets.UTF_8);
    public static final byte[] ESP_CONFIG_DIST1_PREFIX = "ESP_CONFIG_DIST1".getBytes(StandardCharsets.UTF_8);
    public static final byte[] ESP_CONFIG_DIST2_PREFIX = "ESP_CONFIG_DIST2".getBytes(StandardCharsets.UTF_8);
    /*
     * Has a maximum length requirement of DisplayablePosition.SIZE
     */
    public static final byte[] ESP_POS_DATA_END_SUFFIX = "POS_DATA_END".getBytes(StandardCharsets.UTF_8);

    private final Map<byte[], EspSerialRequestHandler> ESP_HANDLERS = Map.of(
            ESP_CONFIG_START_PREFIX, this::handleConfigStart,
            ESP_CONFIG_REQ_PREFIX, this::handleConfiguration,
            ESP_POS_DATA_START_PREFIX, this::handlePositionDataBlock,
            ESP_CONFIG_DIST1_PREFIX, (buf, in, out) -> handleDistance(buf, in, out, DistanceInsertionMode.DIST_AB),
            ESP_CONFIG_DIST2_PREFIX, (buf, in, out) -> handleDistance(buf, in, out, DistanceInsertionMode.DIST_AD)
    );

    private final PositionWebSocketHandler webSocketService;

    private final ExecutorService portListener = Executors.newSingleThreadExecutor();

    private final AtomicBoolean signal = new AtomicBoolean(false);

    private final Scanner scanner = new Scanner(System.in);

    private final LPSRoom room = new LPSRoom();

    public SerialCommunicator(PositionWebSocketHandler webSocketService) {
        this.webSocketService = webSocketService;
    }

    public void start() {
        System.out.println("Starting serial communicator");
        SerialPort com = SerialPort.getCommPort(COM_PORT);
        com.setBaudRate(115200);
        com.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
        signal.set(true);

        if (com.openPort()) {
            portListener.execute(() -> {
                int head = 0;
                byte[] buffer = new byte[2048];

                try (InputStream in = com.getInputStream(); OutputStream out = com.getOutputStream()) {

                    while (signal.get()) {
                        if (in.available() <= 0) {
                            continue;
                        }

                        checkBufferOverflow(buffer, head);
                        buffer[head] = intToByte(in.read());

                        for (Map.Entry<byte[], EspSerialRequestHandler> handler : ESP_HANDLERS.entrySet()) {
                            if (ByteChecker.checkSequence(handler.getKey(), buffer, head)) {
                                handler.getValue().handle(buffer, in, out);
                                head = 0;
                                break;
                            }
                        }

                        head++;
                    }

                } catch (IOException e) {
                    e.printStackTrace();
                }

            });
        }
    }

    /**
     * Terminates the
     */
    public void stop() {
        signal.set(false);
    }

    private static byte intToByte(int i) {
        return (byte) (i & 0xFF);
    }

    private void handleConfigStart(byte[] buffer, InputStream in, OutputStream out) {
        System.out.println("The LPS controller requested to configure.");
        System.out.println(
                """
                        Configuring room setup...
                        This is the schematic of the room. You are now guided to associate each corner with the antenna placed there.
                          B --- C
                          |     |
                          |     |
                          |     |
                        ↑ A --- D
                        0,0 →""");
    }

    private void handleConfiguration(byte[] buffer, InputStream in, OutputStream out) throws IOException {
        System.out.println("An antenna is lighting up. Which corner of the room is this antenna representing?");
        System.out.println("Type A - D to specify:");

        while (signal.get()) {
            if (scanner.hasNext()) {
                String next = scanner.nextLine();

                if (next.length() == 1 && next.charAt(0) >= 'A' && next.charAt(0) <= 'D') {
                    out.write(next.charAt(0));
                    return;
                } else {
                    System.out.println("Type A - D to specify:");
                }
            }
        }

    }

    private void handlePositionDataBlock(byte[] buffer, InputStream in, OutputStream out) throws IOException {
        int head = 0;

        byte[] positionBuffer = new byte[DisplayablePosition.SIZE];
        int positionBufferHead = 0;

        List<DisplayablePosition> positions = new ArrayList<>();

        while (signal.get()) {
            checkBufferOverflow(buffer, head);

            if (in.available() > 0) {
                buffer[head] = intToByte(in.read());
                if (positionBufferHead == ESP_POS_DATA_END_SUFFIX.length - 1 && ByteChecker.checkSequence(
                        ESP_POS_DATA_END_SUFFIX,
                        buffer,
                        head
                )) {
                    // pos buffer head should be one less than suffix's length
                    // because of the other bytes read before successfully processing the suffix
                    webSocketService.sendPositionsToClients(new PositionUpdate(room, new ArrayList<>(positions)));
                    positions.clear();

                    return;
                } else {
                    // we are not at the end of a block. try parsing...
                    positionBuffer[positionBufferHead++] = buffer[head];

                    if (positionBufferHead == DisplayablePosition.SIZE) {
                        positions.add(DisplayablePosition.fromBinaryData(positionBuffer));
                        positionBufferHead = 0;
                    }
                }

                head++;
            }
        }
    }

    private void handleDistance(byte[] buffer, InputStream in, OutputStream out, DistanceInsertionMode mode) throws IOException {
        System.out.printf(
                "Please enter the distance in metres from corner %s to corner %s as a float:%n",
                mode.leftCorner, mode.rightCorner
        );

        while (signal.get()) {
            if (scanner.hasNext()) {
                if (scanner.hasNextFloat()) {
                    float nextFloat = scanner.nextFloat();

                    switch(mode){
                        case DIST_AB -> room.setDistanceAB(nextFloat);
                        case DIST_AD -> room.setDistanceAD(nextFloat);
                    }

                    int next = Float.floatToRawIntBits(nextFloat);
                    out.write(next >> 24);
                    out.write(next >> 16);
                    out.write(next >> 8);
                    out.write(next);
                    return;
                } else {
                    scanner.next();
                    System.out.println("Please enter a float:");
                }
            }
        }
    }

    @FunctionalInterface
    private interface EspSerialRequestHandler {

        void handle(byte[] buffer, InputStream in, OutputStream out) throws IOException;

    }

    private static void checkBufferOverflow(byte[] buffer, int head) {
        if (head >= buffer.length) {
            throw new BufferOverflowException();
        }
    }

    private enum DistanceInsertionMode {
        DIST_AB("A","B"), DIST_AD("A","D");

        private final String leftCorner;
        private final String rightCorner;

        DistanceInsertionMode(String leftCorner, String rightCorner) {
            this.leftCorner = leftCorner;
            this.rightCorner = rightCorner;
        }
    }
}
