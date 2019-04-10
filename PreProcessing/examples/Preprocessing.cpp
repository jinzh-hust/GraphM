
#include "core/graph.hpp"

int main(int argc, char ** argv) {
    if (argc<4)
    {
        fprintf(stderr, "usage: pagerank [path] [cache size in MB] [graph size in MB] [memory budget in GB]\n");
        exit(-1);
    }
    std::string path = argv[1];
    long cache_size = atol(argv[2])*1024l*1024l;
    long graph_size = atol(argv[3])*1024l*1024l;
    long memory_bytes = (argc>=5)?atol(argv[4])*1024l*1024l*1024l:8l*1024l*1024l*1024l;


	Graph graph(path);
	graph.set_memory_bytes(memory_bytes);

	long vertex_data_bytes = (long)graph.vertices * ( sizeof(VertexId) + sizeof(float) + sizeof(float) );
	graph.set_vertex_data_bytes(vertex_data_bytes);

	graph.set_sizeof_chunk(cache_size, graph_size, vertex_data_bytes);

	double begin_time = get_time();

	int pairs = graph.stream_edges<VertexId>(
		[&](Edge & e){
			return 0;
		}, nullptr, 0, 1
	);
	printf("preprocessing used %.2f seconds, and the number of pairs is %d\n", get_time() - begin_time, pairs);
	fflush(stdout);
}
