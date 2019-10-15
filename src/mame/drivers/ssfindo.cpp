// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/************************************************************************

 'RISC PC' hardware

 See See Find Out [Icarus 1999]
 Pang Pang Car [Icarus 1999]
 Tetris Fighters [Sego Entertainment 2001]

 driver by
  Tomasz Slanina  analog[at]op.pl

TODO:
 - Correct IOMD type (CL-PS7500 -> ARM7500FE?)
 - 24c01 eeprom (IOLINES)
 - timing
 - unknown reads/writes
 - sound

*************************************************************************************************************

See See Find Out
Icarus, 1999

PCB Layout
----------

ARIAMEDIA M/B Rev 1.0
|--------------------------------------|
|          B.U29          C.U12        |
|      A.U28     *     *     D.U13     |
|                               1008S-1|
|                    |------|          |
|    KM416C1204      |QS1000|   E.U14  |
|J       KM416C1204  |------| |--------|
|A                     24MHz  |DU2  DU3|
|M     |---------|            |        |
|M     |CL-PS7500|  |------|  |DU5  DU6|
|A     |         |  |QL2003|  |--------|
|      |         |  |      | 24C01A.U36|
|      |         |  |------|           |
|      |---------|          14.31818MHz|
|            54MHz  |------|           |
|                   |PRIME |           |
|                   |------|   DIPSW(8)|
|--------------------------------------|

Notes:

      Chips:
         QS1000: QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
           PRIME: LGS Prime 3C 9930R, clock input of 14.31818MHz (QFP100)
         QL2003: QuickLogic QL2003-XPL84C 0003BA FPGA (PLCC84)
      CL-PS7500: Cirrus Logic CL-PS7500FE-56QC-A 84903-951BD ARM 9843J
                 clock input of 54.000MHz, ARM710C; ARM7-core CPU (QFP240)
     KM416C1204: Samsung KM416C1204CJ-5 2M x8 DRAM (SOJ42)
              *: Unpopulated DIP32 socket

      ROMs:
           24C010A: Amtel 24C01A (128bytes x8Bit Serial EEPROM, DIP8)
        A, B, C, D: TMS27C040 512K x8 EPROM (DIP32)
                 E: ST M27C512 64K x8 EPROM (DIP28)
           1008S-1: HWASS 1008S-1 Wavetable Audio Samples chip, 1M x8 MaskROM (SOP32)
         DU2, DU3,: Samsung KM29W32000AT 32MBit NAND Flash 3.3V Serial EEPROM (TSOP44)
         DU5, DU6   These ROMs are mounted on a small plug-in daughterboard. There are additional
                    mounting pads for 4 more of these ROMs but they're not populated.


**************************************************************************************************************

Pong Pong Car
Icarus, 1999

This game runs on hardware that is similar to that used on 'See See Find Out'
The game is a rip-off of RallyX

PCB Layout
----------

|--------------------------------------|
|    KM416C1204                        |
|    KM416C1204    U24  U25  U26*  U27*|
|  DA1311A                             |
|  DA1311A        |---------|54MHz     |
|  4558           |CL-PS7500| LED      |
|J                |         | |--------|
|A                |         | |DU2  DU3|
|M                |         | |        |
|M EL2386       @ |---------| |DU5  DU6|
|A       &                    |--------|
|                                      |
|  7660             |------|           |
|                   |PRIME |14.31818MHz|
|                   |------|           |
| NASN9289  XILINX                     |
| QS1000    XCS10    MAX232    DIPSW(8)|
|  24MHz     #   7705                  |
|--------------------|DB9|-------------|
                     |---|
