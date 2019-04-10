#ifndef TYPE_H
#define TYPE_H

typedef int VertexId;
typedef long EdgeId;
typedef float Weight;

struct Edge {
	VertexId source;
	VertexId target;
	Weight weight;
};

struct MergeStatus {
  int id;
  long begin_offset;
  long end_offset;
};

#endif
