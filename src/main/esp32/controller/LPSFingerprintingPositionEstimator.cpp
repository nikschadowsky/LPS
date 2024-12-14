#include "LPSPositionEstimator.h"
#include <cmath>
#include <HardwareSerial.h>

const uint8_t OFFLINE_DATA_POINT_AB_DIMENSION = 5;
const uint8_t OFFLINE_DATA_POINT_AD_DIMENSION = 4;

const uint8_t offline_data2[80] = {
    0xB8, 0xA9, 0xB0, 0xB2, // 1 (1,1)
    0xAC, 0xBF, 0xAB, 0xB5, // 2 (1,2)
    0xAF, 0xB8, 0xB9, 0xB1, // 3 (1,3)
    0xAE, 0xBE, 0xB2, 0xB2, // 4 (1,4)
    0xB2, 0xAE, 0xAD, 0xB6, // 5 (2,1)
    0xBD, 0xAF, 0xB0, 0xB9, // 6 (2,2)
    0xBB, 0xB9, 0xB0, 0xAC, // 7 (2,3)
    0xAF, 0xB8, 0xB4, 0xAA, // 8 (2,4)
    0xB2, 0xAF, 0xAF, 0xB5, // 9 (3,1)
    0xB4, 0xBB, 0xB2, 0xB4, // 10 (3,2)
    0xB8, 0xBD, 0xB7, 0xB0, // 11 (3,3)
    0xB5, 0xBB, 0xB6, 0xAF, // 12 (3,4)
    0xAE, 0xB3, 0xAC, 0xB9, // 13 (4,1)
    0xB8, 0xB0, 0xB4, 0xB3, // 14 (4,2)
    0xB0, 0xBE, 0xB2, 0xB1, // 15 (4,3)
    0xB0, 0xB4, 0xB6, 0xB1, // 16 (4,4)
    0xA9, 0xAC, 0xB5, 0xB5, // 17 (5,1)
    0xA6, 0xAF, 0xB8, 0xBA, // 18 (5,2)
    0xB1, 0xB3, 0xBB, 0xB4, // 19 (5,3)
    0xB1, 0xB4, 0xBB, 0xB3, // 20 (5,4)
};

const uint8_t offline_data[80] = 
{
    0xB9,0xB1,0xB6,0xBD,0xB0,0xAD,0xAE,0xB1,0xB1,0xBC,0xB8,0xB0,0xB3,0xBA,0xAD,0xAF,0xB9,0xAB,0xB0,0xBB,0xB5,0xAE,0xAC,0xB0,0xB4,0xBC,0xBC,0xB4,0xB1,0xBD,0xB8,0xB8,0xBB,0xAB,0xB3,0xAF,0xA9,0xB2,0xB7,0xAD,0xAA,0xB8,0xB9,0xB6,0xB2,0xB8,0xB0,0xB8,0xB2,0xAF,0xB3,0xB6,0xBB,0xAD,0xB7,0xB5,0xB0,0xBB,0xAE,0xB1,0xAB,0xB5,0xB7,0xBA,0xB0,0xAD,0xB0,0xBD,0xB5,0xB0,0xBF,0xB2,0xAC,0xB5,0xBE,0xB1,0xA9,0xAB,0xBC,0xB7
};

uint8_t
pow2(int8_t a)
{
    return a * a;
}

int8_t get_best_rssi(int8_t rssi[4], uint8_t *index)
{
    int8_t min = rssi[0];
    *index = 0;

    for (uint8_t i = 1; i < 4; i++)
    {
        if (rssi[i] > min)
        {
            min = rssi[i];
            *index = i;
        }
    }
    return min;
}

int8_t get_worst_rssi(int8_t rssi[4], uint8_t *index)
{
    int8_t max = rssi[0];
    *index = 0;

    for (uint8_t i = 1; i < 4; i++)
    {
        if (rssi[i] < max)
        {
            max = rssi[i];
            *index = i;
        }
    }
    return max;
}

