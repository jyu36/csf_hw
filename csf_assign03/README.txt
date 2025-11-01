CSF Assignment 3: Cache Simulator
Team Members: Jiaqi Yu and Lily Ru


Milestone Contributions

MS1: 
Jiaqi Yu: Handle basic command line and reading logics
Lily Ru: Add validations to the arguments and inputs; Set up for MS2 and MS3

MS2: 
Lily Ru: Implement cache data structures and LRU replacement logic, verify hit/miss accuracy
Jiaqi Yu: Implement cycle counting, write policies and assist with testing and debugging

MS3:
Jiaqi Yu: Run comprehensive experiments and collect performance data
Lily Ru: Analyze experimental results and write final report





Part B: Best Cache Configuration


We tested different cache configurations. 
We varied five parameters: associativity, block size, eviction policy, write policy, and total cache size. 
We used two workload traces: gcc.trace (compiler, 318,197 loads and 197,486 stores) and swim.trace (scientific computing, 220,668 loads and 82,525 stores).
For each configuration, we calculated hit rate as (total hits / total accesses) × 100%, where total hits = load hits + store hits, and total accesses = total loads + total stores.



Experiment 1: Testing Associativity

Goal: Determine if increasing associativity improves performance and where diminishing returns occur. So I tested 1 way direct mapping 4 way and 8 way set associative
Why: Higher associativity reduces conflict misses by providing more placement options, but increases hardware complexity.


Test 1a: Direct-mapped (1024 sets, 1 block, 16 bytes) with LRU and write-back on gcc.trace
./csim 1024 1 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 312238
Load misses: 5959
Store hits: 187502
Store misses: 9984
Total cycles: 11111340

Results:
Total hits = 312238 + 187502 = 499,740
Total misses = 5959 + 9984 = 15,943
Hit rate = 499740 / 515683 × 100% = 96.91%



Test 1b: 4-way set-associative (256 sets, 4 blocks, 16 bytes) with LRU and write-back on gcc.trace
./csim 256 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9331848

Results:
Total hits = 314798 + 188250 = 503,048
Total misses = 3399 + 9236 = 12,635
Hit rate = 503048 / 515683 × 100% = 97.55%
Improvement over direct-mapped: 15943 - 12635 = 3,308 fewer misses (20.7% reduction)
Cycle improvement: 11.1M - 9.3M = 1.8M fewer cycles (16.2% faster)


Test 1c: 8-way set-associative (128 sets, 8 blocks, 16 bytes) with LRU and write-back on gcc.trace
./csim 128 8 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314908
Load misses: 3289
Store hits: 188278
Store misses: 9208
Total cycles: 9265186

Results:
Total hits = 314908 + 188278 = 503,186
Total misses = 3289 + 9208 = 12,497
Hit rate = 503186 / 515683 × 100% = 97.58%
Improvement over 4-way: 12635 - 12497 = 138 fewer misses (1.1% reduction)
Cycle improvement: 9331848 - 9265186 = 66,662 fewer cycles (0.7% faster)



Test 1d: 16-way set-associative (64 sets, 16 blocks, 16 bytes) with LRU and write-back on gcc.trace
./csim 64 16 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314944
Load misses: 3253
Store hits: 188276
Store misses: 9210
Total cycles: 9254420

Results:
Total hits = 314944 + 188276 = 503,220
Total misses = 3253 + 9210 = 12,463
Hit rate = 503220 / 515683 × 100% = 97.58%
Improvement over 8-way: 12497 - 12463 = 34 fewer misses (0.27% reduction)
Cycle improvement: 9265186 - 9254420 = 10,766 fewer cycles (0.12% faster)


Test 1e: Fully associative (1 set, 1024 blocks, 16 bytes) with LRU and write-back on gcc.trace
./csim 1 1024 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314973
Load misses: 3224
Store hits: 188300
Store misses: 9186
Total cycles: 9214473

Results:
Total hits = 314973 + 188300 = 503,273
Total misses = 3224 + 9186 = 12,410
Hit rate = 503273 / 515683 × 100% = 97.59%
Improvement over 16-way: 12463 - 12410 = 53 fewer misses (0.43% reduction)
Cycle improvement: 9254420 - 9214473 = 39,947 fewer cycles (0.43% faster)


