#include "../../include/buffer/ring_buffer.hpp"

ring_buffer::ring_buffer(){
    this->front = 0;
    this->back = 0;
    this->capacity = MAX_BUFFER_CAPACITY + 1;
    this->buffer.resize(MAX_BUFFER_CAPACITY);
}

/**
 * Adds an element into the buffer, and advances the
 *    front pointer to the next element
 */
void ring_buffer::push_back(std::string& message){
    if(!is_full()){
        this->buffer[this->front] = message;
        this->front = (this->front + 1) % this->capacity; // wrap around
    } else{
        // handle buffer overflow
        return;
    }
}

/**
 * Removes an element from the index pointed to by the
 *   back pointer, and advances the back pointer to the
 *   next buffer element
 */
void ring_buffer::pop_back(){
    if(!is_empty()){
        this->buffer[this->back].clear();
        this->back = (this->back + 1) % this->capacity; // wrap arounnd 
    } else{
        // handle buffer underflow
        return;
    }
}

/**
 * Obtains the element pointed to by the front pointer
 *
 */
std::string ring_buffer::get_front(){
    if(!is_empty()){
        return this->buffer[front];
    } else{
        // handle empty access error
        return "";
    }
    return ""
}

/**
 * Obtains the element pointed to by the back pointer
 */
std::string ring_buffer::get_back(){
    if(!is_empty()){
        return this->buffer[back];
    } else{
        // handle empty access error
        return "";
    }
    return "" 
}

/**
 * Checks if the buffer is empty
 */
bool ring_buffer::is_empty(){
    return this->front == this->back;
}

/**
 * Checks if the buffer is full
 */
bool ring_buffer::is_full(){
    return (back + 1) % this->capacity == front;
}