Notes:
      Chips:
         QS1000: QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
          PRIME: LGS Prime 3C 9849R, clock input of 14.31818MHz (QFP100)
   XILINX XCS10: Xilinx Spartan XCS10 FPGA (QFP144)
      CL-PS7500: Cirrus Logic CL-PS7500FE-56QC-A 84877-951BD ARM 9843J
                 clock input of 54.000MHz, ARM710C; ARM7-core CPU (QFP240)
     KM416C1204: Samsung KM416C1204CJ-5 2M x8 DRAM (SOJ42)
           7705: Reset/Watchdog IC (SOIC8)
           7660: DC-DC Voltage Convertor (SOIC8)
         EL2386: Elantec Semiconductor 250MHz Triple Current Feedback Op Amp with Disable (SOIC16)
              *: Unpopulated DIP32 sockets
              &: Unpopulated location for QFP100 IC
              #: Unpopulated location for SOJ42 RAM
              @: Unpopulated location for OSC1

      ROMs:
          U24, U25: AMD 29F040B 512k x8 FlashROM (DIP32)
          NASN9289: Re-badged SOP32 ROM. Should be compatible with existing QS100x Wavetable Audio Sample ROMs,
                    Dumped as 1M x8 SOP32 MaskROM
         DU2, DU3,: Samsung KM29N32000TS 32MBit NAND Flash 3.3V Serial EEPROM (TSOP44)
         DU5, DU6   These ROMs are mounted on a small plug-in daughterboard. There are additional
                    mounting pads for 4 more of these ROMs but they're not populated.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/i2cmem.h"
#include "machine/acorn_vidc.h"
#include "machine/arm_iomd.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

enum
{
	IOCR=0,
	KBDDAT,
	KBDCR,
	IOLINES,
	IRQSTA,
	IRQRQA,
	IRQMSKA,
	SUSMODE,
	IRQSTB,
	IRQRQB,
	IRQMSKB,
	STOPMODE,
	FIQST,
	FIQRQ,
	FIQMSK,
	CLKCTL,
	T0low,
	T0high,
	T0GO,
	T0LAT,
	T1low,
	T1high,
	T1GO,
	T1LAT,
	IRQSTC,
	IRQRQC,
	IRQMSKC,
	VIDMUX,
	IRQSTD,
	IRQRQD,
	IRQMSKD,

	ROMCR0=32,
	ROMCR1,

	REFCR=35,

	ID0=37,
	ID1,
	VERSION,

	MSEDAT=42,
	MSECR,

	IOTCR=49,
	ECTCR,
	ASTCR,
	DRAMCR,
	SELREF,

	ATODICR=56,
	ATODSR,
	ATODCC,
	ATODCNT1,
	ATODCNT2,
	ATODCNT3,
	ATODCNT4,

	SDCURA=96,
	SDENDA,
	SDCURB,
	SDENDB,
	SDCR,
	SDST,

	CURSCUR=112,
	CURSINIT,
	VIDCURB,

	VIDCURA=116,
	VIDEND,
	VIDSTART,
	VIDINITA,
	VIDCR,

	VIDINITB=122,

	DMAST=124,
	DMARQ,
	DMASK,
	MAXIO=128
};


class ssfindo_state : public driver_device
{
public:
	ssfindo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vidc(*this, "vidc"),
		m_iomd(*this, "iomd"),
		m_i2cmem(*this, "i2cmem"),
		m_flashrom(*this, "flash")
		{ }

	void ssfindo(machine_config &config);
	void ppcar(machine_config &config);

	void init_ssfindo();
	void init_ppcar();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<arm_vidc20_device> m_vidc;
	required_device<arm_iomd_device> m_iomd;
	optional_device<i2cmem_device> m_i2cmem;
	required_region_ptr<uint16_t> m_flashrom;
	
	void init_common();
	virtual void machine_start() override;
	virtual void machine_reset() override;

	typedef void (ssfindo_state::*speedup_func)();
	speedup_func m_speedup;

	uint32_t m_flashType;

	bool m_i2cmem_clock;
private:

	// ssfindo and ppcar
	uint32_t m_flashAdr;
	uint32_t m_flashOffset;
	uint32_t m_adrLatch;
	uint32_t m_flashN;

	// ssfindo and ppcar
	DECLARE_READ32_MEMBER(io_r);
	DECLARE_WRITE32_MEMBER(io_w);

	// ssfindo
	DECLARE_WRITE32_MEMBER(debug_w);
	DECLARE_READ32_MEMBER(ff4_r);
	DECLARE_READ32_MEMBER(SIMPLEIO_r);

