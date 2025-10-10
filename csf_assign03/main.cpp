#include <iostream>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <cmath>

//helper functions

// Utility function to check if a number is a power of two
static bool isPowerOfTwo(uint64_t x) {
  return x && ((x & (x - 1)) == 0);
}

// Cache Structures
struct Block {
    bool valid = false;
    uint64_t tag = 0;
    bool dirty = false;
};

int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <sets> <blocks_per_set> <block_size> <write-allocate|no-write-allocate> <write-through|write-back> <lru|fifo>" << std::endl;
        return 1;
    }
    
    int num_sets = std::atoi(argv[1]);
    int num_blocks_per_set = std::atoi(argv[2]);
    int block_size = std::atoi(argv[3]);
    std::string write_alloc = argv[4];
    std::string write_policy = argv[5];
    std::string eviction = argv[6];
    
    //validate arguments
    if (num_sets <= 0 || num_blocks_per_set <= 0 || block_size <= 0) {
        std::cerr << "Error: sets, blocks_per_set, and block_size must be positive integers.\n";
        return 1;
    }
    if (!isPowerOfTwo(static_cast<uint64_t>(num_sets)) ||
        !isPowerOfTwo(static_cast<uint64_t>(num_blocks_per_set)) ||
        !isPowerOfTwo(static_cast<uint64_t>(block_size))) {
        std::cerr << "Error: sets, blocks_per_set, and block_size must be powers of two.\n";
        return 1;
    }
    if (block_size < 4) {
        std::cerr << "Error: block_size must be at least 4 bytes.\n";
        return 1;
    }
    if (write_policy == "write-back" && write_alloc == "no-write-allocate") {
        std::cerr << "Error: write-back cannot be combined with no-write-allocate.\n";
        return 1;
    }

    // Derived params
    int block_offset_bits = static_cast<int>(std::log2(block_size));
    int index_bits = (num_sets == 1) ? 0 : static_cast<int>(std::log2(num_sets));
    uint64_t set_mask = (index_bits == 0) ? 0 : ((1ULL << index_bits) - 1ULL);
    
    // Build cache: vector of sets, each set is vector<Block>
    std::vector< std::vector<Block> > cache(num_sets, std::vector<Block>(num_blocks_per_set));

    // Initialize counters
    uint64_t total_loads = 0;
    uint64_t total_stores = 0;
    uint64_t load_hits = 0;
    uint64_t load_misses = 0;
    uint64_t store_hits = 0;
    uint64_t store_misses = 0;
    uint64_t cycles = 0;
    
    std::string line;
    //Read and Simulate
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        char op;
        std::string addr_str;
        std::string size_str;

        uint64_t addr;

        if (iss >> op >> addr_str >> size_str) {
            // Convert hex string to integer
            addr = std::stoull(addr_str, nullptr, 16);
        } else {
            std::cerr << "Error: Invalid input format.\n";
            continue;
        }

        // compute set index and tag
        uint64_t set_index;
        // Extract set index from address
        if (index_bits > 0) {
            set_index = (addr >> block_offset_bits) & set_mask;
        } else {
            set_index = 0; // fully associative
        }
        // Extract tag from address
        uint64_t tag = addr >> (block_offset_bits + index_bits);
        
        //Access the specific set determined by the address
        auto &set = cache[static_cast<size_t>(set_index)];

        // Search for matching block in the set
        int hit_index = -1;
        for (int i = 0; i < num_blocks_per_set; i++) {
            if (set[i].valid && set[i].tag == tag) {
                hit_index = i;
                break;
            }
        }

        // Operation handling
        if (op == 'l') {
            total_loads++;
            if (hit_index != -1) {
                //implment load hit
                load_hits++;
            } else{
                //implement load miss
                load_misses++;
            }
        } else if (op == 's') {
            total_stores++; {
                if (hit_index != -1) {
                    //implement store hit
                    store_hits++;
                } else {
                    //implement store miss
                    store_misses++;
                }
            }
        } else {
            std::cerr << "Error: Invalid operation '" << op << "'. Use 'l' for load and 's' for store.\n";
            continue;
        }
    }

    // Output statistics
    std::cout << "Total loads: " << total_loads << "\n";
    std::cout << "Total stores: " << total_stores << "\n";
    std::cout << "Load hits: " << load_hits << "\n";
    std::cout << "Load misses: " << load_misses << "\n";
    std::cout << "Store hits: " << store_hits << "\n";
    std::cout << "Store misses: " << store_misses << "\n";
    std::cout << "Total cycles: " << cycles << "\n";

    return 0;
}