#ifndef BITMAP_H
#define BITMAP_H

#define WORD_OFFSET(i) (i >> 6)
#define BIT_OFFSET(i) (i & 0x3f)

class Bitmap {
public:
	size_t size;
	unsigned long * data;
	Bitmap() {
		size = 0;
		data = NULL;
	}
	Bitmap(size_t size) {
		init(size);
	}
	void init(size_t size) {
		this->size = size;
		data = new unsigned long [WORD_OFFSET(size)+1];
	}
	void clear() {
		size_t bm_size = WORD_OFFSET(size);
		#pragma omp parallel for
		for (size_t i=0;i<=bm_size;i++) {
			data[i] = 0;
		}
		#pragma omp barrier
	}
	void fill() {
		size_t bm_size = WORD_OFFSET(size);
		#pragma omp parallel for
		for (size_t i=0;i<bm_size;i++) {
			data[i] = 0xffffffffffffffff;
		}
		#pragma omp barrier
		data[bm_size] = 0;
		for (size_t i=(bm_size<<6);i<size;i++) {
			data[bm_size] |= 1ul << BIT_OFFSET(i);
		}
	}
	unsigned long get_bit(size_t i) {
		return data[WORD_OFFSET(i)] & (1ul<<BIT_OFFSET(i));
	}
	void set_bit(size_t i) {
		__sync_fetch_and_or(data+WORD_OFFSET(i), 1ul<<BIT_OFFSET(i));
	}
};

#endif
