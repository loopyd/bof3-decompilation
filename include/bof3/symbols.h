#ifndef BOF3_SYMBOLS_H
#define BOF3_SYMBOLS_H

#define SYMBOL_AT(name, addr) \
  asm(".weak " #name "\n.set " #name ", " #addr "\n")

#define DATA_AT(type, name, addr) \
  extern type name;               \
  SYMBOL_AT(name, addr)

#define DEFINE_FUNC_AT(ret, name, addr, args) \
  extern ret name args;                       \
  SYMBOL_AT(name, addr)

#endif
