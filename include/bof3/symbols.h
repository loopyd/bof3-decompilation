#ifndef SYMBOLS_H
#define SYMBOLS_H

/*
 * Weak original-binary address bindings. A normal strong C definition with
 * the same name overrides the absolute symbol, so data and functions can be
 * replaced one at a time without changing their callers. This is the only
 * authored assembly helper; never use it inside an executable function body.
 */
#define WEAK_SYMBOL_AT(name, addr) \
  asm(".weak " #name "\n.set " #name ", " #addr "\n")

#endif
