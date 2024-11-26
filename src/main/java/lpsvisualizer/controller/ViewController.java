package lpsvisualizer.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class ViewController {

    private final SerialCommunicator serialCommunicator;

    @Autowired
    public ViewController(SerialCommunicator serialCommunicator) {
        this.serialCommunicator = serialCommunicator;
    }

    @GetMapping("/")
    String index() {
        return "<h1>Goodbye world :( %s</h1>".formatted(serialCommunicator.incCounter());
    }

}
