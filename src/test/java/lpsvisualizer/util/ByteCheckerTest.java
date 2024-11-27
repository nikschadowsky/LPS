package lpsvisualizer.util;

import org.junit.jupiter.api.Test;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

/**
 * @since 27.11.2024
 */
class ByteCheckerTest {


    @Test
    void checkSequence() {
        final byte[] buffer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
        byte[] empty = {};

        assertThat(ByteChecker.checkSequence(empty, buffer, 0)).isTrue();

        byte[] single = {4};
        assertThat(ByteChecker.checkSequence(single, buffer, 1)).isFalse();
        assertThat(ByteChecker.checkSequence(single, buffer, 3)).isTrue();
        assertThat(ByteChecker.checkSequence(single, buffer, 4)).isFalse();

        byte[] seq = {6, 7, 8, 9};
        assertThat(ByteChecker.checkSequence(seq, buffer, 0)).isFalse();
        assertThat(ByteChecker.checkSequence(seq, buffer, 8)).isTrue();
        assertThat(ByteChecker.checkSequence(seq, buffer, 9)).isFalse();

        byte[] invalid = {4, 3, 2};
        for (int i = 0; i < 16; i++) {
            assertThat(ByteChecker.checkSequence(invalid, buffer, i)).isFalse();
        }

        assertThatThrownBy(() -> ByteChecker.checkSequence(
                invalid,
                buffer,
                16
        )).isInstanceOf(IllegalArgumentException.class);
        assertThatThrownBy(() -> ByteChecker.checkSequence(
                invalid,
                buffer,
                -1
        )).isInstanceOf(IllegalArgumentException.class);
    }
}