Edge Case: Minimal cache (1 set, 1 block, 4 bytes) with LRU and write-back on gcc.trace
./csim 1 1 4 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 13415
Load misses: 304782
Store hits: 13556
Store misses: 183930
Total cycles: 68284271

Results:
Total hits = 13415 + 13556 = 26,971
Total misses = 304782 + 183930 = 488,712
Hit rate = 26971 / 515683 × 100% = 5.23%
This minimal 4-byte cache can only hold one memory location and performs extremely poorly, demonstrating the importance of adequate cache capacity.



Conclusion from Experiment 1:

Increasing associativity from 1 way direct-mapped to 4-way reduced misses by 20.7% and execution time by 16.2%. This large improvement occurs because direct-mapped caches suffer from conflict misses when multiple frequently-accessed addresses map to the same cache line.
However, moving from 4-way to 8-way only reduces misses by 1.1% and going to 16-way reduced by 0.27%. 
Even going to fully associative (1024-way) only improved by an additional 0.43% over 16-way, showing extreme diminishing returns.
At these higher associativities, most conflict misses have already been eliminated, so additional ways don't help much.

The edge case of a minimal cache (1 set, 1 block, 4 bytes) shows only 5.23% hit rate with 488,712 misses, demonstrating that without adequate capacity and associativity, a cache is nearly useless.

The 4-way configuration captures most of the performance benefit while keeping hardware complexity reasonable. 
Higher associativity requires more comparators for parallel tag matching and more complex LRU tracking logic, making the marginal improvements not worth the added cost and power consumption.





Experiment 2: Testing Block Size

Goal: Find optimal block size considering miss rate vs miss penalty trade-off.
Why: Larger blocks exploit spatial locality and reduce misses but increase miss penalty because each miss transfers more data. So we should find out which one dominates the effect


Edge Case: 4-byte blocks (1024 sets, 4 blocks, 4 bytes) with LRU and write-back on gcc.trace
./csim 1024 4 4 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 312620
Load misses: 5577
Store hits: 169672
Store misses: 27814
Total cycles: 6415192

Results:
Total misses = 5577 + 27814 = 33,391
Hit rate = (312620 + 169672) / 515683 × 100% = 93.52%
Miss penalty per block = 100 × (4/4) = 100 cycles (minimal penalty)
Total cycles: 6.4M (FASTEST despite more misses because penalty is so low)


Test 2a: 16-byte blocks (256 sets, 4 blocks, 16 bytes) with LRU and write-back on gcc.trace
./csim 256 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9331848

Results:
Total misses = 12,635
Hit rate = 97.55%
Miss penalty per block = 100 × (16/4) = 400 cycles


Test 2b: 32-byte blocks (128 sets, 4 blocks, 32 bytes) with LRU and write-back on gcc.trace
./csim 128 4 32 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 315689
Load misses: 2508
Store hits: 192637
Store misses: 4849
Total cycles: 10609126

Results:
Total misses = 2508 + 4849 = 7,357
Hit rate = (315689 + 192637) / 515683 × 100% = 98.56%
Miss reduction: 12635 - 7357 = 5,278 fewer misses (41.8% reduction)
Miss penalty per block = 100 × (32/4) = 800 cycles (2× larger than 16-byte)
Total cycles: 10.6M vs 9.3M = 1.3M MORE cycles (13.7% SLOWER)


Test 2c: 64-byte blocks (64 sets, 4 blocks, 64 bytes) with LRU and write-back on gcc.trace
./csim 64 4 64 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 316157
Load misses: 2040
Store hits: 194821
Store misses: 2665
Total cycles: 13045378

Results:
Total misses = 2040 + 2665 = 4,705
Hit rate = (316157 + 194821) / 515683 × 100% = 99.09%
Miss reduction: 12635 - 4705 = 7,930 fewer misses (62.7% reduction)
Miss penalty per block = 100 × (64/4) = 1600 cycles (4× larger than 16-byte)
Total cycles: 13.0M vs 9.3M = 3.7M MORE cycles (39.8% SLOWER)



Conclusion from Experiment 2:
As block size increases, cache misses decrease from 33,391 misses (4-byte) to 12,635 misses (16-byte) to 7,357 misses (32-byte) to 4,705 misses (64-byte).

