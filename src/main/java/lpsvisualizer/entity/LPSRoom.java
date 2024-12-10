package lpsvisualizer.entity;

import java.util.Objects;

public class LPSRoom {

    private float distanceAB;
    private float distanceAD;


    public float getDistanceAB() {
        return distanceAB;
    }

    public void setDistanceAB(float distanceAB) {
        this.distanceAB = distanceAB;
    }

    public float getDistanceAD() {
        return distanceAD;
    }

    public void setDistanceAD(float distanceAD) {
        this.distanceAD = distanceAD;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof LPSRoom lpsRoom)) return false;
        return Float.compare(distanceAB, lpsRoom.distanceAB) == 0 && Float.compare(distanceAD, lpsRoom.distanceAD) == 0;
    }

    @Override
    public int hashCode() {
        return Objects.hash(distanceAB, distanceAD);
    }
}
