#include <stdlib.h>
#include <string.h>

#include "psx_machine.h"

#define PSX_SCRATCHPAD_BASE 0x1f800000u
#define PSX_SCRATCHPAD_SIZE 0x400u
#define PSX_IO_BASE 0x1f801000u
#define PSX_IO_SIZE 0x1000u

struct PsxMachine {
    uint8_t ram[PSF1_RAM_SIZE];
    uint8_t scratchpad[PSX_SCRATCHPAD_SIZE];
    uint8_t io[PSX_IO_SIZE];
    uint32_t registers[32];
    uint32_t hi;
    uint32_t lo;
    uint32_t pc;
    uint32_t next_pc;
    uint64_t cycles;
    uint32_t pending_load_value;
    unsigned pending_load_register;
    int pending_load_valid;
    PsxSpu *spu;
    PsxMachineFault fault;
};

static PsxMachineStatus memory_pointer(PsxMachine *machine, uint32_t address,
                                       size_t size, uint8_t **pointer);

static int io_range_supported(uint32_t address, size_t size)
{
    uint32_t end = address + (uint32_t)size;
    return (address >= 0x1f801070u && end <= 0x1f801078u) ||
           (address >= 0x1f8010c0u && end <= 0x1f8010d0u) ||
           (address >= 0x1f8010f0u && end <= 0x1f8010f8u) ||
           (address >= 0x1f801100u && end <= 0x1f801130u);
}

static uint8_t *io_pointer(PsxMachine *machine, uint32_t address)
{
    return machine->io + address - PSX_IO_BASE;
}

static uint32_t io_read32(PsxMachine *machine, uint32_t address)
{
    uint8_t *pointer = io_pointer(machine, address);
    return (uint32_t)pointer[0] | ((uint32_t)pointer[1] << 8) |
           ((uint32_t)pointer[2] << 16) | ((uint32_t)pointer[3] << 24);
}

static PsxMachineStatus start_spu_dma(PsxMachine *machine)
{
    uint32_t address = io_read32(machine, 0x1f8010c0u);
    uint32_t block = io_read32(machine, 0x1f8010c4u);
    uint32_t control = io_read32(machine, 0x1f8010c8u);
    uint32_t block_size = block & 0xffffu;
    uint32_t block_count = block >> 16;
    uint64_t words;
    uint64_t size;
    uint8_t *source;
    PsxMachineStatus status;

    if ((control & (1u << 24)) == 0)
        return PSX_MACHINE_OK;
    if ((control & 1u) == 0)
        return PSX_MACHINE_ERROR_DEVICE;
    if (block_size == 0)
        block_size = 0x10000u;
    if (((control >> 9) & 3u) == 1u) {
        if (block_count == 0)
            block_count = 0x10000u;
        words = (uint64_t)block_size * block_count;
    } else {
        words = block_size;
    }
    size = words * 4;
    if (size > PSF1_RAM_SIZE)
        return PSX_MACHINE_ERROR_ADDRESS;
    status = memory_pointer(machine, address, (size_t)size, &source);
    if (status != PSX_MACHINE_OK)
        return status;
    if (psx_spu_dma_write(machine->spu, source, (size_t)size,
                          machine->cycles) != 0)
        return PSX_MACHINE_ERROR_DEVICE;
    control &= ~(1u << 24);
    io_pointer(machine, 0x1f8010c8u)[3] = (uint8_t)(control >> 24);
    return PSX_MACHINE_OK;
}

static uint32_t physical_address(uint32_t address)
{
    if ((address & 0xe0000000u) == 0x80000000u ||
        (address & 0xe0000000u) == 0xa0000000u)
        return address & 0x1fffffffu;
    return address;
}