However, larger blocks have significantly higher miss penalties because each miss must transfer the entire block from memory at 100 cycles per 4-byte word:
- 4-byte: 100 cycles per miss
- 16-byte: 400 cycles per miss
- 32-byte: 800 cycles per miss 
- 64-byte: 1,600 cycles per miss 

Interestingly, the edge case of 4-byte blocks achieved the fastest execution time (6.4M cycles) despite having the most misses (33,391), because the minimal miss penalty (100 cycles) more than compensates for the poor hit rate. However, 4-byte blocks are impractical because they don't exploit any spatial locality and would require cache line size to match memory bus width.

For practical designs, 16-byte blocks provide the best balance. They achieve 97.55% hit rate and 9.3M cycles. Larger blocks like 32-byte and 64-byte reduce misses by 41.8% and 62.7% respectively, but their higher penalties make them 13.7% and 39.8% slower.

This suggests gcc.trace has moderate spatial locality where not all data in larger blocks gets used before eviction and wasting memory bandwidth.




Experiment 3: Testing Eviction Policies (LRU vs FIFO)

Goal: Compare LRU and FIFO replacement policies.
Why: LRU exploits temporal locality by keeping recently-used data, but requires tracking access order. FIFO is simpler but may evict frequently-used blocks. Need to measure if LRU's complexity is justified.


Test 3a: LRU eviction (256 sets, 4 blocks, 16 bytes) with write-back on gcc.trace
Command: ./csim 256 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9331848

Results:
Total misses = 12,635
Hit rate = 97.55%


Test 3b: FIFO eviction (256 sets, 4 blocks, 16 bytes) 
Command: ./csim 256 4 16 write-allocate write-back fifo < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314171
Load misses: 4026
Store hits: 188047
Store misses: 9439
Total cycles: 9831818

Results:
Total misses = 4026 + 9439 = 13,465
Hit rate = (314171 + 188047) / 515683 × 100% = 97.39%
LRU advantage: 13465 - 12635 = 830 more misses with FIFO (6.6% worse)
Cycle difference: 9831818 - 9331848 = 499,970 more cycles (5.4% slower)


Test 3c: LRU on swim.trace
Command: ./csim 256 4 16 write-allocate write-back lru < traces/swim.trace

Output:
Total loads: 220668
Total stores: 82525
Load hits: 219507
Load misses: 1161
Store hits: 71956
Store misses: 10569
Total cycles: 8997863

Results:
Total misses = 1161 + 10569 = 11,730
Hit rate = (219507 + 71956) / 303193 × 100% = 96.13%


Test 3d: FIFO on swim.trace
Command: ./csim 256 4 16 write-allocate write-back fifo < traces/swim.trace

Output:
Total loads: 220668
Total stores: 82525
Load hits: 218357
Load misses: 2311
Store hits: 71787
Store misses: 10738
Total cycles: 9642544

Results:
Total misses = 2311 + 10738 = 13,049
Hit rate = (218357 + 71787) / 303193 × 100% = 95.74%
LRU advantage: 13049 - 11730 = 1,319 more misses with FIFO (11.2% worse)
Cycle difference: 9642544 - 8997863 = 644,681 more cycles (7.2% slower)

Conclusion: 
LRU consistently outperforms FIFO (5.4% to 7.2% faster). 
The swim workload benefits more from LRU (11.2% fewer misses vs 6.6% for gcc), this is likely because scientific computing has stronger temporal locality patterns in array accesses. 
The performance benefit justifies LRU's modest hardware cost.





Experiment 4: Testing Write Policies

Goal: Compare write-back vs write-through performance.
Why: Write-back only updates cache (1 cycle) and defers memory writes, while write-through writes to both cache and memory (101 cycles). Need to quantify the performance difference.


Test 4a: write-allocate + write-back (256 sets, 4 blocks, 16 bytes) with LRU on gcc.trace
Command: ./csim 256 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9331848

Results:
Total misses = 12,635
Hit rate = 97.55%


Test 4b: write-allocate + write-through (256 sets, 4 blocks, 16 bytes) with LRU on gcc.trace
Command: ./csim 256 4 16 write-allocate write-through lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 25305648

Results:
Total misses = 12,635 (SAME as write-back)
Hit rate = 97.55% (SAME as write-back)
Cycle difference: 25305648 - 9331848 = 15,973,800 MORE cycles (171% SLOWER)
Why slower: Every store (197,486 total) writes to memory (+100 cycles each), adding ~19.7M extra cycles

