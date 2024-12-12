package lpsvisualizer.websocket;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

/**
 * @since 26.11.2024
 */
@Configuration
@EnableWebSocket
public class WebSocketConfig implements WebSocketConfigurer {

    private final PositionWebSocketHandler positionWebSocketHandler;

    @Autowired
    public WebSocketConfig(PositionWebSocketHandler positionWebSocketHandler) {
        this.positionWebSocketHandler = positionWebSocketHandler;
    }

    @Override
    public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
        registry.addHandler(positionWebSocketHandler, "/api/ws").setAllowedOrigins("*");
    }
}