static PsxMachineStatus memory_pointer(PsxMachine *machine, uint32_t address,
                                       size_t size, uint8_t **pointer)
{
    uint32_t physical = physical_address(address);
    if (physical < PSF1_RAM_SIZE && size <= PSF1_RAM_SIZE - physical) {
        *pointer = machine->ram + physical;
        return PSX_MACHINE_OK;
    }
    if (physical >= PSX_SCRATCHPAD_BASE &&
        physical - PSX_SCRATCHPAD_BASE < PSX_SCRATCHPAD_SIZE &&
        size <= PSX_SCRATCHPAD_SIZE - (physical - PSX_SCRATCHPAD_BASE)) {
        *pointer = machine->scratchpad + physical - PSX_SCRATCHPAD_BASE;
        return PSX_MACHINE_OK;
    }
    machine->fault.address = address;
    return PSX_MACHINE_ERROR_ADDRESS;
}

static PsxMachineStatus read8(PsxMachine *machine, uint32_t address,
                              uint8_t *value)
{
    uint8_t *pointer;
    if (io_range_supported(address, 1)) {
        *value = *io_pointer(machine, address);
        return PSX_MACHINE_OK;
    }
    PsxMachineStatus status = memory_pointer(machine, address, 1, &pointer);
    if (status == PSX_MACHINE_OK)
        *value = *pointer;
    return status;
}

static PsxMachineStatus read16(PsxMachine *machine, uint32_t address,
                               uint16_t *value)
{
    uint8_t *pointer;
    if ((address & 1u) != 0) {
        machine->fault.address = address;
        return PSX_MACHINE_ERROR_ADDRESS;
    }
    if (address >= PSX_SPU_REGISTER_BASE &&
        address < PSX_SPU_REGISTER_BASE + PSX_SPU_REGISTER_SIZE) {
        if (psx_spu_read16(machine->spu, address, value) != 0)
            return PSX_MACHINE_ERROR_DEVICE;
        return PSX_MACHINE_OK;
    }
    if (io_range_supported(address, 2)) {
        pointer = io_pointer(machine, address);
        *value = (uint16_t)(pointer[0] | ((uint16_t)pointer[1] << 8));
        return PSX_MACHINE_OK;
    }
    {
        PsxMachineStatus status = memory_pointer(machine, address, 2, &pointer);
        if (status != PSX_MACHINE_OK)
            return status;
    }
    *value = (uint16_t)(pointer[0] | ((uint16_t)pointer[1] << 8));
    return PSX_MACHINE_OK;
}

static PsxMachineStatus read32(PsxMachine *machine, uint32_t address,
                               uint32_t *value)
{
    uint8_t *pointer;
    PsxMachineStatus status;
    if ((address & 3u) != 0) {
        machine->fault.address = address;
        return PSX_MACHINE_ERROR_ADDRESS;
    }
    if (io_range_supported(address, 4)) {
        pointer = io_pointer(machine, address);
        *value = (uint32_t)pointer[0] | ((uint32_t)pointer[1] << 8) |
                 ((uint32_t)pointer[2] << 16) | ((uint32_t)pointer[3] << 24);
        return PSX_MACHINE_OK;
    }
    status = memory_pointer(machine, address, 4, &pointer);
    if (status != PSX_MACHINE_OK)
        return status;
    *value = (uint32_t)pointer[0] | ((uint32_t)pointer[1] << 8) |
             ((uint32_t)pointer[2] << 16) | ((uint32_t)pointer[3] << 24);
    return PSX_MACHINE_OK;
}

static PsxMachineStatus write8(PsxMachine *machine, uint32_t address,
                               uint8_t value)
{
    uint8_t *pointer;
    if (io_range_supported(address, 1)) {
        *io_pointer(machine, address) = value;
        return PSX_MACHINE_OK;
    }
    PsxMachineStatus status = memory_pointer(machine, address, 1, &pointer);
    if (status == PSX_MACHINE_OK)
        *pointer = value;
    return status;
}

