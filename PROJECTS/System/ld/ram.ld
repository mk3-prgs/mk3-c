/* Entry Point */
ENTRY(startup)

MEMORY
{
    ram (xrw)     : ORIGIN = 0x2000c000, LENGTH = 16K
}

SECTIONS
{
    .vectors :
    {
    _type = .;
	. = ALIGN(4);
        KEEP(*(vectors))    /* Startup code */
	. = ALIGN(4);
    } >ram

    .text :
    {
	    . = ALIGN(4);

        *(.text)                   /* remaining code */
        *(.text.*)                 /* remaining code */
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)

    } >ram

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ram

    .ARM : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > ram

	_data_ram = .;
	_sidata = .;

	.data : AT ( _data_ram )
	{
		. = ALIGN(8);
		_data_begin = .;
		_sdata = .;
		*(.data)
		*(.data*)
		. = ALIGN(8);
		_data_end = .;
		_edata = . ;

		_bdata = . ;
		_idata = . ;
	} > ram

	.bss :
	{
		. = ALIGN(8);
		_bss_begin = .;
		_sbss = .;
		__bss_start__ = _bss_begin;

		*(.bss)
		*(.bss*)
		*(COMMON)

		. = ALIGN(8);
		_bss_end = .;
		_ebss = .;
		__bss_end__ = _bss_end;
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

    . = ALIGN(8);
    _end = . ;
}

