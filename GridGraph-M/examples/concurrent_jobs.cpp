#include <unistd.h>
#include <pthread.h>
#include "core/graph.hpp"


#define MAX_DEPTH 100000000

int main(int argc, char ** argv)
{
    if (argc<3)
    {
        fprintf(stderr, "usage: pagerank [path] [jobs] [iterations] [start vertex id] [cache size in MB] [graph size in MB] [memory budget in GB]\n");
        exit(-1);
    }
    std::string path = argv[1];
    int PRO_NUM = atoi(argv[2]);
    int iterations = atoi(argv[3]);
    VertexId start_vid = atoi(argv[4]);
    long cache_size = atol(argv[5])*1024l*1024l;
    long graph_size = atol(argv[6])*1024l*1024l;
    long memory_bytes = (argc>=8)?atol(argv[7])*1024l*1024l*1024l:8l*1024l*1024l*1024l;

    int parallelism = std::thread::hardware_concurrency();
    if(parallelism>PRO_NUM)
        parallelism=PRO_NUM;

    double begin_time = get_time();
    Graph graph(path);
    graph.set_memory_bytes(memory_bytes);

    BigVector<float> pagerank[PRO_NUM];
    BigVector<float> sum[PRO_NUM];
    BigVector<VertexId> degree(graph.path+"/degree", graph.vertices);

    long vertex_data_bytes = (long)graph.vertices * ( sizeof(VertexId)+ sizeof(float) + sizeof(float));
    graph.set_vertex_data_bytes(vertex_data_bytes);
    degree.fill(0);

    VertexId active_vertices = 1;
    //sssp
    Bitmap * active_in_sssp[PRO_NUM];
    Bitmap * active_out_sssp[PRO_NUM];
    BigVector<VertexId> parent[PRO_NUM];

    //bfs
    Bitmap * active_in_bfs[PRO_NUM];
    Bitmap * active_out_bfs[PRO_NUM];
    BigVector<VertexId> depth[PRO_NUM];

    //wcc
    Bitmap * active_in_wcc[PRO_NUM];
    Bitmap * active_out_wcc[PRO_NUM];
    BigVector<VertexId> label[PRO_NUM];

    #pragma omp parallel for schedule(dynamic) num_threads(parallelism)
    for(int i = 0; i< PRO_NUM; i++){
        pagerank[i].init(graph.path+"/pagerank"+std::to_string(i), graph.vertices);
        sum[i].init(graph.path+"/sum"+std::to_string(i), graph.vertices);

        active_in_sssp[i] = graph.alloc_bitmap();
        active_out_sssp[i] = graph.alloc_bitmap();
        depth[i].init(graph.path+"/depth"+std::to_string(i), graph.vertices);
        active_out_sssp[i]->clear();
        active_out_sssp[i]->set_bit(start_vid);
        depth[i].fill(MAX_DEPTH);
        depth[i][start_vid] = 0;
        active_out_sssp[i]->fill();

        active_in_bfs[i] = graph.alloc_bitmap();
        active_out_bfs[i] = graph.alloc_bitmap();
        parent[i].init(graph.path+"/parent"+std::to_string(i), graph.vertices);
        //graph.set_vertex_data_bytes( graph.vertices * sizeof(VertexId) );
        active_out_bfs[i]->clear();
        active_out_bfs[i]->set_bit(start_vid);
        parent[i].fill(-1);
        parent[i][start_vid] = start_vid;
        active_out_bfs[i]->fill();


        active_in_wcc[i] = graph.alloc_bitmap();
        active_out_wcc[i] = graph.alloc_bitmap();
        label[i].init(graph.path+"/label"+std::to_string(i), graph.vertices);
        //graph.set_vertex_data_bytes( graph.vertices * sizeof(VertexId));
        active_out_wcc[i]->fill();
    }
    #pragma omp barrier

    graph.set_sizeof_blocks(cache_size, graph_size, vertex_data_bytes*4*PRO_NUM);

    graph.get_should_access_shard(graph.should_access_shard, nullptr);
    graph.stream_edges<VertexId>(
        [&](Edge & e){
        write_add(&degree[e.source], 1);

        return 0;
    },[&](Edge & e){return 0;},
    [&](Edge & e){return 0;},
    [&](Edge & e){return 0;}, nullptr, 0, 0
    );
    printf("degree calculation used %.2f seconds\n", get_time() - begin_time);
    fflush(stdout);

    graph.hint(pagerank[0], sum[0]);
    graph.stream_vertices<VertexId>(
        [&](VertexId i)
    {
        for(int j = 0; j < PRO_NUM; j++){
            //pagerank
            pagerank[j][i] = 1.f / degree[i];
            sum[j][i] = 0;
            //wcc
            label[j][i] = i;
        }
        return 0;
    }, nullptr, 0,
    [&](std::pair<VertexId,VertexId> vid_range)
    {
        for(int i = 0; i < PRO_NUM; i++)
        {
            pagerank[i].load(vid_range.first, vid_range.second);
            sum[i].load(vid_range.first, vid_range.second);
        }
    },
    [&](std::pair<VertexId,VertexId> vid_range)
    {
        for(int i = 0; i < PRO_NUM; i++)
        {
            pagerank[i].save();
            sum[i].save();
        }
    }
    );


    graph.get_should_access_shard(graph.should_access_shard_pagerank, nullptr);
    for (int iter=0; (iter<iterations || active_vertices!=0) && iter < iterations; iter++){
        printf("%7d: %d\n", iter, active_vertices);
        graph.hint(parent[0]);

        if(active_vertices!=0){
            graph.clear_should_access_shard(graph.should_access_shard_sssp);
            graph.clear_should_access_shard(graph.should_access_shard_bfs);
            graph.clear_should_access_shard(graph.should_access_shard_wcc);

            std::swap(active_in_sssp[0], active_out_sssp[0]);
            active_out_sssp[0]->clear();
            graph.get_should_access_shard(graph.should_access_shard_sssp, active_in_sssp[0]);

            std::swap(active_in_bfs[0], active_out_bfs[0]);
            active_out_bfs[0]->clear();
            graph.get_should_access_shard(graph.should_access_shard_bfs, active_in_bfs[0]);

            #pragma omp parallel for schedule(dynamic) num_threads(parallelism)
            for(int i = 0; i < PRO_NUM; i++){
                std::swap(active_in_wcc[i], active_out_wcc[i]);
                active_out_wcc[i]->clear();
                graph.get_should_access_shard(graph.should_access_shard_wcc, active_in_wcc[i]);
            }
            #pragma omp barrier
        }

        graph.get_global_should_access_shard(graph.should_access_shard_wcc, graph.should_access_shard_pagerank,
                                             graph.should_access_shard_bfs,graph.should_access_shard_sssp);
        active_vertices = graph.stream_edges<VertexId>([&](Edge & e){
            //pagerank
            for(int i = 0; i < PRO_NUM; i++){
                write_add(&sum[i][e.target], pagerank[i][e.source]);
            }
            return 0;
        }, [&](Edge & e){
            //SSSP
            int return_state = 0;
            if(active_in_sssp[0] -> get_bit(e.source)){
                for(int i = 0; i < PRO_NUM; i++){
                    int r = depth[i][e.target];
                    int n = depth[i][e.source]+ e.weight;
                    if(n < r){
                        if (cas(&depth[i][e.target], r, n)){
                            active_out_sssp[0]->set_bit(e.target);
                            return_state = 1;
                        }
                    }
                }
            }
            return return_state;
        },[&](Edge & e){
            //bfs
            int return_state = 0;
            if(active_in_bfs[0] ->get_bit(e.source)){
                for(int i = 0; i < PRO_NUM; i++){
                    if (parent[i][e.target]==-1){
                        if (cas(&parent[i][e.target], -1, e.source)){
                            active_out_bfs[0]->set_bit(e.target);
                            return_state = 1;
                        }
                    }
                }
            }
            return return_state;
        }, [&](Edge & e){
            //wcc
            int return_state = 0;
            for(int i = 0; i < PRO_NUM; i++){
                if(active_in_wcc[i] ->get_bit(e.source)){
                    if (label[i][e.source]<label[i][e.target]){
                        if (write_min(&label[i][e.target], label[i][e.source])){
                            active_out_wcc[i]->set_bit(e.target);
                            return_state = 1;
                        }
                    }
                }
            }
            return return_state;
        }, nullptr, 0, 1,
        [&](std::pair<VertexId,VertexId> source_vid_range)
        {
            for(int i = 0; i < PRO_NUM; i++)
                pagerank[i].lock(source_vid_range.first, source_vid_range.second);
        },
        [&](std::pair<VertexId,VertexId> source_vid_range)
        {
            for(int i = 0; i < PRO_NUM; i++)
                pagerank[i].unlock(source_vid_range.first, source_vid_range.second);
        }
        );

        graph.hint(pagerank[0], sum[0]);
        if (iter==iterations-1)
        {
            graph.stream_vertices<VertexId>(
                [&](VertexId i)
            {
                for(int j = 0; j < PRO_NUM; j++){
                    pagerank[j][i] = 0.15f + 0.85f * sum[j][i];
                }
                return 0;
            }, nullptr, 0,
            [&](std::pair<VertexId,VertexId> vid_range)
            {
                for(int j = 0; j < PRO_NUM; j++)
                {
                    pagerank[j].load(vid_range.first, vid_range.second);
                }
            },
            [&](std::pair<VertexId,VertexId> vid_range)
            {
                for(int j = 0; j < PRO_NUM; j++)
                {
                    pagerank[j].save();
                }
            }
            );
        }
        else
        {
            graph.stream_vertices<float>(
                [&](VertexId i)
            {
                for(int j = 0; j < PRO_NUM; j++){
                    pagerank[j][i] = (0.15f + 0.85f * sum[j][i]) / degree[i];
                    sum[j][i] = 0;
                }
                return 0;
            }, nullptr, 0,
            [&](std::pair<VertexId,VertexId> vid_range)
            {
                for(int j = 0; j < PRO_NUM; j++){
                    pagerank[j].load(vid_range.first, vid_range.second);
                    sum[j].load(vid_range.first, vid_range.second);
                }
            },
            [&](std::pair<VertexId,VertexId> vid_range)
            {
                for(int j = 0; j < PRO_NUM; j++){
                    pagerank[j].save();
                    sum[j].save();
                }
            }
            );
        }
    }
    double end_time = get_time();

    printf("%d iterations of concurrent jobs took %.2f seconds\n", iterations, end_time - begin_time);
}
