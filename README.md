# GraphM
It is an efficient storage system for for high throughput of concurrent graph analytics, and a lightweight runtime system which runs in any existing graph processing sytems

# Integrated with existing graph processing systems
Sharing() call is inserted between successive graph loads in the existing systems (e.g., the function EdgeStreams() in GridGraph), and init() call is implemented before the processing. Besides, declarations are made while traversing the graph structure for fine-grained synchronization.

# Compilation
Compilers supporting basic C++11 features (lambdas, threads, etc.) and OpenMP are required, the other requirements and the compiled method are same as the original systems. Take GridGraph as an example:
```
make
```
# Preprocessing
Before running concurrent applications on a graph, the original graph data needs to be first partitioned into the grid format for GridGraph. To partition the original graph data:
```
./bin/preprocess -i [input path] -o [output path] -v [vertices] -p [partitions] -t [edge type: 0=unweighted, 1=weighted]
```
Then, the graph partitions need to be further logically labeled into chunks. In order to label the graph data, just give the size of the last-level cache and the size of the graph data:
```
./bin/Preprocessing [path]  [cache size in MB] [graph size in MB] [memory budget in GB]
```
For example, we want to divide the grid format [LiveJournal](http://snap.stanford.edu/data/soc-LiveJournal1.html) graph into chunks using a machine with 20M Last-level Cache and 8 GB RAM:
```
./bin/Preprocessing /data/LiveJournal 20 526.38 8
```

# Running Applications
We concurrently submmit the PageRank, WCC, BFS, SSSP to GridGraph-M through the concurrent_jobs application. To concurrently run these applications, just need to give the follwing parameters:
```
./bin/concurrent_jobs [path] [number of jobs] [iterations] [start vertex id] [cache size in MB] [graph size in MB] [memory budget in GB
```