static PsxMachineStatus write16(PsxMachine *machine, uint32_t address,
                                uint16_t value)
{
    uint8_t *pointer;
    if ((address & 1u) != 0) {
        machine->fault.address = address;
        return PSX_MACHINE_ERROR_ADDRESS;
    }
    if (address >= PSX_SPU_REGISTER_BASE &&
        address < PSX_SPU_REGISTER_BASE + PSX_SPU_REGISTER_SIZE) {
        if (psx_spu_write16(machine->spu, address, value, machine->cycles) != 0)
            return PSX_MACHINE_ERROR_DEVICE;
        return PSX_MACHINE_OK;
    }
    if (io_range_supported(address, 2)) {
        pointer = io_pointer(machine, address);
        pointer[0] = (uint8_t)value;
        pointer[1] = (uint8_t)(value >> 8);
        return PSX_MACHINE_OK;
    }
    {
        PsxMachineStatus status = memory_pointer(machine, address, 2, &pointer);
        if (status != PSX_MACHINE_OK)
            return status;
    }
    pointer[0] = (uint8_t)value;
    pointer[1] = (uint8_t)(value >> 8);
    return PSX_MACHINE_OK;
}

static PsxMachineStatus write32(PsxMachine *machine, uint32_t address,
                                uint32_t value)
{
    uint8_t *pointer;
    PsxMachineStatus status;
    if ((address & 3u) != 0) {
        machine->fault.address = address;
        return PSX_MACHINE_ERROR_ADDRESS;
    }
    if (io_range_supported(address, 4)) {
        pointer = io_pointer(machine, address);
        pointer[0] = (uint8_t)value;
        pointer[1] = (uint8_t)(value >> 8);
        pointer[2] = (uint8_t)(value >> 16);
        pointer[3] = (uint8_t)(value >> 24);
        if (address == 0x1f8010c8u)
            return start_spu_dma(machine);
        return PSX_MACHINE_OK;
    }
    status = memory_pointer(machine, address, 4, &pointer);
    if (status != PSX_MACHINE_OK)
        return status;
    pointer[0] = (uint8_t)value;
    pointer[1] = (uint8_t)(value >> 8);
    pointer[2] = (uint8_t)(value >> 16);
    pointer[3] = (uint8_t)(value >> 24);
    return PSX_MACHINE_OK;
}

static int32_t sign_extend16(uint32_t value)
{
    return (int16_t)(value & 0xffffu);
}

