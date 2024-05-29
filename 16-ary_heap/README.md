## \_minpos_
https://github.com/riscv/riscv-v-spec/issues/804

## Pseudorandom int32_t
1e6:
```
$ ./test.sh 

	Heap-sort (w/o `heapify()`): 48 ms

	`qsort()`: 124 ms

```
1e7:
```
$ ./test.sh 

	Heap-sort (w/o `heapify()`): 1203 ms

	`qsort()`: 1451 ms

```
1e8:
```
$ ./test.sh 

	Heap-sort (w/o `heapify()`): 21012 ms

	`qsort()`: 16485 ms

```