	// ppcar
	DECLARE_READ32_MEMBER(randomized_r);

	DECLARE_WRITE_LINE_MEMBER(sound_drq);

	void ssfindo_speedups();
	void ppcar_speedups();

	void ppcar_map(address_map &map);
	void ssfindo_map(address_map &map);
	
	uint32_t m_vidc_sndcur, m_vidc_sndend;
	bool m_audio_dma_on;
	uint8_t m_vidc_sndcur_buffer, m_vidc_snd_overrun, m_vidc_snd_int;
	bool m_vidc_sndbuffer_ok[2];
	inline void sound_swap_buffer();
	
	DECLARE_READ8_MEMBER(iolines_r);
	DECLARE_WRITE8_MEMBER(iolines_w);
	bool m_flash_bank_select;
};

class tetfight_state : public ssfindo_state
{
public:
	tetfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: ssfindo_state(mconfig, type, tag)
		{}

	void init_tetfight();
	void tetfight(machine_config &config);

protected:

private:
	void tetfight_map(address_map &map);

	DECLARE_READ_LINE_MEMBER(iocr_od0_r);
	DECLARE_READ_LINE_MEMBER(iocr_od1_r);
	DECLARE_WRITE_LINE_MEMBER(iocr_od0_w);
	DECLARE_WRITE_LINE_MEMBER(iocr_od1_w);
	DECLARE_READ32_MEMBER(tetfight_unk_r);
	DECLARE_WRITE32_MEMBER(tetfight_unk_w);

};

//TODO: eeprom  24c01 & 24c02
// TODO: untangle, kill hacks
READ8_MEMBER(ssfindo_state::iolines_r)
{
	if(m_flashType == 1)
		return 0;
	else
		return machine().rand();
}

WRITE8_MEMBER(ssfindo_state::iolines_w)
{
	if(data&0xc0)
		m_adrLatch=0;

	if(m_maincpu->pc() == 0xbac0 && m_flashType == 1)
		m_flashN=data&1;

	m_flash_bank_select = BIT(data, 0);
}

READ_LINE_MEMBER(tetfight_state::iocr_od1_r)
{
	// TODO: completely get rid of this speedup fn or move anywhere else
	//if (m_speedup) (this->*m_speedup)();
	// returning 0 here causes a tight loop with cyan border.
	return (m_i2cmem->read_sda() ? 1 : 0); //eeprom read
}

READ_LINE_MEMBER(tetfight_state::iocr_od0_r)
{
	// TODO: presuming same as Acorn Archimedes, where i2c clock can be readback
	return m_i2cmem_clock;
}

// TODO: correct hookup
WRITE_LINE_MEMBER(tetfight_state::iocr_od1_w)
{
	m_i2cmem->write_sda(state);
}

WRITE_LINE_MEMBER(tetfight_state::iocr_od0_w)
{
	m_i2cmem_clock = state;
	m_i2cmem->write_scl(m_i2cmem_clock);
}

#if 0
inline void ssfindo_state::sound_swap_buffer()
{
	if (m_vidc_sndcur_buffer == 1)
	{
		m_vidc_sndcur = m_PS7500_IO[SDCURB] & 0x1fffffff;
		m_vidc_sndend = m_vidc_sndcur + (m_PS7500_IO[SDENDB] + 0x10);
		m_vidc_sndbuffer_ok[1] = true;
	}
	else
	{
		m_vidc_sndcur = m_PS7500_IO[SDCURA] & 0x1fffffff;
		m_vidc_sndend = m_vidc_sndcur + (m_PS7500_IO[SDENDA] + 0x10);
		m_vidc_sndbuffer_ok[0] = true;
	}

	// TODO: actual condition for int
	m_vidc_snd_overrun = false;
	m_vidc_snd_int = false;
}
#endif

