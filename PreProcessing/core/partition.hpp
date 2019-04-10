#ifndef PARTITION_H
#define PARTITION_H

inline size_t get_partition_id(const size_t vertices, const size_t partitions, const size_t vertex_id) {
        if (vertices % partitions==0) {
                const size_t partition_size = vertices / partitions;
                return vertex_id / partition_size;
        }
        const size_t partition_size = vertices / partitions + 1;
        const size_t split_point = vertices % partitions * partition_size;
        return (vertex_id < split_point) ? vertex_id / partition_size : (vertex_id - split_point) / (partition_size - 1) + (vertices % partitions);
}

inline std::pair<size_t, size_t> get_partition_range(const size_t vertices, const size_t partitions, const size_t partition_id) {
        const size_t split_partition = vertices % partitions;
        const size_t partition_size = vertices / partitions + 1;
        if (partition_id < split_partition) {
                const size_t begin = partition_id * partition_size;
                const size_t end = (partition_id + 1) * partition_size;
                return std::make_pair(begin, end);
        }
        const size_t split_point = split_partition * partition_size;
        const size_t begin = split_point + (partition_id - split_partition) * (partition_size - 1);
        const size_t end = split_point + (partition_id - split_partition + 1) * (partition_size - 1);
        return std::make_pair(begin, end);
}

#endif
