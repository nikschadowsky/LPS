package lpsvisualizer.websocket;

import lpsvisualizer.entity.DisplayablePosition;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.MockitoAnnotations;
import org.springframework.web.socket.WebSocketMessage;
import org.springframework.web.socket.WebSocketSession;

import java.io.IOException;
import java.util.List;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.*;

/**
 * @since 26.11.2024
 */
class PositionWebSocketHandlerTest {

    private WebSocketSession session1;
    private WebSocketSession session2;

    @Captor
    private ArgumentCaptor<WebSocketMessage<String>> captor;

    private AutoCloseable closeable;

    // unit under test
    private PositionWebSocketHandler handler;

    @BeforeEach
    void setup() throws Exception {
        closeable = MockitoAnnotations.openMocks(this);

        session1 = mock(WebSocketSession.class);
        when(session1.isOpen()).thenReturn(true);
        session2 = mock(WebSocketSession.class);
        when(session2.isOpen()).thenReturn(false);

        handler = new PositionWebSocketHandler();

        handler.afterConnectionEstablished(session1);
        handler.afterConnectionEstablished(session2);
    }

    @AfterEach
    void tearDown() throws Exception {
        closeable.close();
    }

    @Test
    void sendPositionsToClients() throws IOException {
        handler.sendPositionsToClients(List.of());
        handler.sendPositionsToClients(List.of(new DisplayablePosition(1, 30f, 40f)));
        handler.sendPositionsToClients(List.of(
                new DisplayablePosition(10, 30f, 40f),
                new DisplayablePosition(1, 3f, 20f)
        ));

        verify(session1, times(3)).sendMessage(captor.capture());
        assertThat(captor.getAllValues().get(0).getPayload()).isEqualTo("[]");
        assertThat(captor.getAllValues().get(1).getPayload()).isEqualTo("[{\"id\":1,\"x\":30.0,\"y\":40.0}]");
        assertThat(captor.getAllValues().get(2).getPayload()).isEqualTo(
                "[{\"id\":10,\"x\":30.0,\"y\":40.0},{\"id\":1,\"x\":3.0,\"y\":20.0}]");

        verify(session2, never()).sendMessage(any(WebSocketMessage.class));
    }


}