WRITE_LINE_MEMBER(ssfindo_state::sound_drq)
{
	if (!state)
		return;
	
	#if 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	
	if (m_vidc->get_dac_mode() == true)
	{
		for (int ch=0;ch<2;ch++)
			m_vidc->write_dac32(ch, (space.read_word(m_vidc_sndcur + ch*2)));
		
		m_vidc_sndcur += 4;
		
		if (m_vidc_sndcur >= m_vidc_sndend)
		{
			// TODO: interrupt bit

			m_vidc->update_sound_mode(m_audio_dma_on);
			if (m_audio_dma_on)
			{
				m_vidc_sndbuffer_ok[m_vidc_sndcur_buffer] = false;
				m_vidc_sndcur_buffer ^= 1;
				m_vidc_snd_overrun = (m_vidc_sndbuffer_ok[m_vidc_sndcur_buffer] == false);				
				sound_swap_buffer();
			}
			else
			{
				// ...
			}
		}
	}
	else
	{
		// ...
	}
	#endif
}

#if 0
void ssfindo_state::PS7500_reset()
{
	m_PS7500_IO[IOCR]  = 0x3f;
	m_PS7500_IO[VIDCR] = 0;
	m_vidc_snd_overrun = 1;
	m_vidc_snd_int = 1;
	m_vidc_sndcur_buffer = 0;
	m_vidc_sndbuffer_ok[0] = false;
	m_vidc_sndbuffer_ok[1] = false;

	m_PS7500timer0->adjust( attotime::never);
	m_PS7500timer1->adjust( attotime::never);
}
#endif

void ssfindo_state::ssfindo_speedups()
{
	if (m_maincpu->pc()==0x2d6c8) // ssfindo
		m_maincpu->spin_until_time(attotime::from_usec(20));
	else if (m_maincpu->pc()==0x2d6bc) // ssfindo
		m_maincpu->spin_until_time(attotime::from_usec(20));
}

void ssfindo_state::ppcar_speedups()
{
	if (m_maincpu->pc()==0x000bc8) // ppcar
		m_maincpu->spin_until_time(attotime::from_usec(20));
	else if (m_maincpu->pc()==0x000bbc) // ppcar
		m_maincpu->spin_until_time(attotime::from_usec(20));
}

#if 0
READ32_MEMBER(ssfindo_state::PS7500_IO_r)
{
	switch(offset)
	{
		case MSECR:
			return machine().rand();

		case IOLINES: //TODO: eeprom  24c01
#if 0
		osd_printf_debug("IOLINESR %i @%x\n", offset, m_maincpu->pc());
#endif

		if(m_flashType == 1)
			return 0;
		else
			return machine().rand();

		case IRQSTA:
			return (m_PS7500_IO[offset] & (~2)) | 0x80;

		case IRQRQA:
			return (m_PS7500_IO[IRQSTA] & m_PS7500_IO[IRQMSKA]) | 0x80;

		case IOCR: //TODO: nINT1, OD[n] p.81
			if (m_speedup) (this->*m_speedup)();

			if( m_iocr_hack)
			{
				return (m_vidc->flyback_r()<<7) | 0x34 | (m_i2cmem->read_sda() ? 0x03 : 0x00); //eeprom read
			}

			return (m_vidc->flyback_r()<<7) | 0x37;

		case VIDCR:
			return (m_PS7500_IO[offset] | 0x50) & 0xfffffff0;

		case T1low:
		case T0low:
		case T1high:
		case T0high:
		case IRQMSKA:
		case VIDEND:
		case VIDSTART:
		case VIDINITA: //TODO: bits 29 ("equal") and 30 (last bit)  p.105
		case SDCR:
		case SDCURA:
		case SDCURB:
		case SDENDA:
		case SDENDB:
			return m_PS7500_IO[offset];
		
		case SDST:
			return (m_vidc_snd_overrun<<2) | (m_vidc_snd_int<<1) | m_vidc_sndcur_buffer;
	}
	return machine().rand();//m_PS7500_IO[offset];
}

