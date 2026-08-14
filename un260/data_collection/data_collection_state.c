#include "data_collection_state.h"

#include <stdio.h>

#define DATA_COLLECTION_STATUS_MAX 128

static data_collect_mode_t g_mode = DATA_COLLECT_MODE_NONE;
static uint16_t g_pcs;
static char g_status[DATA_COLLECTION_STATUS_MAX] = "Please select a collection mode.";

data_collect_mode_t data_collection_state_mode(void)
{
    return g_mode;
}

uint16_t data_collection_state_pcs(void)
{
    return g_pcs;
}

const char *data_collection_state_status(void)
{
    return g_status;
}

void data_collection_state_set_status(const char *status)
{
    snprintf(g_status, sizeof(g_status), "%s", status != NULL ? status : "");
}

void data_collection_state_select_mode(data_collect_mode_t mode, const char *status)
{
    if (mode != DATA_COLLECT_MODE_ALL && mode != DATA_COLLECT_MODE_FALSE) {
        return;
    }
    g_mode = mode;
    g_pcs = 0;
    data_collection_state_set_status(status);
}

void data_collection_state_reset_pcs(void)
{
    g_pcs = 0;
}

void data_collection_state_exit(const char *status)
{
    g_mode = DATA_COLLECT_MODE_NONE;
    g_pcs = 0;
    data_collection_state_set_status(status);
}
