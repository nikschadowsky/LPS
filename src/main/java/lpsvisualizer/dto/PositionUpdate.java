package lpsvisualizer.dto;

import lpsvisualizer.entity.DisplayablePosition;
import lpsvisualizer.entity.LPSRoom;

import java.util.ArrayList;
import java.util.List;

public class PositionUpdate {

    private final LPSRoom room;

    private final List<DisplayablePosition> positions;


    public PositionUpdate(LPSRoom room, List<DisplayablePosition> positions) {
        this.room = room;
        this.positions = new ArrayList<>(positions);
    }

    public LPSRoom getRoom() {
        return room;
    }

    public List<DisplayablePosition> getPositions() {
        return positions;
    }
}
