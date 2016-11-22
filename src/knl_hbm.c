#include <stdint.h>
#include "knl_hbm.h"
#include "numa_common.h"

#define CPUID_MODEL_MASK        (0xf<<4)
#define CPUID_EXT_MODEL_MASK    (0xf<<16)

/*
 * This code is mostly taken from memkind: check to see if the CPU is an
 * Intel Xeon Phi x200 by looking at CPU ID, and if it is, the HBM nodes
 * are the ones that aren't also CPU nodes.
 */
int sicm_knl_hbm_add(struct sicm_device* device_list, int idx, struct bitmask* numa_mask) {
  int i;
  uint32_t xeon_phi_model = (0x7<<4);
  uint32_t xeon_phi_ext_model = (0x5<<16);
  uint32_t registers[4];
  uint32_t expected = xeon_phi_model | xeon_phi_ext_model;
  asm volatile("cpuid":"=a"(registers[0]),
                         "=b"(registers[1]),
                         "=c"(registers[2]),
                         "=d"(registers[2]):"0"(1), "2"(0));
  uint32_t actual = registers[0] & (CPUID_MODEL_MASK | CPUID_EXT_MODEL_MASK);

  if (actual == expected) {
    struct bitmask* cpumask = sicm_cpu_mask();
    for(i = 0; i <= numa_max_node(); i++) {
      if(!numa_bitmask_isbitset(cpumask, i)) {
        numa_bitmask_setbit(numa_mask, i);
        ((int*)device_list[idx].payload)[0] = i;
        device_list[idx].ty = SICM_KNL_HBM;
        device_list[idx].move_ty = SICM_MOVER_NUMA;
        device_list[idx].move_payload.numa = i;
        device_list[idx].alloc = sicm_numa_common_alloc;
        device_list[idx].free = sicm_numa_common_free;
        device_list[idx].used = sicm_numa_common_used;
        device_list[idx].capacity = sicm_numa_common_capacity;
        device_list[idx].add_to_bitmask = sicm_numa_common_add_to_bitmask;
        idx++;
      }
    }
  }
  return idx;
}

struct sicm_device_spec sicm_knl_hbm_spec() {
  struct sicm_device_spec spec;
  spec.non_numa_count = zero;
  spec.add_devices = sicm_knl_hbm_add;
  return spec;
}