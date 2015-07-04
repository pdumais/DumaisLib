#pragma once

namespace Dumais{
namespace WebServer{
typedef unsigned int uint32_t;

// Multi-Producer, Single-Consumer, lock-free ring buffer
template <class T>
struct MPSCRingBufferItem
{
    T item;
    bool valid;
};

template <class T>
class MPSCRingBuffer
{
public:
    MPSCRingBuffer(uint32_t size)
    {
        this->head = 0;
        this->tail = 0;
        this->size = size;
        this->items = new MPSCRingBufferItem<T>[size];
        for (int i = 0; i < size; i++) this->items[i].valid = false;
    }

    ~MPSCRingBuffer()
    {
        delete[] this->items;
    }

    bool put(T item)
    {
        uint32_t oldTail = this->tail;
        uint32_t newTail = (oldTail+1)%this->size;

        // This is the critical section, this is where we reserve our slot
        while (!__sync_bool_compare_and_swap(&this->tail,oldTail,newTail))
        {
            oldTail = this->tail;
            newTail = (oldTail+1)%this->size;
            if (newTail == this->head) return false; // overflow
        }

        if (newTail == this->head) return false; // overflow
        this->items[oldTail].item = item;
        this->items[oldTail].valid = true;
        return true;
    }

    // head needs no protection. This is class is for single consumer only
    bool get(T& item)
    {
        if (this->head == this->tail) return false;
        if (!this->items[this->head].valid) return false; // tail was increased but object not inserted yet.
        item = this->items[this->head].item;
        this->items[this->head].valid = false;
        this->head = (this->head+1)%this->size;
        return true;
    }


private:
    MPSCRingBufferItem<T>* items;
    uint32_t head;
    uint32_t size;
    uint32_t tail;

};

}
}
