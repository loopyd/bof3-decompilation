# PSX I/O Registers — fixed addresses, all targets
# GPU
f hwreg.GPU_GP0     4 @ 0x1f801810
CC "GPU GP0 — command/parameter port" @ 0x1f801810
f hwreg.GPU_GP1     4 @ 0x1f801814
CC "GPU GP1 — status/control port" @ 0x1f801814
f hwreg.GPU_TEXTURE 2 @ 0x1f8010f0
CC "GPU texture window" @ 0x1f8010f0

# Timers
f hwreg.TIMER_BASE  4 @ 0x1f801100
CC "Timer 0..2 base" @ 0x1f801100

# CD-ROM
f hwreg.CD_REG0     4 @ 0x1f801800
CC "CD-ROM status/index port" @ 0x1f801800

# SPU
f hwreg.SPU_BASE    4 @ 0x1f801c00
CC "SPU base" @ 0x1f801c00

# DMA
f hwreg.DMA_BASE    4 @ 0x1f801080
CC "DMA base control/status" @ 0x1f801080

# Scratchpad
f hwreg.SCRATCHPAD  4 @ 0x1f800000
CC "Scratchpad base (1 KB)" @ 0x1f800000

fs functions