static PsxMachineStatus execute_special(PsxMachine *machine, uint32_t instruction)
{
    unsigned rs = (instruction >> 21) & 31u;
    unsigned rt = (instruction >> 16) & 31u;
    unsigned rd = (instruction >> 11) & 31u;
    unsigned shift = (instruction >> 6) & 31u;
    unsigned function = instruction & 63u;

    switch (function) {
    case 0x00: machine->registers[rd] = machine->registers[rt] << shift; break;
    case 0x02: machine->registers[rd] = machine->registers[rt] >> shift; break;
    case 0x03: machine->registers[rd] = (uint32_t)((int32_t)machine->registers[rt] >> shift); break;
    case 0x04: machine->registers[rd] = machine->registers[rt] << (machine->registers[rs] & 31u); break;
    case 0x06: machine->registers[rd] = machine->registers[rt] >> (machine->registers[rs] & 31u); break;
    case 0x07: machine->registers[rd] = (uint32_t)((int32_t)machine->registers[rt] >> (machine->registers[rs] & 31u)); break;
    case 0x08: machine->next_pc = machine->registers[rs]; break;
    case 0x09:
        machine->registers[rd ? rd : 31] = machine->next_pc;
        machine->next_pc = machine->registers[rs];
        break;
    case 0x0c:
        if (machine->registers[4] == 1u)
            machine->registers[2] = 1;
        else if (machine->registers[4] == 2u)
            machine->registers[2] = 0;
        else
            return PSX_MACHINE_ERROR_BIOS;
        break;
    case 0x10: machine->registers[rd] = machine->hi; break;
    case 0x11: machine->hi = machine->registers[rs]; break;
    case 0x12: machine->registers[rd] = machine->lo; break;
    case 0x13: machine->lo = machine->registers[rs]; break;
    case 0x18: {
        int64_t product = (int64_t)(int32_t)machine->registers[rs] *
                          (int64_t)(int32_t)machine->registers[rt];
        machine->lo = (uint32_t)product;
        machine->hi = (uint32_t)((uint64_t)product >> 32);
        break;
    }
    case 0x19: {
        uint64_t product = (uint64_t)machine->registers[rs] *
                           (uint64_t)machine->registers[rt];
        machine->lo = (uint32_t)product;
        machine->hi = (uint32_t)(product >> 32);
        break;
    }
    case 0x1a: {
        int32_t dividend = (int32_t)machine->registers[rs];
        int32_t divisor = (int32_t)machine->registers[rt];
        if (divisor == 0) {
            machine->lo = dividend < 0 ? 1u : 0xffffffffu;
            machine->hi = (uint32_t)dividend;
        } else if (dividend == INT32_MIN && divisor == -1) {
            machine->lo = (uint32_t)INT32_MIN;
            machine->hi = 0;
        } else {
            machine->lo = (uint32_t)(dividend / divisor);
            machine->hi = (uint32_t)(dividend % divisor);
        }
        break;
    }
    case 0x1b:
        if (machine->registers[rt] == 0) {
            machine->lo = 0xffffffffu;
            machine->hi = machine->registers[rs];
        } else {
            machine->lo = machine->registers[rs] / machine->registers[rt];
            machine->hi = machine->registers[rs] % machine->registers[rt];
        }
        break;
    case 0x20:
    case 0x21: machine->registers[rd] = machine->registers[rs] + machine->registers[rt]; break;
    case 0x22:
    case 0x23: machine->registers[rd] = machine->registers[rs] - machine->registers[rt]; break;
    case 0x24: machine->registers[rd] = machine->registers[rs] & machine->registers[rt]; break;
    case 0x25: machine->registers[rd] = machine->registers[rs] | machine->registers[rt]; break;
    case 0x26: machine->registers[rd] = machine->registers[rs] ^ machine->registers[rt]; break;
    case 0x27: machine->registers[rd] = ~(machine->registers[rs] | machine->registers[rt]); break;
    case 0x2a: machine->registers[rd] = (int32_t)machine->registers[rs] < (int32_t)machine->registers[rt]; break;
    case 0x2b: machine->registers[rd] = machine->registers[rs] < machine->registers[rt]; break;
    default: return PSX_MACHINE_ERROR_INSTRUCTION;
    }
    return PSX_MACHINE_OK;
}

