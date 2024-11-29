package lpsvisualizer.util;


/**
 * @since 27.11.2024
 */
public class ByteChecker {

    private ByteChecker() {
    }

    public static boolean checkSequence(byte[] sequence, byte[] buffer, int offset) {
        if (offset >= buffer.length || offset < 0) {
            throw new IllegalArgumentException("Offset must be between 0 and buffer length!");
        }

        if (sequence.length == 0) {
            return true;
        }

        // if sequence is larger than the offset to allow for enough bytes before offset
        if (offset < sequence.length - 1) {
            return false;
        }

        int diff = offset - sequence.length;

        for (int i = offset; i > diff; i--) {
            if (buffer[i] != sequence[i - diff - 1]) {
                return false;
            }
        }
        return true;
    }
}
