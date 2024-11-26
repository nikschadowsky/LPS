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

    // unit under test
    private PositionWebSocketHandler handler;

    @Captor
    private ArgumentCaptor<WebSocketMessage<String>> captor;

    private AutoCloseable closeable;

    @BeforeEach
    void setup() throws Exception {
        closeable = MockitoAnnotations.openMocks(this);

        session1 = mock(WebSocketSession.class);
        when(session1.isOpen()).thenReturn(true);
        session2 = mock(WebSocketSession.class);
        when(session2.isOpen()).thenReturn(true);

        handler = new PositionWebSocketHandler();

        handler.afterConnectionEstablished(session1);
        handler.afterConnectionEstablished(session2);
    }

    @AfterEach
    void tearDown() throws Exception {
        closeable.close();
    }

    @Test
    void sendMessageToClients() throws IOException {
        handler.sendMessageToClients(List.of());
        handler.sendMessageToClients(List.of(new DisplayablePosition(1, 30f, 40f)));
        handler.sendMessageToClients(List.of(
                new DisplayablePosition(10, 30f, 40f),
                new DisplayablePosition(1, 3f, 20f)
        ));

        verify(session1, times(3)).sendMessage(captor.capture());
        assertThat(captor.getAllValues().get(0).getPayload()).isEqualTo("[]");
        assertThat(captor.getAllValues().get(1).getPayload()).isEqualTo("[]");
        assertThat(captor.getAllValues().get(2).getPayload()).isEqualTo("[]");
    }


}