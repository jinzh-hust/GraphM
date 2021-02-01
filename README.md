# GraphM
It is an efficient storage system for high throughput of concurrent graph analytics, and a lightweight runtime system which runs in any existing graph processing sytems. Specifically, GraphM can be integrated into the existing graph processing systems by using a few APIs provided by GraphM, thereby efficiently supportting concurrent iterative graph processing jobs for higher throughput by fully exploiting the similarities of the data accesses between these concurrent jobs.

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
./bin/concurrent_jobs [path] [number of submissions] [number of iterations] [start vertex id] [cache size in MB] [graph size in MB] [memory budget in GB]
```
For example, to run 10 iterations of above four algorithms as eight jobs (i.e., submitting the same job twice in succession) on the LiveJournal:
```
./bin/concurrent_jobs /data/LiveJournal 2 10 0 20 526.38 8
```

## Resources
Jin Zhao, Yu Zhang, Xiaofei Liao, Ligang He, Bingsheng He, Hai Jin, Haikun Liu, and Yicheng Chen. [GraphM: An Efficient Storage System for High Throughput of Concurrent Graph Processing](https://dl.acm.org/doi/10.1145/3295500.3356143). Proceedings of the 2019 International Conference for High Performance Computing, Networking, Storage, and Analysis.

To cite GraphM, you can use the following BibTeX entry:
```
@inproceedings{DBLP:conf/sc/ZhaoZLHH0LC19,
  author    = {Jin Zhao and Yu Zhang and Xiaofei Liao and Ligang He and Bingsheng He and Hai Jin and Haikun Liu and Yicheng Chen},
  title     = {GraphM: an efficient storage system for high throughput of concurrent graph processing},
  booktitle = {Proceedings of the International Conference for High Performance Computing,
               Networking, Storage and Analysis, {SC} 2019, Denver, Colorado, USA,
               November 17-19, 2019},
  pages     = {3:1--3:14},
  publisher = {{ACM}},
  year      = {2019},
  url       = {https://doi.org/10.1145/3295500.3356143},
  doi       = {10.1145/3295500.3356143},
  timestamp = {Mon, 10 Aug 2020 08:13:11 +0200},
  biburl    = {https://dblp.org/rec/conf/sc/ZhaoZLHH0LC19.bib},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}
```
