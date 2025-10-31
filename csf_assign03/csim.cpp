 #include <iostream>
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <cmath>

// Cache Structures
struct Block {
    uint32_t tag;
    bool valid, dirty;
    uint32_t load_ts, access_ts;
    
    Block() : tag(0), valid(false), dirty(false), load_ts(0), access_ts(0) {}
};

struct Set {
    std::vector<Block> blocks;  // fixed size: num_blocks_per_set
    std::map<uint32_t, Block*> index;  // tag -> pointer to valid block
    
    Set(uint32_t num_blocks) : blocks(num_blocks) {}
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
    std::string eviction_policy;  // FIFO or LRU
    bool write_allocate;
    bool write_through;
    
    std::vector<Set> sets;
    Stats stats;
    uint32_t timestamp;
    
    uint32_t offset_bits;
    uint32_t index_bits;
    uint32_t tag_bits;
    
    // Extract set index from address
    uint32_t get_set_index(uint32_t address) const {
        return (address >> offset_bits) & ((1 << index_bits) - 1);
    }
    
    // Extract tag from address
    uint32_t get_tag(uint32_t address) const {
        return address >> (offset_bits + index_bits);
    }
    
    // Find victim block for eviction through sequential search
    // Should be called only on misses
    Block* find_victim(Set& set) {
        Block* victim = nullptr;
        if (eviction_policy == "lru") {  // LRU
            // Find block with smallest access_ts
            uint32_t min_ts = UINT32_MAX;
            for (auto& block : set.blocks) {
                if (!block.valid) {
                    return &block;  // Invalid block is best victim
                }
                if (block.access_ts < min_ts) {
                    min_ts = block.access_ts;
                    victim = &block;
                }
            }
        } else if (eviction_policy == "fifo") {
            // Find block with smallest load_ts
            uint32_t min_ts = UINT32_MAX;
            for (auto& block : set.blocks) {
                if (!block.valid) {
                    return &block;  // Invalid block is best victim
                }
                if (block.load_ts < min_ts) {
                    min_ts = block.load_ts;
                    victim = &block;
                }
            }
        }
        return victim;
    }
    
    // Handle cache miss - load block into cache
    void handle_miss(Set& set, uint32_t tag, bool is_store) {
        if (is_store) {
            stats.store_misses++;
        } else {
            stats.load_misses++;
        }
        
        // Cycle calculation: memory read takes 100 * (block_size / 4) cycles
        stats.total_cycles += 100 * (block_size / 4);
        
        // Find victim block
        Block* victim = find_victim(set);
        
        // If victim is valid, we're evicting
        if (victim->valid) {
            // Remove old tag from index
            set.index.erase(victim->tag);
            // Write back if dirty and not write-through
            if (victim->dirty && !write_through) {
                // Write-back takes 100 * (block_size / 4) cycles
                stats.total_cycles += 100 * (block_size / 4);
            }
        }
        
        // Load new block
        victim->tag = tag;
        victim->valid = true;
        victim->load_ts = timestamp;
        victim->access_ts = timestamp;
        
        if (write_allocate && is_store) {
            // Store operation on miss with write-allocate
            if (write_through) {
                // Write to memory takes 100 cycles
                stats.total_cycles += 100;
                victim->dirty = false;
            } else {
                victim->dirty = true;  // Mark dirty for write-back
            }
        } else {
            victim->dirty = false;
        }
        
        // Add to index
        set.index[tag] = victim;
    }
    
    // Handle cache hit
    void handle_hit(Block* block, bool is_store) {
        if (is_store) {
            stats.store_hits++;
        } else {
            stats.load_hits++;
        }
        
        // Cache hit takes 1 cycle
        stats.total_cycles += 1;
        
        block->access_ts = timestamp;
        
        if (is_store) {
            if (write_through) {
                // Write to memory takes 100 cycles
                stats.total_cycles += 100;
            } else {
                block->dirty = true;  // Mark dirty for write-back
            }
        }
    }

public:
    Cache(uint32_t sets, uint32_t blocks, uint32_t bytes, 
          const std::string& policy, bool write_alloc, bool write_thru)
        : num_sets(sets), block_size(bytes),
          eviction_policy(policy), write_allocate(write_alloc), 
          write_through(write_thru), sets(sets, Set(blocks)), timestamp(0) {
        
        // Calculate bit widths
        offset_bits = log2(block_size);
        index_bits = log2(num_sets);
        tag_bits = 32 - offset_bits - index_bits;
    }
    
    // Process a memory access
    void access(uint32_t address, bool is_store) {
        if (is_store) {
            stats.total_stores++;
        } else {
            stats.total_loads++;
        }
        
        timestamp++;
        
        uint32_t set_idx = get_set_index(address);
        uint32_t tag = get_tag(address);
        
        Set& set = sets[set_idx];
        
        // Fast path: check index for hit
        auto it = set.index.find(tag);
        if (it != set.index.end()) {
            // HIT: found valid block with matching tag
            handle_hit(it->second, is_store);
        } else {
            // MISS: need to search for invalid block or evict
            if (write_allocate || !is_store) {
                // Load into cache
                handle_miss(set, tag, is_store);
            } else {
                // no-write-allocate store miss: write directly to memory
                stats.store_misses++;
                // Write to memory takes 100 cycles
                stats.total_cycles += 100;
            }
        }
    }
    
    // Get statistics
    const Stats& get_stats() const {
        return stats;
    }
    
    // Print statistics
    void print_stats() const {
        std::cout << "Total loads: " << stats.total_loads << "\n";
        std::cout << "Total stores: " << stats.total_stores << "\n";
        std::cout << "Load hits: " << stats.load_hits << "\n";
        std::cout << "Load misses: " << stats.load_misses << "\n";
        std::cout << "Store hits: " << stats.store_hits << "\n";
        std::cout << "Store misses: " << stats.store_misses << "\n";
        std::cout << "Total cycles: " << stats.total_cycles << "\n";
    }
};