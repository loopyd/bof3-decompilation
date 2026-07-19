# compat/capcom97/psyq.mk — PsyQ/BIOS SDK symbol layer toggle
#
# The extracted PsyQ/BIOS symbols live as weak address bindings
# (WEAK_SYMBOL_AT) in each target's symbols/psyq.c, sourced from the shared
# catalog config/sdk/psyq-slus-symbols.txt (and psyq-logo-symbols.txt for the
# logo binary). They are overridable one symbol at a time by a real SDK library
# (see include/bof3/symbols.h).
#
# USE_PSYQ_BINDINGS=1 (default): compile the extracted weak-binding layer.
#    Current behavior; the SDK functions are satisfied by their original-binary
#    addresses. No SDK bodies are lifted.
#
# USE_PSYQ_BINDINGS=0: omit the extracted bindings and link the real PsyQ 4.7
#    SDK objects instead. The .a archives live under toolchains/psyq/4.7/lib.
#    Wiring the real-lib link line is deferred; this switch only gates the
#    extracted layer today.

USE_PSYQ_BINDINGS ?= 1

ifeq ($(USE_PSYQ_BINDINGS),1)
  # Extracted weak-binding layer is active (current behavior). Nothing to add:
  # src/**/symbols/psyq.c are compiled like any other source.
  PSYQ_BINDINGS_ACTIVE := 1
else
  # Future: PSYQ_LIBS := $(ROOT)/toolchains/psyq/4.7/lib/libapi.a ...
  # Future: LDLIBS += $(PSYQ_LIBS)
  PSYQ_BINDINGS_ACTIVE :=
endif
