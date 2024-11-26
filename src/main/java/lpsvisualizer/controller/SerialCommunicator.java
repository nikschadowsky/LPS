package lpsvisualizer.controller;


import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * @since 25.11.2024
 */
@Controller
public class SerialCommunicator {

    private int counter = 0;

    public int incCounter() {
        return counter++;
    }

}
