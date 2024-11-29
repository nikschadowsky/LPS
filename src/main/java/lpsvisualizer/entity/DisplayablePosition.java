package lpsvisualizer.entity;


import java.util.Objects;

/**
 * @since 26.11.2024
 */
public class DisplayablePosition {

    /**
     * 2 Bytes ID (uint16) + 2 * 4 Byte position data (float)
     */
    public static int SIZE = 10;

    private int id;

    private float x;
    private float y;

    public DisplayablePosition(int id, float x, float y) {
        this.id = id;
        this.x = x;
        this.y = y;
    }

    public static DisplayablePosition fromBinaryData(byte[] data) {
        if (data.length != SIZE) {
            throw new IllegalArgumentException("Passed array is not a valid binary container. Sizes do not match!");
        }

        int id = 0;
        id |= data[0];
        id <<= 8;
        id |= data[1];

        float x = Float.intBitsToFloat(
                (data[2] & 0xFF) << 24
                        | (data[3] & 0xFF) << 16
                        | (data[4] & 0xFF) << 8
                        | (data[5] & 0xFF)
        );
        float y = Float.intBitsToFloat(
                (data[6] & 0xFF) << 24
                        | (data[7] & 0xFF) << 16
                        | (data[8] & 0xFF) << 8
                        | (data[9] & 0xFF)
        );

        return new DisplayablePosition(id, x, y);
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public float getX() {
        return x;
    }

    public void setX(float x) {
        this.x = x;
    }

    public float getY() {
        return y;
    }

    public void setY(float y) {
        this.y = y;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof DisplayablePosition that)) return false;
        return id == that.id && Float.compare(x, that.x) == 0 && Float.compare(y, that.y) == 0;
    }

    @Override
    public int hashCode() {
        return Objects.hash(id, x, y);
    }

    @Override
    public String toString() {
        return "DisplayablePosition{id=%d, x=%s, y=%s}".formatted(id, x, y);
    }
}