const int8_t RSSI_OPTIMIZATION_THRESHOLD = 7;

const uint8_t ITERATION_INDICES_OPTIMIZED[12][8] = {
    {0, 1, 3, 4, 6, 7, 9, 10},  // AB
    {2, 5, 1, 4, 0, 3, 7, 6},   // BC
    {11, 10, 8, 7, 5, 4, 2, 1}, // CD
    {9, 6, 10, 7, 11, 8, 4, 5}, // DA
    {0, 3, 1, 4, 2, 5, 7, 8},   // AD
    {9, 10, 6, 7, 3, 4, 0, 1},  // DC
    {11, 8, 10, 7, 9, 6, 4, 3}, // CB
    {2, 1, 5, 4, 8, 7, 11, 10}, // BA
    {0, 1, 3, 4, 2, 6, 7, 5},   // AC
    {2, 1, 5, 4, 0, 8, 6, 3},   // BD
    {11, 10, 8, 7, 9, 5, 4, 6}, // CA
    {9, 10, 6, 7, 11, 3, 4, 8}, // DB
};

const uint8_t LOOKUP[4][4]{
    // A  B  C  D
    {255, 0, 8, 4},  // A
    {7, 255, 1, 9},  // B
    {10, 6, 255, 2}, // C
    {3, 11, 5, 255}  // D
};

