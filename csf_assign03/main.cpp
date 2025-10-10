#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <sets> <blocks_per_set> <block_size> <write-allocate|no-write-allocate> <write-through|write-back> <lru|fifo>" << std::endl;
        return 1;
    }
    
    int num_sets = std::atoi(argv[1]);
    int num_blocks_per_set = std::atoi(argv[2]);
    int block_size = std::atoi(argv[3]);
    std::string write_alloc = argv[4];
    std::string write_through = argv[5];
    std::string eviction = argv[6];
    
    std::string line;
    int load_count = 0;
    int store_count = 0;
    
    while (std::getline(std::cin, line)) {
        if (line[0] == 'l') {
            load_count++;
        } else if (line[0] == 's') {
            store_count++;
        }
    }
    return 0;
}
