package lpsvisualizer.service;

import com.fazecast.jSerialComm.SerialPort;
import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.awaitility.Durations;
import org.junit.jupiter.api.*;
import org.mockito.*;

import java.io.IOException;
import java.io.InputStream;
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

    private static MockedStatic<SerialPort> mockStatic;

    // unit under test
    private SerialCommunicator serialCommunicator;

    @Captor
    ArgumentCaptor<List<DisplayablePosition>> positionCaptor;

    private AutoCloseable closeable;

    @BeforeAll
    static void setup() {
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

        serialCommunicator = new SerialCommunicator(mockedWSHandler);
    }

    @AfterEach
    void tearDownEach() throws Exception {
        closeable.close();
    }

    @AfterAll
    static void tearDown() {
        mockStatic.close();
    }


    @Test
    void startWithEmptyStream() {
        in.setData(List.of());
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());
    }

    @Test
    void startWithoutPrefixOrSuffix() {
        in.setData(List.of(0x1, 0x2, 0x5, 0x6, 0x8, 0x9, 0x0, 0xFF, 0xFF));
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());
    }

    @Test
    void startWithEmptyBlock() {
        in.setData(List.of(0x1, 0x2, 0x5, 0x4C, 0x50, 0x53, 0xFF, 0xFF, 0xFF));
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        assertThat(positionCaptor.getValue()).isEmpty();
    }

    @Test
    void startWithBlockContainingOnePosition() {
        in.setData(List.of(
                0x1, 0x2, 0x5, 0x4C,
                0x50, 0x53, 0, 0x4, // 2nd part of prefix + id
                0x40, 0x80, 0x0, 0x0, // x coordinate
                0x40, 0xF0, 0x0, 0x0, // y coordinate
                0xFF, 0xFF, 0xFF, 0xFE // end marker + additional data
        ));
        serialCommunicator.start();
        await().atMost(Durations.ONE_MINUTE).until(() -> in.available() == 0);
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        serialCommunicator.stop();
        assertThat(positionCaptor.getValue()).hasSize(1).contains(new DisplayablePosition(4, 4.0f, 7.5f));
    }

    @Test
    void startWithBlockContainingTwoPositions() {
        in.setData(List.of(
                0x1, 0x2, 0x5, // random data
                0x4C, 0x50, 0x53, // prefix
                0, 0x4, // 1st id
                0x40, 0x80, 0x0, 0x0, // 1st x coordinate (4.0f)
                0x40, 0xF0, 0x0, 0x0, // 1st y coordinate (7.5f)
                0x0, 0x5, // 2nd id
                0x7F, 0x80, 0x0, 0x0, // 2nd x coordinate (+inf)
                0x7F, 0x80, 0x0, 0x0, // 2nd y coordinate (+inf)
                0xFF, 0xFF, 0xFF, 0xFE // end marker + additional data
        ));
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
    void startWithTwoBlocks() {
        in.setData(List.of(0x4C, 0x50, 0x53, //1st prefix
                           0, 0x4, // 1st id
                           0x40, 0x80, 0x0, 0x0, // 1st x coordinate (4.0f)
                           0x40, 0xF0, 0x0, 0x0, // 1st y coordinate (7.5f)
                           0xFF, 0xFF, 0xFF, // 1st suffix
                           0x4C, 0x50, 0x53, // 2nd prefix
                           0x0, 0x5, // 2nd id
                           0x7F, 0x80, 0x0, 0x0, // 2nd x coordinate (+inf)
                           0x7F, 0x80, 0x0, 0x0, // 2nd y coordinate (+inf)
                           0xFF, 0xFF, 0xFF // 2nd suffix
        ));
        serialCommunicator.start();
        await().atMost(Durations.ONE_MINUTE).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, times(2)).sendPositionsToClients(positionCaptor.capture());
        assertThat(positionCaptor.getAllValues().get(0)).hasSize(1).contains(new DisplayablePosition(4, 4.0f, 7.5f));
        assertThat(positionCaptor.getAllValues().get(1)).hasSize(1)
                                                        .contains(new DisplayablePosition(5,
                                                                                          Float.POSITIVE_INFINITY,
                                                                                          Float.POSITIVE_INFINITY
                                                        ));

    }

    @Test
    void startWithBlockContainingIncompleteData() {
        in.setData(List.of(1, 2, 5, 0x4C, 0x50, 0x53, 0, 0x4, 0x40, 0x80, 0x0, 0x0, 0xFF, 0xFF, 0xFF));
        serialCommunicator.start();
        await().atMost(Durations.ONE_SECOND).until(() -> in.available() == 0);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());
    }

    @Test
    void startWithBlockContainingSuffixInData() {
        in.setData(List.of(
                0x1, 0x2, 0x5, // random data
                0x4C, 0x50, 0x53, // prefix
                0, 0x4, // 1st id
                0xFF, 0xFF, 0xFF, 0xFF, // 1st x coordinate (4.0f)
                0xFF, 0xFF, 0xFF, 0xFF, // 1st y coordinate (7.5f)
                0xFF, 0xFF, 0xFF, 0xFE // end marker + additional data
        ));
        serialCommunicator.start();
        await().atMost(Durations.ONE_MINUTE).until(() -> in.available() == 0);
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        serialCommunicator.stop();
        assertThat(positionCaptor.getValue()).hasSize(1)
                                             .contains(
                                                     new DisplayablePosition(
                                                             4,
                                                             Float.intBitsToFloat(-1),
                                                             Float.intBitsToFloat(-1)
                                                     )
                                             );
    }

    private static class MockInputStream extends InputStream {

        private Iterator<Integer> iterator;

        public void setData(List<Integer> integers) {
            iterator = integers.iterator();
        }

        @Override
        public int read() throws IOException {
            return iterator.next();
        }

        @Override
        public int available() throws IOException {
            return iterator.hasNext() ? 1 : 0;
        }
    }
}