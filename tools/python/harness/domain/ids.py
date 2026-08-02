FUNCTION_ID_FORMAT = "TARGET@0xADDRESS"
FUNCTION_ID_HELP = (
    "TARGET@0xADDRESS; EMI targets may use BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS"
)

    """Parse the shared function selector accepted by harness commands.

    Executables use a target name such as ``SLUS_004.22@0x8014AE08``. An EMI
    entry uses its archive path and slot, for example
    ``BIN/BATTLE/BATL_END.EMI#0@0x800AF66C``.
    """
        raise ValueError(
            f"function ID must be TARGET@8-digit-address ({FUNCTION_ID_HELP})"
        )
