package lpsvisualizer.service;

import com.fazecast.jSerialComm.SerialPort;
import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.junit.jupiter.api.*;
import org.mockito.*;

import javax.swing.text.Position;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.ExecutionException;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.*;

class SerialCommunicatorTest {

    private static SerialPort mockedPort;

    private static PositionWebSocketHandler mockedWSHandler;

    private static MockInputStream in;
    private static MockedStatic<SerialPort> mockStatic;

    private SerialCommunicator serialCommunicator;

    @Captor
    ArgumentCaptor<List<DisplayablePosition>> positionCaptor;

    private AutoCloseable closeable;

    @BeforeAll
    static void setup() {
        mockStatic = Mockito.mockStatic(SerialPort.class);
        in = new MockInputStream();

        mockedPort = mock(SerialPort.class);
        when(mockedPort.openPort()).thenReturn(true);
        when(mockedPort.getInputStream()).thenReturn(in);

        mockStatic.when(() -> SerialPort.getCommPort(anyString())).thenReturn(mockedPort);

        mockedWSHandler = mock(PositionWebSocketHandler.class);
    }

    @BeforeEach
    void setupEach() {
        closeable = MockitoAnnotations.openMocks(this);
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
    void start() throws ExecutionException, InterruptedException {
        in.setData(List.of());
        serialCommunicator = new SerialCommunicator(mockedWSHandler);
        serialCommunicator.start();
        Thread.sleep(1000);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());

        in.setData(List.of(1,2,5,6,8,9,0,255,253));
        serialCommunicator.start();
        Thread.sleep(1000);
        serialCommunicator.stop();
        verify(mockedWSHandler, never()).sendPositionsToClients(any());

        in.setData(List.of(1,2,5, 0x4C, 0x50, 0x53, 0xFF,0xFF,0xFF));
        serialCommunicator.start();
        Thread.sleep(1000);
        //serialCommunicator.stop();
        verify(mockedWSHandler, times(1)).sendPositionsToClients(positionCaptor.capture());
        assertThat(positionCaptor.getValue()).isEmpty();
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