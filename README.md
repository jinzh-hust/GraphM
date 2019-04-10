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
Then, the graph partitions need to be further logically divided into chunks:
```
./bin/preprocessing -i [input path] -o [output path] -v [vertices] -p [partitions] -t [edge type: 0=unweighted, 1=weighted]
```
