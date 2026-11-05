#include "cell_scan.h"

#include <errno.h>
#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(cell_scan,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* A surrounding-cell search can take tens of seconds on a quiet network. */
#define CELL_MEASUREMENT_TIMEOUT K_SECONDS(60)

static K_SEM_DEFINE(cell_measurement_finished, 0, 1);

static struct lte_lc_cell measured_cells[CELL_SCAN_MAX_CELL_COUNT];
static size_t measured_cell_count;

static bool is_cell_already_measured(uint32_t cell_id) {
  for (size_t i = 0; i < measured_cell_count; i++) {
    if (measured_cells[i].id == cell_id) {
      return true;
    }
  }
  return false;
}

/* The event's cell arrays only live during the callback, so they are copied. */
static void keep_measured_cell(const struct lte_lc_cell *cell) {
  if (cell->id == LTE_LC_CELL_EUTRAN_ID_INVALID ||
      measured_cell_count >= CELL_SCAN_MAX_CELL_COUNT ||
      is_cell_already_measured(cell->id)) {
    return;
  }
  measured_cells[measured_cell_count++] = *cell;
}

static void handle_cell_measurement_event(const struct lte_lc_evt *const event) {
  if (event->type != LTE_LC_EVT_NEIGHBOR_CELL_MEAS) {
    return;
  }

  const struct lte_lc_cells_info *cells = &event->cells_info;

  keep_measured_cell(&cells->current_cell);
  for (uint8_t i = 0; i < cells->gci_cells_count; i++) {
    keep_measured_cell(&cells->gci_cells[i]);
  }

  LOG_INF("Cell measurement finished with %u cell(s)",
          (unsigned)measured_cell_count);
  k_sem_give(&cell_measurement_finished);
}

static int16_t rsrp_index_to_dbm(int16_t rsrp_index) {
  if (rsrp_index == LTE_LC_CELL_RSRP_INVALID) {
    return 0;
  }
  return (int16_t)RSRP_IDX_TO_DBM(rsrp_index);
}

static void pack_measured_cells(struct cell_scan_frame *frame,
                                uint8_t battery_percent) {
  frame->bytes[0] = CELL_SCAN_MESSAGE_TYPE;
  frame->bytes[1] = (uint8_t)measured_cell_count;
  sys_put_le32((uint32_t)(k_uptime_get() / 1000), &frame->bytes[2]);

  for (size_t i = 0; i < measured_cell_count; i++) {
    const struct lte_lc_cell *cell = &measured_cells[i];
    uint8_t *cell_bytes = &frame->bytes[CELL_SCAN_HEADER_LENGTH_BYTES +
                                        i * CELL_SCAN_LENGTH_BYTES_PER_CELL];

    sys_put_le16((uint16_t)cell->mcc, &cell_bytes[0]);
    sys_put_le16((uint16_t)cell->mnc, &cell_bytes[2]);
    sys_put_le16((uint16_t)cell->tac, &cell_bytes[4]);
    sys_put_le32(cell->id, &cell_bytes[6]);
    sys_put_le16((uint16_t)rsrp_index_to_dbm(cell->rsrp), &cell_bytes[10]);
  }

  frame->length_bytes = CELL_SCAN_FRAME_LENGTH_BYTES(measured_cell_count);
  frame->bytes[frame->length_bytes - 1] = battery_percent;
}

int cell_scan_measure_nearby_cells(struct cell_scan_frame *frame,
                                   uint8_t battery_percent) {
  static bool is_event_handler_registered;
  struct lte_lc_ncellmeas_params measurement_params = {
      .search_type = LTE_LC_NEIGHBOR_SEARCH_TYPE_GCI_DEFAULT,
      /* The serving cell is reported on top of these. */
      .gci_count = CELL_SCAN_MAX_CELL_COUNT - 1,
  };

  if (!is_event_handler_registered) {
    lte_lc_register_handler(handle_cell_measurement_event);
    is_event_handler_registered = true;
  }

  measured_cell_count = 0;
  k_sem_reset(&cell_measurement_finished);

  int err = lte_lc_neighbor_cell_measurement(&measurement_params);

  if (err) {
    LOG_ERR("Error: cell measurement request failed (%d)", err);
    return err;
  }

  if (k_sem_take(&cell_measurement_finished, CELL_MEASUREMENT_TIMEOUT) != 0) {
    LOG_ERR("Error: cell measurement timed out");
    (void)lte_lc_neighbor_cell_measurement_cancel();
    return -ETIMEDOUT;
  }

  if (measured_cell_count == 0) {
    return -ENODATA;
  }

  pack_measured_cells(frame, battery_percent);
  return 0;
}