WRITE32_MEMBER(ssfindo_state::PS7500_IO_w)
{
	uint32_t temp=m_PS7500_IO[offset];

	COMBINE_DATA(&temp);

	switch(offset)
	{
		case IOLINES: //TODO: eeprom  24c01
			m_PS7500_IO[offset]=data;
				if(data&0xc0)
					m_adrLatch=0;

			if(m_maincpu->pc() == 0xbac0 && m_flashType == 1)
			{
				m_flashN=data&1;
			}

#if 0
				logerror("IOLINESW %i = %x  @%x\n",offset,data,m_maincpu->pc());
#endif
			break;

		case IRQRQA:
			m_PS7500_IO[IRQSTA]&=~temp;
			break;

		case IRQMSKA:
			m_PS7500_IO[IRQMSKA]=(temp&(~2))|0x80;
			break;

		case T1GO:
			PS7500_startTimer1();
			break;

		case T0GO:
			PS7500_startTimer0();
			break;

		case VIDEND:
		case VIDSTART:
			COMBINE_DATA(&m_PS7500_IO[offset]);
			m_PS7500_IO[offset]&=0xfffffff0; // qword align
			break;

		case IOCR:
			//popmessage("IOLINESW %i = %x  @%x\n",offset,data,m_maincpu->pc());
			COMBINE_DATA(&m_PS7500_IO[offset]);
			// TODO: correct hook-up
			m_i2cmem->write_scl((data & 0x01) ? 1 : 0);
			m_i2cmem->write_sda((data & 0x02) ? 1 : 0);
			break;

		case SDCR:
			COMBINE_DATA(&m_PS7500_IO[offset]);
			m_audio_dma_on = BIT(m_PS7500_IO[SDCR], 5);

			m_vidc->update_sound_mode(m_audio_dma_on);
			if (m_audio_dma_on)
			{
				m_vidc_sndcur_buffer = 0;
				sound_swap_buffer();
			}
			else
			{
				// ...
			}
			break;

		case SDCURA:
		case SDENDA:
			COMBINE_DATA(&m_PS7500_IO[offset]);
			break;

		case SDCURB:
		case SDENDB:
			COMBINE_DATA(&m_PS7500_IO[offset]);
			break;

		case REFCR:
		case DRAMCR:
		case ROMCR0:
		case VIDMUX:
		case CLKCTL:
		case T1low:
		case T0low:
		case T1high:
		case T0high:
		case VIDCR:
		case VIDINITA: //TODO: bit 30 (last bit) p.105ù
			COMBINE_DATA(&m_PS7500_IO[offset]);
			break;
	}
}
#endif

READ32_MEMBER(ssfindo_state::io_r)
{
	int adr=m_flashAdr*0x200+(m_flashOffset);


	switch(m_flashType)
	{
		case 0:
			//bit 0 of IOLINES  = flash select ( 5/6 or 3/2 )
			if (m_flash_bank_select) 
				adr+=0x400000;
			break;

		case 1:
			adr+=0x400000*m_flashN;
			break;
	}

	if(adr<0x400000*2)
	{
		m_flashOffset++;
		return m_flashrom[adr];
	}
	return 0;
}

WRITE32_MEMBER(ssfindo_state::io_w)
{
	uint32_t temp = 0;
	COMBINE_DATA(&temp);

#if 0
	logerror("[io_w] = %x @%x [latch=%x]\n",data,m_maincpu->pc(),m_adrLatch);
#endif

	if(m_adrLatch==1)
		m_flashAdr=(temp>>16)&0xff;
	if(m_adrLatch==2)
	{
		m_flashAdr|=(temp>>16)&0xff00;
		m_flashOffset=0;
	}
	m_adrLatch=(m_adrLatch+1)%3;
}

WRITE32_MEMBER(ssfindo_state::debug_w)
{
#if 0
	osd_printf_debug("%c",data&0xff); //debug texts - malloc (ie "64 KBytes allocated, elapsed : 378 KBytes, free : 2231 KBytes")
#endif
}

READ32_MEMBER(ssfindo_state::ff4_r)
{
	return machine().rand()&0x20;
}

READ32_MEMBER(ssfindo_state::SIMPLEIO_r)
{
	return machine().rand()&1;
}

READ32_MEMBER(ssfindo_state::randomized_r)
{
	return machine().rand();
}

