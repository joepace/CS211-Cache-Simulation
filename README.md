# CS211-Cache-Simulation
Create a simulation of various types of one level, FIFO caches.

To be quite honest, this project was a challenge for me, especially since I have mono. My main problem I encountered was misunderstanding how exactly I was supposed to construct structs to represent the cache from the given inputs. Once I overcame that issue, it was just a matter of making sure memory was allocated properly and rearranges some things. Completing the cache B simulation was extremely simple as it just involved moving around a few lines of code. I'm sure there was a aesthetically pleasing (and possibly more efficient) way of doing this, such as creating a second cache and running the simulation simultaneously, but my main focus was just getting the program to work while sick and under time pressure.

The data structures I utilized in my program were a structure similar to a hash table and a linked list. The "hash table" was a triple-nested array structure. The first level acted as a pointer to the rest of the structure. The next level represented the sets of a cache, which each pointed to an array of blocks, which pointed to a line array that held the tag. The linked list served as a primitive queue structure in order to implement the First In First Out (FIFO) replacement policy. 

Based on my results from this program, cache A gives a better cache hit ratio.  This is obviously due to the difference in how the original memory address is split up. The only situation where this is not the case and the ratios are equal is in a fully associative cache where the set index is not taken into consideration. In cache A, the order is: tag bits, set indexing bits, then block offset bits. In cache B, the order is: set indexing bits, tag bits, then block offset bits. Since cache B is using the beginning bits as index bits for the sets within the cache, there will be more collisions as, more often than not, the first bits are all 0's. The tag bits will also be more unique as it will contain more of the significant bits from the memory address, making it harder to match. Thus, the addressing convention used in cache A is better than that of cache B in terms of a cache hit ratio. 
