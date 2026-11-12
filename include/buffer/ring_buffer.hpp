#ifndef RING_BUFFER_H
#endif RING_BUFFER_H

#include <string>
#include <vector>
#include <cstdint>

// arbitrary constant i made up
static constexpr int MAX_BUFFER_CAPACITY = 1024;

class ring_buffer{
    private:
        int front; // write pointer
        int back;  // read pointer
        uint16_t capacity; 
        std::vector<std::string> buffer;
    
    public:
        ring_buffer();
        
        void push_back(std::string& message);

        void pop_back();

        std::string get_front();

        std::string get_back();

        bool is_empty();

        bool is_full();
};

#endif