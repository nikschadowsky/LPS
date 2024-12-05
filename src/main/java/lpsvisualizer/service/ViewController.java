package lpsvisualizer.service;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class ViewController {

    public ViewController() {

    }

    @GetMapping("/")
    String index() {
        return "<h1>Goodbye world :(</h1>";
    }

}
