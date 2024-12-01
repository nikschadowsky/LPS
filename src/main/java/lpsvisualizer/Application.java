package lpsvisualizer;

import lpsvisualizer.service.SerialCommunicator;
import lpsvisualizer.websocket.PositionWebSocketHandler;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * @since 25.11.2024
 */
@SpringBootApplication
public class Application implements CommandLineRunner {

    private SerialCommunicator serialCommunicator;

    public static void main(String[] args) throws Exception {
        SpringApplication.run(Application.class, args);
    }

    @Override
    public void run(String... args) throws Exception {
        serialCommunicator = new SerialCommunicator(new PositionWebSocketHandler());
        serialCommunicator.start();
    }
}
