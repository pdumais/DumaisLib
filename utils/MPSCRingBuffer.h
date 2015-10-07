/* 
Copyright (c) 2015 Patrick Dumais

http://www.dumaisnet.ca

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

typedef unsigned int uint32_t;

namespace Dumais
{
    namespace Utils
    {

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
            MPSCRingBuffer(uint32_t size = 128)
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
