package lpsvisualizer.service;

import com.fazecast.jSerialComm.SerialPort;
import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.awaitility.Durations;
import org.junit.jupiter.api.*;
import org.mockito.*;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Iterator;
import java.util.List;

import static org.assertj.core.api.Assertions.assertThat;
import static org.awaitility.Awaitility.await;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.*;

class SerialCommunicatorTest {

    private static SerialPort mockedPort;

    @Mock
    private PositionWebSocketHandler mockedWSHandler;

    private MockInputStream in;

    @Mock
    private OutputStream out;

    private static InputStream standardIn;
    private static MockInputStream mockStandardIn;


    private static MockedStatic<SerialPort> mockStatic;

    // unit under test
    private SerialCommunicator serialCommunicator;

    @Captor
    private ArgumentCaptor<List<DisplayablePosition>> positionCaptor;

    private AutoCloseable closeable;

    private ByteArrayBuilder byteArrayBuilder;

    @BeforeAll
    static void setup() {
        standardIn = System.in;
        mockStandardIn = new MockInputStream();
        System.setIn(mockStandardIn);

        mockedPort = mock(SerialPort.class);

        mockStatic = Mockito.mockStatic(SerialPort.class);
        mockStatic.when(() -> SerialPort.getCommPort(anyString())).thenReturn(mockedPort);
    }

    @BeforeEach
    void setupEach() {
        closeable = MockitoAnnotations.openMocks(this);
        in = new MockInputStream();
        when(mockedPort.openPort()).thenReturn(true);
        when(mockedPort.getInputStream()).thenReturn(in);
        when(mockedPort.getOutputStream()).thenReturn(out);

        serialCommunicator = new SerialCommunicator(mockedWSHandler);

        byteArrayBuilder = new ByteArrayBuilder();
    }

    @AfterEach
    void tearDownEach() throws Exception {
        closeable.close();
    }

    @AfterAll
    static void tearDown() {
        mockStatic.close();
        System.setIn(standardIn);
    }


    @Test
    void startPositionDataWithEmptyStream() {
        in.setData(byteArrayBuilder.build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());
    }

    @Test
    void startPositionDataWithoutPrefixOrSuffix() {
        in.setData(byteArrayBuilder.s("Anything but the sequence").build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());
    }

    @Test
    void startPositionDataWithEmptyBlock() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_MINUTE).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        assertThat(positionCaptor.getValue()).isEmpty();
    }

    @Test
    void startPositionDataWithBlockContainingOnePosition() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .i(0x0).i(0x4) // id
                                   .i(0x40).i(0x80).i(0x0).i(0x0) // x = 4.0f
                                   .i(0x40).i(0xF0).i(0x0).i(0x0) // y = 7.5f
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        serialCommunicator.stop();
        assertThat(positionCaptor.getValue()).hasSize(1).contains(new DisplayablePosition(4, 4.0f, 7.5f));
    }

    @Test
    void startPositionDataWithBlockContainingTwoPositions() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .i(0x0).i(0x4) // id1
                                   .i(0x40).i(0x80).i(0x0).i(0x0) // x1 = 4.0f
                                   .i(0x40).i(0xF0).i(0x0).i(0x0) // y1 = 7.5f
                                   .i(0x0).i(0x5) // id2
                                   .i(0x7F).i(0x80).i(0x0).i(0x0) // x2 = +inf
                                   .i(0x7F).i(0x80).i(0x0).i(0x0) // y2 = +inf
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_MINUTE).until(() -> in.available() == 0);
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        serialCommunicator.stop();
        assertThat(positionCaptor.getValue()).hasSize(2)
                                             .contains(
                                                     new DisplayablePosition(4, 4.0f, 7.5f),
                                                     new DisplayablePosition(
                                                             5,
                                                             Float.POSITIVE_INFINITY,
                                                             Float.POSITIVE_INFINITY
                                                     )
                                             );
    }

    @Test
    void startPositionDataWithTwoBlocks() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .i(0x0).i(0x4) // id1
                                   .i(0x40).i(0x80).i(0x0).i(0x0) // x1 = 4.0f
                                   .i(0x40).i(0xF0).i(0x0).i(0x0) // y1 = 7.5f
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some data in the middle")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .i(0x0).i(0x5) // id2
                                   .i(0x7F).i(0x80).i(0x0).i(0x0) // x2 = +inf
                                   .i(0x7F).i(0x80).i(0x0).i(0x0) // y2 = +inf
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_MINUTE).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, times(2)).sendPositionsToClients(positionCaptor.capture());
        assertThat(positionCaptor.getAllValues().get(0)).hasSize(1).contains(new DisplayablePosition(4, 4.0f, 7.5f));
        assertThat(positionCaptor.getAllValues().get(1)).hasSize(1)
                                                        .contains(new DisplayablePosition(
                                                                5,
                                                                Float.POSITIVE_INFINITY,
                                                                Float.POSITIVE_INFINITY
                                                        ));

    }

    @Test
    void startPositionDataWithBlockContainingIncompleteData() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .i(0x0).i(0x4) // id
                                   .i(0x40).i(0x80).i(0x0).i(0x0) // x = 4.0f
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());
    }

    @Test
    void startPositionDataWithBlockContainingSuffixInData() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_POS_DATA_START_PREFIX)
                                   .i(0x0).i(0x4) // id1
                                   .i(0xFF).i(0xFF).i(0xFF).i(0xFF) // x1
                                   .i(0xFF).i(0xFF).i(0xFF).i(0xFF) // y1
                                   .i(0x0).i(0x5) // id2
                                   .i(0x00) // start x2
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX) // overlaps with x2, y2
                                   .ba(SerialCommunicator.ESP_POS_DATA_END_SUFFIX)
                                   .s("some more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        serialCommunicator.stop();
        assertThat(positionCaptor.getValue()).hasSize(2)
                                             .contains(
                                                     new DisplayablePosition(
                                                             4,
                                                             Float.intBitsToFloat(-1),
                                                             Float.intBitsToFloat(-1)
                                                     )
                                             );
    }

    @Test
    void startConfigStart() {
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_CONFIG_START_PREFIX)
                                   .s("more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
    }

    @Test
    void startConfiguration() throws IOException {
        mockStandardIn.setData(byteArrayBuilder.b((byte) 'A').build());
        in.setData(byteArrayBuilder.s("some data")
                                   .ba(SerialCommunicator.ESP_CONFIG_REQ_PREFIX)
                                   .s("more data")
                                   .build());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(out, times(1)).write(eq((int) 'A'));
    }

    private static class MockInputStream extends InputStream {

        private Iterator<Integer> iterator;

        public void setData(List<Integer> integers) {
            iterator = integers.iterator();
        }

        @Override
        public int read() {
            return iterator.next();
        }

        @Override
        public int available() {
            return iterator.hasNext() ? 1 : 0;
        }
    }
}