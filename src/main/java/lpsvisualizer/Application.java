package lpsvisualizer;

import lpsvisualizer.service.SerialCommunicator;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ConfigurableApplicationContext;

/**
 * @since 25.11.2024
 */
@SpringBootApplication
public class Application implements CommandLineRunner {

    private SerialCommunicator serialCommunicator;

    private final PositionWebSocketHandler positionWebSocketHandler;

    public Application(PositionWebSocketHandler positionWebSocketHandler) {
        this.positionWebSocketHandler = positionWebSocketHandler;
    }

    public static void main(String[] args) throws Exception {
        SpringApplication.run(Application.class, args);
    }

    @Override
    public void run(String... args) {
        serialCommunicator = new SerialCommunicator(positionWebSocketHandler);
        serialCommunicator.start();
    }
}
