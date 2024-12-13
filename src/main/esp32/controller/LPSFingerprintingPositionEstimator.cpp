#include "LPSPositionEstimator.h"
#include <cmath>

const uint8_t OFFLINE_DATA_POINT_AB_DIMENSION = 5;
const uint8_t OFFLINE_DATA_POINT_AD_DIMENSION = 4;

const int8_t offline_data[80] = {
    0xB8, 0xA9, 0xB0, 0xB2, //
    0xAC, 0xBF, 0xAB, 0xB5, //
    0xAF, 0xB8, 0xB9, 0xB1, //
    0xAE, 0xBE, 0xB2, 0xB2, //
    0xB2, 0xAE, 0xAD, 0xB6, //
    0xBD, 0xAF, 0xB0, 0xB9, //
    0xBB, 0xB9, 0xB0, 0xB8, //
    0xAF, 0xB8, 0xAE, 0xA9, //
    0xB2, 0xAF, 0xAF, 0xB5, //
    0xB4, 0xBB, 0xB2, 0xB4, //
    0xB8, 0xBD, 0xB7, 0xA9, //
    0xB5, 0xBB, 0xB2, 0xA6, //
    0xAE, 0xB3, 0xAC, 0xB9, //
    0xB8, 0xB0, 0xB4, 0xB3, //
    0xB0, 0xBE, 0xB2, 0xB1, //
    0xB0, 0xB4, 0xB6, 0xAB, //
    0xA9, 0xAC, 0xB5, 0xB5, //
    0xA6, 0xAF, 0xB8, 0xBA, //
    0xB1, 0xB3, 0xBB, 0xB4, //
    0xB1, 0xAC, 0xBB, 0xA9, //
};

LPSPosition estimate_position(
    const uint16_t id,
    const LPSRoom *room_ptr,
    const LPSDEVICE *measurement_A_ptr,
    const LPSDEVICE *measurement_B_ptr,
    const LPSDEVICE *measurement_C_ptr,
    const LPSDEVICE *measurement_D_ptr)
{
    int8_t rssi[4] = {
        measurement_A_ptr->rssi,
        measurement_B_ptr->rssi,
        measurement_C_ptr->rssi,
        measurement_D_ptr->rssi};

    uint8_t diff[80];

    for (int i = 0; i < 80; i++)
    {
        diff[i] = offline_data[i] - rssi[i % 4]; // -x => measurement is better; +x => measurement is worse
    }

    // todo now iterate over the squares and add up the differences. the closer we are to 0, the more certain we are that our sender is positioned there

    uint8_t square_performance[48];

    uint8_t best_square_index = -1;
    int8_t best_square_acc = 0x7F; // accumulator starts at +inf

    // square offset 0,1, 3,4
    for (int i = 0; i < 48; i++)
    {
        square_performance[i] = diff[i] + diff[i + 1] + diff[i + 3] + diff[i + 4];

        if (abs(square_performance[i]) < best_square_acc)
        {
            best_square_acc = square_performance[i];
            best_square_index = i;
        }
    }

    return LPSPosition{id, Point{best_square_index * 1.5f, (best_square_index / 4) * 1.5f}, 1.5f};

    // Calculate the coordinates of the best square
    // Assuming a grid layout with a known scaling factor
    float SQUARE_SIZE = 1.5f; // Size of each square in meters
    int grid_width = 4;       // Number of squares per row
    int grid_height = 4;

    float x = ((best_square_index % grid_width) + 1) * SQUARE_SIZE + SQUARE_SIZE / 2;
    float y = (best_square_index / grid_height) * SQUARE_SIZE + SQUARE_SIZE / 2;

    // Return the estimated position
    return LPSPosition{id, Point{x, y}, SQUARE_SIZE};
}