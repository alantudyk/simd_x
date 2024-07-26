```
$ ./test.sh

	500359 values are less than 555

```
## Compact to the right
```c
const uint32_t mask = _mm256_movemask_epi8(v_reg),
                cnt = __builtin_popcount(mask),
     left_compacted = _pext_u32(0x76543210, mask),
    right_compacted = left_compacted << (32 - cnt);
```