std::vector<uint8_t> get_iteration_indices(const LPSDEVICE *device[4])
{
    int8_t rssi[4] = {device[0]->rssi, device[1]->rssi, device[2]->rssi, device[3]->rssi};

    std::vector<uint8_t> indices = {};

    uint8_t best_rssi_antenna_index = -1;
    uint8_t worst_rssi_antenna_index = -1;
    int8_t best_rssi = get_best_rssi(rssi, &best_rssi_antenna_index);
    int8_t worst_rssi = get_worst_rssi(rssi, &worst_rssi_antenna_index);

    Serial.print("Best antenna signal ");
    Serial.print(best_rssi);
    Serial.print(" on ");
    Serial.println(best_rssi_antenna_index);
    Serial.print("Worst antenna signal ");
    Serial.print(worst_rssi);
    Serial.print(" on ");
    Serial.println(worst_rssi_antenna_index);

    if (RSSI_OPTIMIZATION_THRESHOLD < best_rssi - worst_rssi) // since both values are negatives and we have a positive threshold
    {
        // optimize according to mapping
        uint8_t lookup = LOOKUP[best_rssi_antenna_index][worst_rssi_antenna_index];

        Serial.print("Optimization using lookup: ");
        Serial.println((int)lookup);

        for (uint8_t i = 0; i < 8; i++)
        {
            indices.push_back(ITERATION_INDICES_OPTIMIZED[lookup][i]);
        }
    }

    else
    {
        Serial.println("Not using optimization");
        for (uint8_t i = 0; i < 12; i++)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

LPSPosition estimate_position2(
    const uint16_t id,
    const LPSRoom *room_ptr,
    const LPSDEVICE *measurement_A_ptr,
    const LPSDEVICE *measurement_B_ptr,
    const LPSDEVICE *measurement_C_ptr,
    const LPSDEVICE *measurement_D_ptr)
{
    const LPSDEVICE *device[4] = {
        measurement_A_ptr,
        measurement_B_ptr,
        measurement_C_ptr,
        measurement_D_ptr};

    int8_t diff[80]; // DIFF A1, DIFF B1, DIFF C1, DIFF D1,

    for (uint8_t i = 0; i < 80; i++)
    {
        diff[i] = ((int8_t)offline_data[i]) - device[i % 4]->rssi; // -x => measurement is better; +x => measurement is worse
    }

    // todo now iterate over the squares and add up the differences. the closer we are to 0, the more certain we are that our sender is positioned there

    uint8_t square_performance[4][12];

    uint8_t best_square_index = -1;
    uint8_t best_square_acc = 0xFF; // accumulator starts at +inf

    int8_t rssi[4] = {device[0]->rssi, device[1]->rssi, device[2]->rssi, device[3]->rssi};

    std::vector<uint8_t> indices = {};

    uint8_t best_rssi_antenna_index = -1;
    uint8_t worst_rssi_antenna_index = -1;
    int8_t best_rssi = get_best_rssi(rssi, &best_rssi_antenna_index);
    int8_t worst_rssi = get_worst_rssi(rssi, &worst_rssi_antenna_index);

    Serial.print("Best antenna signal ");
    Serial.print(best_rssi);
    Serial.print(" on ");
    Serial.println(best_rssi_antenna_index);
    Serial.print("Worst antenna signal ");
    Serial.print(worst_rssi);
    Serial.print(" on ");
    Serial.println(worst_rssi_antenna_index);

    if (RSSI_OPTIMIZATION_THRESHOLD < best_rssi - worst_rssi) // since both values are negatives and we have a positive threshold
    {
        // optimize according to mapping
        uint8_t lookup = LOOKUP[best_rssi_antenna_index][worst_rssi_antenna_index];

        Serial.print("Optimization using lookup: ");
        Serial.println((int)lookup);

        for (uint8_t i = 0; i < 8; i++)
        {
            indices.push_back(ITERATION_INDICES_OPTIMIZED[lookup][i]);
        }
    }

    else
    {
        Serial.println("Not using optimization");
        for (uint8_t i = 0; i < 12; i++)
        {
            indices.push_back(i);
        }
    }

    // square offset 0,1, 3,4
    for (auto i : indices)
    {
        square_performance[0][i] = pow2(diff[i + 0] + diff[i + 4] + diff[i + 16] + diff[i + 20]);
        square_performance[1][i] = pow2(diff[i + 1] + diff[i + 5] + diff[i + 17] + diff[i + 21]);
        square_performance[2][i] = pow2(diff[i + 2] + diff[i + 6] + diff[i + 18] + diff[i + 22]);
        square_performance[3][i] = pow2(diff[i + 3] + diff[i + 7] + diff[i + 19] + diff[i + 23]);

        uint8_t perf = square_performance[0][i] + square_performance[1][i] + square_performance[2][i] + square_performance[3][i];

        if (perf < best_square_acc)
        {
            best_square_acc = perf;
            best_square_index = i;
        }
    }

    Serial.print("Best square index: ");
    Serial.print(best_square_index);
    Serial.print(" with performance: ");
    Serial.println(best_square_acc);

    // return LPSPosition{id, Point{(best_square_index % 4) * 1.5f, (best_square_index / 4) * 1.5f}, 1.5f};

    // Calculate the coordinates of the best square
    // Assuming a grid layout with a known scaling factor
    float SQUARE_SIZE = 1.5f; // Size of each square in meters
    int grid_width = 3;       // Number of squares per row
    int grid_height = 3;

    Point best_antenna_point = room_ptr->corner[best_rssi_antenna_index].position;

    float x = ((best_square_index % grid_width) + 1) * 2 + 1;
    float y = (best_square_index / grid_height) * SQUARE_SIZE + SQUARE_SIZE / 2;

    float dist_estimation = estimate_distance(device[best_rssi_antenna_index]);

    Serial.print("Best antenna's distance: ");
    Serial.println(dist_estimation);


    float v_b_x = x - best_antenna_point.x;
    float v_b_y = y - best_antenna_point.y;

    float dist_antenna_fingerprinting = hypotf(v_b_x, v_b_y);

    float new_x = (best_antenna_point.x + v_b_x);
    float new_y = (best_antenna_point.y + v_b_y);

    // Return the estimated position
    return LPSPosition{id, Point{x, y}, SQUARE_SIZE};
}

// quadranteneinschränkung auf basis vom besten rssi wert
// wir bruachen einen threshold bei dem die differenz von best rssi und worst rssi dafür sorgt, dass in einem quadranten/in allen quadranten gesucht wird.
