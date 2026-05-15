#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdint>

extern "C" {
#include "tx_api.h"
/* sensor_q is created in app_threadx.c and pushed by Sensor_Task. */
extern TX_QUEUE sensor_q;
}

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
    /* Drain pending samples from the producer queue.  TX_NO_WAIT so we
     * don't block the render thread if the queue is empty.  The loop
     * coalesces bursts (if the producer outpaces VSYNC the latest
     * sample wins -- typical TouchGFX MVP pattern). */
    if (!modelListener) {
        return;
    }
    ULONG mv;
    while (tx_queue_receive(&sensor_q, &mv, TX_NO_WAIT) == TX_SUCCESS) {
        modelListener->sensorUpdated(static_cast<uint32_t>(mv));
    }
}
