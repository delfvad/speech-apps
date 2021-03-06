#include "vectordp.h"

#include "stdlib.h"

double VectorSignal::valueAt(int index)
{
    return array.v[index];
}

int VectorSignal::size()
{
    return array.x;
}

void VectorSignal::setValueAt(double value, int index)
{
    array.v[index] = value;
}

Signal<double> * VectorSignal::makeSignal(int size)
{
    return new VectorSignal(makev(size));
}

void VectorSignal::freeSignal()
{
    freev(this->array);
}

int VectorDP::calculateError(double value1, double value2)
{
    return abs(value1 - value2);
}
