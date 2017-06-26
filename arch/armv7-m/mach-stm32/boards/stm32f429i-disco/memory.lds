MEMORY
{
	sdram (rwx)	: ORIGIN = 0xD0000000, LENGTH = 8M
	ccm (rwx)	: ORIGIN = 0x10000000, LENGTH = 64K
	ram (rwx)	: ORIGIN = 0x20000000, LENGTH = 192K
	rom (rx)	: ORIGIN = 0x08000000, LENGTH = 2048K
}

PROVIDE(_sdram_start   = ORIGIN(sdram));
