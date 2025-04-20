OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
  /*
   * ensure that entry.S / _entry is at 0x80000000,
   * where qemu's -kernel jumps.
   */
  . = 0x80000000;

  .text : {
    *(.text .text.*)
    . = ALIGN(0x1000);

    _trampoline0 = .;
    *(trampsec0)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline0 == 0x1000, "error: trampoline0 larger than 1 page");


    _trampoline1 = .;
    *(trampsec1)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline1 == 0x1000, "error: trampoline1 larger than 1 page");


    _trampoline2 = .;
    *(trampsec2)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline2 == 0x1000, "error: trampoline2 larger than 1 page");


    _trampoline3 = .;
    *(trampsec3)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline3 == 0x1000, "error: trampoline3 larger than 1 page");


    _dummy = .;
    *(dummysec)
    . = ALIGN(0x1000);
    ASSERT(. - _dummy == 0x1000, "error: _dummy larger than one page");

    PROVIDE(etext = .);
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  }

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .);
}
