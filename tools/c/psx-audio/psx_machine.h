#ifndef PSX_AUDIO_PSX_MACHINE_H
#define PSX_AUDIO_PSX_MACHINE_H

#include <stddef.h>
#include <stdint.h>

#include "psf.h"
#include "spu_device.h"

typedef enum {
  PSX_MACHINE_OK = 0,
  PSX_MACHINE_ERROR_ARGUMENT,
  PSX_MACHINE_ERROR_MEMORY,
  PSX_MACHINE_ERROR_ADDRESS,
  PSX_MACHINE_ERROR_INSTRUCTION,
  PSX_MACHINE_ERROR_DEVICE,
  PSX_MACHINE_ERROR_BIOS,
  PSX_MACHINE_ERROR_LIMIT
} PsxMachineStatus;

typedef struct PsxMachine PsxMachine;

typedef struct {
  uint32_t pc;
  uint32_t address;
  uint32_t instruction;
} PsxMachineFault;

PsxMachine*      psx_machine_create(const Psf1Image* image, PsxSpu* spu);
void             psx_machine_destroy(PsxMachine* machine);
PsxMachineStatus psx_machine_run(PsxMachine* machine,
                                 uint64_t    instruction_count);
PsxMachineStatus psx_machine_write_ram(PsxMachine* machine, uint32_t address,
                                       const void* data, size_t size);
PsxMachineStatus psx_machine_call(PsxMachine* machine, uint32_t address,
                                  const uint32_t arguments[4],
                                  uint64_t       instruction_limit);
uint32_t         psx_machine_pc(const PsxMachine* machine);
uint32_t psx_machine_register(const PsxMachine* machine, unsigned index);
uint64_t psx_machine_cycles(const PsxMachine* machine);
const PsxMachineFault* psx_machine_fault(const PsxMachine* machine);
const char*            psx_machine_status_string(PsxMachineStatus status);

#endif