Conclusion: Write-back is 2.7× faster than write-through because stores only update cache (1 cycle) instead of writing through to memory (100 cycles). 
The miss rates are identical because write policy doesn't affect cache lookup, only what happens on hits and misses.




Experiment 5: Testing Cache Size

Goal: Find optimal cache size considering cost vs performance trade-offs.

Why: Larger caches reduce misses but cost more silicon area and power. Need to identify the point of diminishing returns.


Test 5a: 4 KB cache (64 sets, 4 blocks, 16 bytes) with LRU and write-back on gcc.trace
Command: ./csim 64 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 312099
Load misses: 6098
Store hits: 187526
Store misses: 9960
Total cycles: 11398825

Results:
Total misses = 6098 + 9960 = 16,058
Hit rate = (312099 + 187526) / 515683 × 100% = 96.89%


Test 5b: 16 KB cache (256 sets, 4 blocks, 16 bytes) with LRU and write-back on gcc.trace
Command: ./csim 256 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9331848

Results:
Total misses = 12,635
Hit rate = 97.55%
Improvement over 4KB: 16058 - 12635 = 3,423 fewer misses (21.3% reduction)
Cycle improvement: 11.4M - 9.3M = 2.1M fewer cycles (18.1% faster)


Test 5c: 64 KB cache (1024 sets, 4 blocks, 16 bytes) with LRU and write-back on gcc.trace
Command: ./csim 1024 4 16 write-allocate write-back lru < traces/gcc.trace

Output:
Total loads: 318197
Total stores: 197486
Load hits: 315505
Load misses: 2692
Store hits: 188561
Store misses: 8925
Total cycles: 7609266

Results:
Total misses = 2692 + 8925 = 11,617
Hit rate = (315505 + 188561) / 515683 × 100% = 97.75%
Improvement over 16KB: 12635 - 11617 = 1,018 fewer misses (8.1% reduction)
Cost: 4× cache size (16KB → 64KB) for only 8.1% fewer misses

Conclusion: 
Increasing cache size from 4KB to 16KB made 21.3% fewer misses.
However, quadrupling from 16KB to 64KB only reduces misses by 8.1%, showing clear diminishing returns. 
The 16KB cache approaches the working set size of these applications, making larger caches less cost-effective.
However, the cache sizes tested above (4KB, 16KB, 64KB) refer only to data storage capacity. Real caches require additional storage for metadata. 
This overhead affects the actual silicon area and power consumption:

Data storage: 256 × 4 × 16 = 16,384 bytes = 16 KB

Tag bits per block:
Total address bits = 32
Offset bits = log2(16) = 4
Index bits = log2(256) = 8
Tag bits = 32 - 4 - 8 = 20 bits

Metadata per block:
Tag: 20 bits
Valid bit: 1 bit
Dirty bit: 1 bit
Total: 22 bits per block

Total overhead:
256 sets × 4 blocks × 22 bits = 22,528 bits = 2,816 bytes ≈ 2.75 KB
Total cache size: 16 KB (data) + 2.75 KB (overhead) = 18.75 KB





Best Cache Configuration

Based on all experiments, the optimal cache configuration is:

Sets: 256
Blocks per set: 4 (4-way set-associative)
Block size: 16 bytes
Total cache size: ~18.75 KB (16 KB data + 2.75 KB overhead)
Write policy: write-allocate + write-back
Eviction policy: LRU

Performance on gcc.trace: 97.55% hit rate, 9,331,848 cycles
Performance on swim.trace: 96.13% hit rate, 8,997,863 cycles



From Experiment 1, we get that 4-way associativity is the best choice. It provides 20.7% fewer misses than direct-mapped while going to 8-way only improves by 1.1%.

From Experiment 2, we get that 16-byte blocks are optimal. They achieve the lowest total execution time at 9.3M cycles. 

From Experiment 3, we get that LRU eviction is better than FIFO. LRU is consistently 5-7% faster by keeping recently-used data in cache. 

From Experiment 4, we get that write-back is much faster than write-through. Write-back is 2.7× faster because stores only update the cache instead of also writing to memory every time.

From Experiment 5, we get that 16 KB is the right cache size. It provides 97.55% hit rate with good efficiency. 