void ssfindo_state::ssfindo_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom();
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm_iomd_device::map));
	map(0x03012e60, 0x03012e67).noprw();
	map(0x03012fe0, 0x03012fe3).w(FUNC(ssfindo_state::debug_w));
	map(0x03012ff0, 0x03012ff3).noprw();
	map(0x03012ff4, 0x03012ff7).nopw().r(FUNC(ssfindo_state::ff4_r)); //status flag ?
	map(0x03012ff8, 0x03012fff).noprw();
	map(0x03240000, 0x03240003).portr("IN0").nopw();
	map(0x03241000, 0x03241003).portr("IN1").nopw();
	map(0x03242000, 0x03242003).r(FUNC(ssfindo_state::io_r)).w(FUNC(ssfindo_state::io_w));
	map(0x03243000, 0x03243003).portr("DSW").nopw();
	map(0x0324f000, 0x0324f003).r(FUNC(ssfindo_state::SIMPLEIO_r));
	map(0x03245000, 0x03245003).nopw(); /* sound ? */
	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	map(0x10000000, 0x11ffffff).ram();
}

void ssfindo_state::ppcar_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom();
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm_iomd_device::map));
	map(0x03012b00, 0x03012bff).r(FUNC(ssfindo_state::randomized_r)).nopw();
	map(0x03012e60, 0x03012e67).nopw();
	map(0x03012ff8, 0x03012ffb).portr("IN0").nopw();
	map(0x032c0000, 0x032c0003).portr("IN1").nopw();
	map(0x03340000, 0x03340007).nopw();
	map(0x03341000, 0x0334101f).nopw();
	map(0x033c0000, 0x033c0003).r(FUNC(ssfindo_state::io_r)).w(FUNC(ssfindo_state::io_w));
	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	map(0x08000000, 0x08ffffff).ram();
	map(0x10000000, 0x10ffffff).ram();
}

READ32_MEMBER(tetfight_state::tetfight_unk_r)
{
	//sound status ?
	return machine().rand();
}

WRITE32_MEMBER(tetfight_state::tetfight_unk_w)
{
	//sound latch ?
}

void tetfight_state::tetfight_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm_iomd_device::map));
	map(0x03240000, 0x03240003).portr("IN0");
	map(0x03240004, 0x03240007).portr("IN1");
	map(0x03240008, 0x0324000b).portr("DSW2");
	map(0x03240020, 0x03240023).rw(FUNC(tetfight_state::tetfight_unk_r), FUNC(tetfight_state::tetfight_unk_w));
	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	map(0x10000000, 0x14ffffff).ram();
}

void ssfindo_state::machine_start()
{
	// TODO: convert to exact flash device
	save_item(NAME(m_flashAdr));
	save_item(NAME(m_flashOffset));
	save_item(NAME(m_adrLatch));
	save_item(NAME(m_flashN));
	save_item(NAME(m_i2cmem_clock));
	save_item(NAME(m_flash_bank_select));
}

void ssfindo_state::machine_reset()
{
//	PS7500_reset();
}

static INPUT_PORTS_START( ssfindo )
	// TODO: incomplete structs
	PORT_START("IN0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )               // IPT_START2 ??
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1  )

	PORT_START("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ppcar )
	PORT_START("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1   )

	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT   ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1   )
INPUT_PORTS_END

static INPUT_PORTS_START( tetfight )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2    ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3    ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2    ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3    ) PORT_PLAYER(2)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Initialize" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x010, "DSW 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x040, "DSW 6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Number of rounds" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x00, "1" )
INPUT_PORTS_END


void ssfindo_state::ssfindo(machine_config &config)
{
	/* basic machine hardware */
	ARM7(config, m_maincpu, 54000000); // guess...
	m_maincpu->set_addrmap(AS_PROGRAM, &ssfindo_state::ssfindo_map);
//	m_maincpu->set_vblank_int("screen", FUNC(ssfindo_state::interrupt));

	I2C_24C01(config, m_i2cmem);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	ARM_VIDC20(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_iomd, FUNC(arm_iomd_device::vblank_irq));
	m_vidc->sound_drq().set(FUNC(ssfindo_state::sound_drq));
	
	ARM_IOMD(config, m_iomd, 0);
	m_iomd->set_host_cpu_tag(m_maincpu);
	m_iomd->set_vidc_tag(m_vidc);
	m_iomd->iolines_read().set(FUNC(ssfindo_state::iolines_r));
	m_iomd->iolines_write().set(FUNC(ssfindo_state::iolines_w));
}

