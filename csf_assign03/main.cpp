#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <cstdlib>
#include "csim.cpp"  // Include the cache implementation

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " <sets> <blocks> <bytes> <allocate> <write> <evict> <trace>\n";
    std::cerr << "  <sets>     : Number of sets in the cache (power of 2)\n";
    std::cerr << "  <blocks>   : Number of blocks per set (power of 2)\n";
    std::cerr << "  <bytes>    : Number of bytes per block (power of 2, >= 4)\n";
    std::cerr << "  <allocate> : write-allocate or no-write-allocate\n";
    std::cerr << "  <write>    : write-through or write-back\n";
    std::cerr << "  <evict>    : lru or fifo\n";
}

bool is_power_of_2(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

bool validate_parameters(uint32_t sets, uint32_t blocks, uint32_t bytes,
                         const std::string& allocate, const std::string& write,
                         const std::string& evict) {
    if (!is_power_of_2(sets)) {
        std::cerr << "Error: Number of sets must be a power of 2\n";
        return false;
    }
    if (!is_power_of_2(blocks)) {
        std::cerr << "Error: Number of blocks per set must be a power of 2\n";
        return false;
    }
    if (!is_power_of_2(bytes) || bytes < 4) {
        std::cerr << "Error: Block size must be a power of 2 and at least 4\n";
        return false;
    }
    if (allocate != "write-allocate" && allocate != "no-write-allocate") {
        std::cerr << "Error: Allocate policy must be 'write-allocate' or 'no-write-allocate'\n";
        return false;
    }
    if (write != "write-through" && write != "write-back") {
        std::cerr << "Error: Write policy must be 'write-through' or 'write-back'\n";
        return false;
    }
    if (evict != "lru" && evict != "fifo") {
        std::cerr << "Error: Eviction policy must be 'lru' or 'fifo'\n";
        return false;
    }
    // Invalid combination check
    if (allocate == "no-write-allocate" && write == "write-back") {
        std::cerr << "Error: no-write-allocate cannot be used with write-back\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <sets> <blocks_per_set> <block_size> <write-allocate|no-write-allocate> <write-through|write-back> <lru|fifo>" << std::endl;
        return 1;
    }

    // Parse command line arguments
    uint32_t num_sets = std::atoi(argv[1]);
    uint32_t num_blocks_per_set = std::atoi(argv[2]);
    uint32_t block_size = std::atoi(argv[3]);
    std::string allocate_policy = argv[4];
    std::string write_policy = argv[5];
    std::string eviction_policy = argv[6];
    
    // Validate parameters
    if (!validate_parameters(num_sets, num_blocks_per_set, block_size,
                            allocate_policy, write_policy, eviction_policy)) {
        return 1;
    }

    // Convert policy strings to boolean flags
    bool write_allocate = (allocate_policy == "write-allocate");
    bool write_through = (write_policy == "write-through");

    // Create cache
    Cache cache(num_sets, num_blocks_per_set, block_size,
                eviction_policy, write_allocate, write_through);

    // Process trace file
    std::string line;
    while (std::getline(std::cin, line)) {
        // Skip empty lines 
        if (line.empty()) continue;

        std::istringstream iss(line);
        char operation;
        std::string address_str;
        uint32_t ignore;

        // Parse line
        if (!(iss >> operation >> address_str >> ignore)) {
            std::cerr << "Warning: Malformed trace line: " << line << "\n";
            continue;
        }

        // Convert hex address string to uint32_t
        uint32_t address = std::stoul(address_str, nullptr, 16);

        // Process the access
        bool is_store = (operation == 's' || operation == 'S');
        cache.access(address, is_store);
    }

    // Print statistics
    cache.print_stats();

    return 0;
}