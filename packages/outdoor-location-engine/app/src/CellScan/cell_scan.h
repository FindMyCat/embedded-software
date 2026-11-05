#ifndef CELL_SCAN_H
#define CELL_SCAN_H

#include <stddef.h>
#include <stdint.h>

/* The serving cell plus up to five surrounding cells with full cell IDs. */
#define CELL_SCAN_MAX_CELL_COUNT 6

/* First byte of the frame; a WiFi scan frame starts with 0x01. */
#define CELL_SCAN_MESSAGE_TYPE 0x02

/* Message type, cell count, and uptime in seconds (u32), like a WiFi scan. */
#define CELL_SCAN_HEADER_LENGTH_BYTES 6

/* Per cell, little endian: MCC (u16), MNC (u16), tracking area code (u16),
 * cell ID (u32), and RSRP in dBm (i16, 0 when unknown). */
#define CELL_SCAN_LENGTH_BYTES_PER_CELL 12

/* Battery percent after the cells, UINT8_MAX when unknown. */
#define CELL_SCAN_TRAILER_LENGTH_BYTES 1

#define CELL_SCAN_FRAME_LENGTH_BYTES(cell_count)                               \
    (CELL_SCAN_HEADER_LENGTH_BYTES +                                           \
     (cell_count) * CELL_SCAN_LENGTH_BYTES_PER_CELL +                          \
     CELL_SCAN_TRAILER_LENGTH_BYTES)

struct cell_scan_frame {
    uint8_t bytes[CELL_SCAN_FRAME_LENGTH_BYTES(CELL_SCAN_MAX_CELL_COUNT)];
    size_t length_bytes;
};

/* Asks the modem to measure nearby cells and packs them into the frame.
 * Needs LTE registered. Blocks until the measurement finishes.
 * Returns 0, -ENODATA when no cell was found, -ETIMEDOUT, or a modem error. */
int cell_scan_measure_nearby_cells(struct cell_scan_frame *frame,
                                   uint8_t battery_percent);

#endif /* CELL_SCAN_H */