package lpsvisualizer.service;


import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * @since 01.12.2024
 */
public class ByteArrayBuilder {

    private final List<Integer> data = new ArrayList<>();

    public ByteArrayBuilder() {
    }

    public ByteArrayBuilder b(byte b) {
        data.add(b & 0xFF);
        return this;
    }

    public ByteArrayBuilder i(int i) {
        data.add(i);
        return this;
    }

    public ByteArrayBuilder s(String s) {
        ba(s.getBytes(StandardCharsets.UTF_8));
        return this;
    }

    public ByteArrayBuilder ba(byte[] ba) {
        for (byte b : ba) {
            b(b);
        }
        return this;
    }

    public List<Integer> build() {
        return new ArrayList<>(data);
    }

}
