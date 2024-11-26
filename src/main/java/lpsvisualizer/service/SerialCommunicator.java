package lpsvisualizer.service;


import com.fazecast.jSerialComm.SerialPort;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Controller;

/**
 * @since 25.11.2024
 */
@Controller
public class SerialCommunicator {

    private int counter = 0;

    private SerialCommunicator() {
        // start communicator thread
        System.err.println("Started");
    }

    public int incCounter() {

        return counter++;
    }

    @Async
    public void serialRead() {
        SerialPort com3 = SerialPort.getCommPort("COM3");
        com3.setBaudRate(115200);
        com3.openPort();
    }

}