void ssfindo_state::ppcar(machine_config &config)
{
	ssfindo(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &ssfindo_state::ppcar_map);

	config.device_remove("i2cmem");
}

void tetfight_state::tetfight(machine_config &config)
{
	ppcar(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &tetfight_state::tetfight_map);

	I2C_24C02(config, m_i2cmem);
	
	m_iomd->iocr_read_od<0>().set(FUNC(tetfight_state::iocr_od0_r));
	m_iomd->iocr_read_od<1>().set(FUNC(tetfight_state::iocr_od1_r));
	m_iomd->iocr_write_od<0>().set(FUNC(tetfight_state::iocr_od0_w));
	m_iomd->iocr_write_od<1>().set(FUNC(tetfight_state::iocr_od1_w));
}

ROM_START( ssfindo )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD16_BYTE( "a.u28",   0x000000, 0x80000, CRC(c93edbd3) SHA1(9c703cfef49b59ccd5d68bab9bd59344bd18d67e) )
	ROM_LOAD16_BYTE( "b.u29",   0x000001, 0x80000, CRC(39ecb9e4) SHA1(9ebd3962d8014b97c68c364729248ed22f9298a4) )

	ROM_REGION16_LE(0x1000000, "flash", 0 ) /* flash roms */
	ROM_LOAD16_BYTE( "du5",     0x000000, 0x400000, CRC(b32bd453) SHA1(6d5694bfcc67102256f857932b83b38f62ca2010) )
	ROM_LOAD16_BYTE( "du6",     0x000001, 0x400000, CRC(00559591) SHA1(543aefddc02f6a521d3bd5e6e3d8e42127ff9baa) )

	ROM_LOAD16_BYTE( "du3",     0x800000, 0x400000, CRC(d1e8afb2) SHA1(598dfcbba14435a1d0571dcefe0ec62fec657fca) )
	ROM_LOAD16_BYTE( "du2",     0x800001, 0x400000, CRC(56998515) SHA1(9b71a44f56a545ff0c1170775c839d21bd01f545) )

	ROM_REGION(0x80, "i2cmem", 0 ) /* eeprom */
	ROM_LOAD( "24c01a.u36",     0x00, 0x80, CRC(b4f4849b) SHA1(f8f17dc94b2a305048693cfb78d14be57310ce56) )

	ROM_REGION(0x10000, "user4", 0 ) /* qdsp code */
	ROM_LOAD( "e.u14",      0x000000, 0x10000, CRC(49976f7b) SHA1(eba5b97b81736f3c184ae0c19f1b10c5ae250d51) )

	ROM_REGION(0x100000, "user5", 0 ) /* HWASS 1008S-1  qdsp samples */
	ROM_LOAD( "1008s-1.u16",    0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )

	ROM_REGION(0x100000, "user6", 0 ) /* samples - same internal structure as qdsp samples  */
	ROM_LOAD( "c.u12",      0x000000, 0x80000, CRC(d24b5e56) SHA1(d89983cf4b0a6e0e4137f3799bdbcfd72c7bebe4) )
	ROM_LOAD( "d.u11",      0x080000, 0x80000, CRC(c0fdd82a) SHA1(a633045e0f5c144b4e24e04fb9446522fdb222f4) )
ROM_END

