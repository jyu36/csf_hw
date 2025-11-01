#ifndef CSIM_H
#define CSIM_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>

// Cache Structures
struct Block {
    uint32_t tag;
    bool valid, dirty;
    uint32_t load_ts, access_ts;
    
    Block();
};

struct Set {
    std::vector<Block> blocks;  // fixed size: num_blocks_per_set
    std::map<uint32_t, Block*> index; // tag to Block pointer for quick lookup
    
    Set(uint32_t num_blocks);
};

struct Stats {
    uint64_t total_loads = 0;
    uint64_t total_stores = 0;
    uint64_t load_hits = 0;
    uint64_t load_misses = 0;
    uint64_t store_hits = 0;
    uint64_t store_misses = 0;
    uint64_t total_cycles = 0;
};

class Cache {
private:
    uint32_t num_sets;
    uint32_t block_size;
    std::string eviction_policy;    // FIFO or LRU
    bool write_allocate;
    bool write_through;
    
    std::vector<Set> sets;
    Stats stats;
    uint32_t timestamp;
    
    uint32_t offset_bits;
    uint32_t index_bits;
    uint32_t tag_bits;
    
    // Extract set index from address
    uint32_t get_set_index(uint32_t address) const;
    // Extract tag from address
    uint32_t get_tag(uint32_t address) const;

    // Find victim block for eviction through sequential search
    // Should be called only on misses
    Block* find_victim(Set& set);

    // Handle cache miss - load block into cache
    void handle_miss(Set& set, uint32_t tag, bool is_store);

    // Handle cache hit
    void handle_hit(Block* block, bool is_store);

public:
    //Initialize Cache
    Cache(uint32_t sets, uint32_t blocks, uint32_t bytes, 
        const std::string& policy, bool write_alloc, bool write_thru);
    
    // Process a memory access
    void access(uint32_t address, bool is_store);

    // Print cache statistics
    void print_stats() const;
};

#endif // CSIM_H