static PsxMachineStatus execute(PsxMachine *machine, uint32_t instruction)
{
    unsigned opcode = instruction >> 26;
    unsigned rs = (instruction >> 21) & 31u;
    unsigned rt = (instruction >> 16) & 31u;
    uint32_t immediate = instruction & 0xffffu;
    uint32_t address;
    uint32_t value32;
    uint16_t value16;
    uint8_t value8;
    PsxMachineStatus status;

    if (opcode == 0)
        return execute_special(machine, instruction);
    switch (opcode) {
    case 0x02:
        machine->next_pc = (machine->pc & 0xf0000000u) |
                           ((instruction & 0x03ffffffu) << 2);
        break;
    case 0x03:
        machine->registers[31] = machine->next_pc;
        machine->next_pc = (machine->pc & 0xf0000000u) |
                           ((instruction & 0x03ffffffu) << 2);
        break;
    case 0x04:
        if (machine->registers[rs] == machine->registers[rt])
            machine->next_pc = machine->pc + ((uint32_t)sign_extend16(immediate) << 2);
        break;
    case 0x05:
        if (machine->registers[rs] != machine->registers[rt])
            machine->next_pc = machine->pc + ((uint32_t)sign_extend16(immediate) << 2);
        break;
    case 0x08:
    case 0x09: machine->registers[rt] = machine->registers[rs] + (uint32_t)sign_extend16(immediate); break;
    case 0x0a: machine->registers[rt] = (int32_t)machine->registers[rs] < sign_extend16(immediate); break;
    case 0x0b: machine->registers[rt] = machine->registers[rs] < (uint32_t)sign_extend16(immediate); break;
    case 0x0c: machine->registers[rt] = machine->registers[rs] & immediate; break;
    case 0x0d: machine->registers[rt] = machine->registers[rs] | immediate; break;
    case 0x0e: machine->registers[rt] = machine->registers[rs] ^ immediate; break;
    case 0x0f: machine->registers[rt] = immediate << 16; break;
    case 0x20:
    case 0x24:
        address = machine->registers[rs] + (uint32_t)sign_extend16(immediate);
        status = read8(machine, address, &value8);
        if (status != PSX_MACHINE_OK) return status;
        machine->pending_load_register = rt;
        machine->pending_load_value = opcode == 0x20 ?
            (uint32_t)(int32_t)(int8_t)value8 : value8;
        machine->pending_load_valid = 1;
        break;
    case 0x21:
    case 0x25:
        address = machine->registers[rs] + (uint32_t)sign_extend16(immediate);
        status = read16(machine, address, &value16);
        if (status != PSX_MACHINE_OK) return status;
        machine->pending_load_register = rt;
        machine->pending_load_value = opcode == 0x21 ?
            (uint32_t)(int32_t)(int16_t)value16 : value16;
        machine->pending_load_valid = 1;
        break;
    case 0x23:
        address = machine->registers[rs] + (uint32_t)sign_extend16(immediate);
        status = read32(machine, address, &value32);
        if (status != PSX_MACHINE_OK) return status;
        machine->pending_load_register = rt;
        machine->pending_load_value = value32;
        machine->pending_load_valid = 1;
        break;
    case 0x28:
        address = machine->registers[rs] + (uint32_t)sign_extend16(immediate);
        status = write8(machine, address, (uint8_t)machine->registers[rt]);
        if (status != PSX_MACHINE_OK) return status;
        break;
    case 0x29:
        address = machine->registers[rs] + (uint32_t)sign_extend16(immediate);
        status = write16(machine, address, (uint16_t)machine->registers[rt]);
        if (status != PSX_MACHINE_OK) return status;
        break;
    case 0x2b:
        address = machine->registers[rs] + (uint32_t)sign_extend16(immediate);
        status = write32(machine, address, machine->registers[rt]);
        if (status != PSX_MACHINE_OK) return status;
        break;
    default:
        return PSX_MACHINE_ERROR_INSTRUCTION;
    }
    return PSX_MACHINE_OK;
}

static PsxMachineStatus execute_bios(PsxMachine *machine)
{
    uint32_t vector = physical_address(machine->pc);
    uint32_t function = machine->registers[9];

    if (vector != 0xa0u && vector != 0xb0u && vector != 0xc0u)
        return PSX_MACHINE_ERROR_ADDRESS;

    if (vector == 0xa0u && function == 0x13u) {
        machine->registers[2] = 0;
        machine->pc = machine->registers[31];
        machine->next_pc = machine->pc + 4;
        return PSX_MACHINE_OK;
    }
    if (vector == 0xa0u && (function == 0x39u || function == 0x3fu ||
                            function == 0x72u)) {
        machine->pc = machine->registers[31];
        machine->next_pc = machine->pc + 4;
        return PSX_MACHINE_OK;
    }
    if (vector == 0xb0u && (function == 0x18u || function == 0x19u ||
                            function == 0x3fu || function == 0x5au ||
                            function == 0x5bu)) {
        machine->registers[2] = 0;
        machine->pc = machine->registers[31];
        machine->next_pc = machine->pc + 4;
        return PSX_MACHINE_OK;
    }
    if (vector == 0xc0u && function == 0x0au) {
        machine->registers[2] = 0;
        machine->pc = machine->registers[31];
        machine->next_pc = machine->pc + 4;
        return PSX_MACHINE_OK;
    }

    machine->fault.pc = machine->pc;
    machine->fault.address = vector;
    machine->fault.instruction = function;
    return PSX_MACHINE_ERROR_BIOS;
}