ROM_START( ppcar )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD16_BYTE( "fk0.u24", 0x000000, 0x80000, CRC(1940a483) SHA1(9456361fd25bf037b53bd2d04764a33b299d96dd) )
	ROM_LOAD16_BYTE( "fk1.u25", 0x000001, 0x80000, CRC(75ad8679) SHA1(392288e56350e3cc49aaca82edf26f2a9e346f21) )

	ROM_REGION16_LE(0x1000000, "flash", 0 ) /* flash roms */
	ROM_LOAD16_BYTE( "du5",     0x000000, 0x400000, CRC(d4b7374a) SHA1(54c93a4235f495ba3794aea511b19db821a8acb1) )
	ROM_LOAD16_BYTE( "du6",     0x000001, 0x400000, CRC(e95a3a62) SHA1(2b1c889d208a749e3d7e4c75588c9c1f979e88d9) )

	ROM_LOAD16_BYTE( "du3",     0x800000, 0x400000, CRC(73882474) SHA1(191b64e662542b5322160c99af8e00079420d473) )
	ROM_LOAD16_BYTE( "du2",     0x800001, 0x400000, CRC(9250124a) SHA1(650f4b89c92fe4fb63fc89d4e08c4c4c611bebbc) )

	ROM_REGION(0x10000, "user4", ROMREGION_ERASE00 ) /* qdsp code */
	/* none */

	ROM_REGION(0x100000, "user5", 0 ) /* HWASS 1008S-1  qdsp samples */
	ROM_LOAD( "nasn9289.u9",    0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )

	ROM_REGION(0x100000, "user6", ROMREGION_ERASE00 ) /* samples - same internal structure as qdsp samples  */
	/* none */
ROM_END

ROM_START( tetfight )
	ROM_REGION(0x200000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "u42",        0x000000, 0x200000, CRC(9101c4d2) SHA1(39da953de734e687ebbf976c821bf1017830f36c) )

	ROM_REGION16_LE(0x1000000, "flash", ROMREGION_ERASEFF ) /* flash roms */
	/* nothing? */

	ROM_REGION(0x100, "i2cmem", 0 ) /* 24c02 eeprom */
	ROM_LOAD( "u1",     0x00, 0x100, CRC(dd207b40) SHA1(6689d9dfa980bdfbd4e4e6cef7973e22ebbfe22e) )

	ROM_REGION(0x10000, "user4", 0 ) /* qdsp code */
	ROM_LOAD( "u12",        0x000000, 0x10000, CRC(49976f7b) SHA1(eba5b97b81736f3c184ae0c19f1b10c5ae250d51) ) // 27c512 = e.u14 on ssfindo

	ROM_REGION(0x100000, "user5", ROMREGION_ERASE00 )/*  qdsp samples */
	// probably the same, but wasn't dumped
	//ROM_LOAD( "1008s-1.u16",  0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )

	ROM_REGION(0x100000, "user6", 0 ) /* samples - same internal structure as qdsp samples  */
	ROM_LOAD( "u11",        0x000000, 0x80000, CRC(073050f6) SHA1(07f362f3ba468bde2341a99e6b26931d11459a92) ) // 27c040
	ROM_LOAD( "u15",        0x080000, 0x80000, CRC(477f8089) SHA1(8084facb254d60da7983d628d5945d27b9494e65) ) // 27c040
ROM_END

void ssfindo_state::init_common()
{
	m_speedup = nullptr;
	//m_PS7500timer0 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ssfindo_state::PS7500_Timer0_callback),this));
	//m_PS7500timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ssfindo_state::PS7500_Timer1_callback),this));

	//save_item(NAME(m_PS7500_IO));
	//save_item(NAME(m_PS7500_FIFO));
}

void ssfindo_state::init_ssfindo()
{
	init_common();
	m_flashType = 0;
	m_speedup = &ssfindo_state::ssfindo_speedups;
}

void ssfindo_state::init_ppcar()
{
	init_ssfindo();
	m_flashType = 1;
	m_speedup = &ssfindo_state::ppcar_speedups;
}

void tetfight_state::init_tetfight()
{
	init_common();
	m_flashType = 0;
}

GAME( 1999, ssfindo, 0,        ssfindo,  ssfindo,  ssfindo_state, init_ssfindo,  ROT0, "Icarus", "See See Find Out", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, ppcar,   0,        ppcar,    ppcar,    ssfindo_state, init_ppcar,    ROT0, "Icarus", "Pang Pang Car",    MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, tetfight,0,        tetfight, tetfight, tetfight_state, init_tetfight, ROT0, "Sego",   "Tetris Fighters",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
