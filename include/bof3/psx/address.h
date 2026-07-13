#ifndef PSX_ADDRESS_H
#define PSX_ADDRESS_H

/*
 * Address casts keep recovered PSX memory maps explicit at call sites.  The
 * macros intentionally do not imply volatility; use vptr/vcptr only when the
 * binary evidence shows a memory-mapped or asynchronously updated location.
 */
#define ptr(type, address)    ((type*)(address))
#define cptr(type, address)   ((const type*)(address))
#define vptr(type, address)   ((volatile type*)(address))
#define vcptr(type, address)  ((const volatile type*)(address))
#define deref(type, address)  (*ptr(type, address))
#define vderef(type, address) (*vptr(type, address))

#endif