PsxMachine *psx_machine_create(const Psf1Image *image, PsxSpu *spu)
{
    PsxMachine *machine;
    if (!image || !image->ram || !spu)
        return NULL;
    machine = (PsxMachine *)calloc(1, sizeof(*machine));
    if (!machine)
        return NULL;
    memcpy(machine->ram, image->ram, PSF1_RAM_SIZE);
    machine->pc = image->initial_pc;
    machine->next_pc = image->initial_pc + 4;
    machine->registers[29] = image->initial_sp;
    machine->spu = spu;
    return machine;
}

void psx_machine_destroy(PsxMachine *machine)
{
    free(machine);
}

PsxMachineStatus psx_machine_run(PsxMachine *machine,
                                 uint64_t instruction_count)
{
    uint64_t i;
    if (!machine)
        return PSX_MACHINE_ERROR_ARGUMENT;
    memset(&machine->fault, 0, sizeof(machine->fault));
    for (i = 0; i < instruction_count; i++) {
        uint32_t instruction;
        uint32_t current_pc = machine->pc;
        uint32_t delayed_value = machine->pending_load_value;
        unsigned delayed_register = machine->pending_load_register;
        int delayed_valid = machine->pending_load_valid;
        PsxMachineStatus status;

        if (physical_address(current_pc) == 0xa0u ||
            physical_address(current_pc) == 0xb0u ||
            physical_address(current_pc) == 0xc0u) {
            status = execute_bios(machine);
            machine->cycles++;
            if (status != PSX_MACHINE_OK)
                return status;
            continue;
        }

        status = read32(machine, current_pc, &instruction);
        if (status != PSX_MACHINE_OK) {
            machine->fault.pc = current_pc;
            return status;
        }
        machine->pc = machine->next_pc;
        machine->next_pc += 4;
        machine->pending_load_valid = 0;
        status = execute(machine, instruction);
        if (delayed_valid && delayed_register != 0)
            machine->registers[delayed_register] = delayed_value;
        machine->registers[0] = 0;
        machine->cycles++;
        if (status != PSX_MACHINE_OK) {
            machine->fault.pc = current_pc;
            machine->fault.instruction = instruction;
            return status;
        }
    }
    return PSX_MACHINE_OK;
}

uint32_t psx_machine_pc(const PsxMachine *machine)
{
    return machine ? machine->pc : 0;
}

uint32_t psx_machine_register(const PsxMachine *machine, unsigned index)
{
    return machine && index < 32 ? machine->registers[index] : 0;
}

uint64_t psx_machine_cycles(const PsxMachine *machine)
{
    return machine ? machine->cycles : 0;
}

const PsxMachineFault *psx_machine_fault(const PsxMachine *machine)
{
    return machine ? &machine->fault : NULL;
}

const char *psx_machine_status_string(PsxMachineStatus status)
{
    switch (status) {
    case PSX_MACHINE_OK: return "ok";
    case PSX_MACHINE_ERROR_ARGUMENT: return "invalid machine argument";
    case PSX_MACHINE_ERROR_MEMORY: return "out of memory";
    case PSX_MACHINE_ERROR_ADDRESS: return "unsupported or unaligned address";
    case PSX_MACHINE_ERROR_INSTRUCTION: return "unsupported instruction";
    case PSX_MACHINE_ERROR_DEVICE: return "device access failed";
    case PSX_MACHINE_ERROR_BIOS: return "unsupported BIOS call";
    default: return "unknown machine error";
    }
}
