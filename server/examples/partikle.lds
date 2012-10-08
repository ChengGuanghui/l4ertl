/*
 * $FILE$
 * 
 * ld script to build arm
 * 
 * $VERSION$
 *
 * Author: Miguel Masmano <mimastel@doctor.upv.es>
 *  
 * $LICENSE$
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)


SECTIONS
{
  . = 0x90000000;
  _spartikle = .;
  _epartikle = .;
  . = ALIGN(4);
  _sapp = .;
  .text ALIGN(0x1000): {
	_stext_app = .;
        * (.text)
	_etext_app = .;
  }
  .rodata ALIGN (0x1000) : {
	_srodata_app = .;
	* (.rodata)
  	* (.rodata.*)
	* (.rodata.*.*)

	. = ALIGN(4);
	__CTOR_LIST__ = .;
	LONG(-1)
	* (.ctors)
	. = ALIGN(4);
	__CTOR_END__ = .;
	LONG(0)

	. = ALIGN(4);
	__DTOR_LIST__ = .;
	LONG(-1)
	* (.dtors)
	. = ALIGN(4);
	__DTOR_END__ = .;
	LONG(0)

	. = ALIGN (4);
	__JCR_LIST__ = .;
	* (.jcr)
	. = ALIGN (4);
	__JCR_END__ = .;
	LONG(0)

	. = ALIGN (4);
	__EH_FRAME_BEGIN__ = .;
	* (.eh_frame)
	. = ALIGN (4);
	__FRAME_END__ = .;
	LONG(0)

	. = ALIGN (4);
	* (.gcc_except_table)
	_erodata_app = .;

  }

  .data ALIGN (0x1000) : {
	_sdata_app = .;
    	* (.data)
	_edata_app = .;
  }
  
  .bss : {
	_sbss_app = .;
	* (COMMON)
	* (.bss)
	_ebss_app = .;
  }
 
  _eapp = .;

 . = ALIGN(4K);

  _sfreemem = .;

  . = 0xa0000000;
  .ARM.exidx   : { 
  	__exidx_start = ABSOLUTE(.);
	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
  	__exidx_end = ABSOLUTE(.);
   }

  /DISCARD/ :
  {
	*(.note)
	*(.comment*)
  }
}
