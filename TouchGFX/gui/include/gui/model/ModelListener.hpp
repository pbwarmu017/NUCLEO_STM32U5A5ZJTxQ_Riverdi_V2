#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include <cstdint>

class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    /* Called by Model::tick() when a fresh sample is pulled from the
     * producer queue.  Screens that don't care can leave the no-op
     * default. */
    virtual void sensorUpdated(uint32_t /*mv*/) {}

protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
