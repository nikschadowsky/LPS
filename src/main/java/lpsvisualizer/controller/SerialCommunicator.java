package lpsvisualizer.controller;


import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * @since 25.11.2024
 */
@RestController
public class SerialCommunicator {

    @GetMapping("/")
    public String index() {
        return "Hello World";
    }

}
