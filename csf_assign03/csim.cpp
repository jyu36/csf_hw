#include "csim.h"
#include <iostream>
#include <cmath>

// Block implementation
Block::Block() : tag(0), valid(false), dirty(false), load_ts(0), access_ts(0) {}

// Set implementation
Set::Set(uint32_t num_blocks) : blocks(num_blocks) {}

// Cache implementation
Cache::Cache(uint32_t sets, uint32_t blocks, uint32_t bytes, 
      const std::string& policy, bool write_alloc, bool write_thru)
    : num_sets(sets), block_size(bytes),
      eviction_policy(policy), write_allocate(write_alloc), 
      write_through(write_thru), sets(sets, Set(blocks)), timestamp(0) {
    
    // Calculate bit widths for tag, index, and offset
    offset_bits = log2(block_size);
    index_bits = log2(num_sets);
    tag_bits = 32 - offset_bits - index_bits;
}

// Extract set index from address
uint32_t Cache::get_set_index(uint32_t address) const {
    return (address >> offset_bits) & ((1 << index_bits) - 1);
}

// Extract tag from address
uint32_t Cache::get_tag(uint32_t address) const {
    return address >> (offset_bits + index_bits);
}

// Find victim block for eviction through sequential search
// Should be called only on misses
Block* Cache::find_victim(Set& set) {
    Block* victim = nullptr;
    uint32_t min_ts = UINT32_MAX;
    for (auto& block : set.blocks) {
        // If we find an invalid (empty) block, use it immediately as the victim
        if (!block.valid) {
            return &block;
        }
        if (eviction_policy == "lru") {
            // LRU (Least Recently Used): evict the block that was accessed longest ago
            if (block.access_ts < min_ts) {
                min_ts = block.access_ts;
                victim = &block;
            }
        } else if (eviction_policy == "fifo") {
            // FIFO (First In First Out): evict the block that was loaded earliest
            if (block.load_ts < min_ts) {
                min_ts = block.load_ts;
                victim = &block;
            }
        }
    }
    return victim;
}

// Handle cache miss - load block into cache
void Cache::handle_miss(Set& set, uint32_t tag, bool is_store) {
    // Update miss statistics and charge memory read cost (100 cycles per 4-byte word)
    (is_store ? stats.store_misses : stats.load_misses)++;
    stats.total_cycles += 100 * (block_size / 4);
    
    // Find a block to evict (invalid block or victim based on policy)
    Block* victim = find_victim(set);
    
    // If evicting a valid block, remove it and writeback if dirty (write-back policy only)
    if (victim->valid) {
        set.index.erase(victim->tag);
        if (victim->dirty && !write_through)
            stats.total_cycles += 100 * (block_size / 4);
    }
    
    // Load new block and set timestamps for LRU/FIFO tracking
    victim->tag = tag;
    victim->valid = true;
    victim->load_ts = victim->access_ts = timestamp;
    victim->dirty = (write_allocate && is_store && !write_through);
    if (write_allocate && is_store && write_through)
        stats.total_cycles += 100;
    set.index[tag] = victim;
}

// Handle cache hit
void Cache::handle_hit(Block* block, bool is_store) {
    // Update hit statistics based on operation type
    (is_store ? stats.store_hits : stats.load_hits)++;
    // Cache hit takes 1 cycle to access
    stats.total_cycles += 1;
     // Update the block's access timestamp for LRU tracking
    block->access_ts = timestamp;

    // Handle write operations based on write policy
    if (is_store) {
        if (write_through) {
            // Write-through: immediately write to memory (100 cycles penalty)
            stats.total_cycles += 100;
        } else {
            // Write-back: mark block as dirty, defer memory write until eviction
            block->dirty = true;
        }
    }
}

// Process a memory access
void Cache::access(uint32_t address, bool is_store) {
    // Update overall operation statistics
    if (is_store) {
        stats.total_stores++;
    } else {
        stats.total_loads++;
    }
    timestamp++;    // Increment global timestamp for tracking access order
    // Extract set index and tag from the memory address
    uint32_t set_idx = get_set_index(address);
    uint32_t tag = get_tag(address);
    // Get the corresponding set and check if the tag exists in it
    Set& set = sets[set_idx];
    auto it = set.index.find(tag);

    if (it != set.index.end()) {
        // Cache hit
        handle_hit(it->second, is_store);
    } else {
        // Cache miss
        if (write_allocate || !is_store) {
            // when encountering write-allocate or load miss, load block into cache
            handle_miss(set, tag, is_store);
        } else { //write directly to memory without caching
            stats.store_misses++;
            stats.total_cycles += 100;
        }
    }
}

// Print cache statistics
void Cache::print_stats() const {
    std::cout << "Total loads: " << stats.total_loads << "\n";
    std::cout << "Total stores: " << stats.total_stores << "\n";
    std::cout << "Load hits: " << stats.load_hits << "\n";
    std::cout << "Load misses: " << stats.load_misses << "\n";
    std::cout << "Store hits: " << stats.store_hits << "\n";
    std::cout << "Store misses: " << stats.store_misses << "\n";
    std::cout << "Total cycles: " << stats.total_cycles << "\n";
}