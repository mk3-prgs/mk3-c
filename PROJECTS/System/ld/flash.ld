/* Entry Point */
ENTRY(startup)

MEMORY
{
    ram (xrw)     : ORIGIN = 0x2000c000, LENGTH = 16K
    flash (xrw)   : ORIGIN = 0x08040000, LENGTH = 64K
}

SECTIONS
{
    .vectors :
    {
    _type = .;
	. = ALIGN(4);
        KEEP(*(vectors))    /* Startup code */
	. = ALIGN(4);
    } >flash

    .text :
    {
	    . = ALIGN(4);

        *(.text)                   /* remaining code */
        *(.text.*)                 /* remaining code */
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)

    } >flash

    .text.end :
    {
    . = ALIGN(4);
     _etext = .;
     _idata = _etext;
    } > flash

    . = ALIGN(4);
	_data_ram = .;
	.data : AT ( _data_ram )
	{
		. = ALIGN(4);
		_data_begin = .;
		_bdata = .;
		*(.data)
		*(.data*)
		. = ALIGN(4);
		_data_end = .;
		_edata = . ;
	} > ram

	.bss :
	{
		. = ALIGN(4);
		_bss_begin = .;
		_sbss = .;
		__bss_start__ = _bss_begin;

		*(.bss)
		*(.bss*)
		*(COMMON)

		. = ALIGN(4);
		_bss_end = .;
		_ebss = .;
		__bss_end__ = _bss_end;
	} > ram

	.ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ram

    .ARM : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > ram

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 :
    {
        *(.ARM.attributes)
    }

    . = ALIGN(4);
    _end = . ;
    end = . ;
}

