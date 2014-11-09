/*

Space Bugger
Game-A-Tron, 1981

TS 20060821:
- there's really huge difference between both sets (code and data). weird
- wrong colors, it looks like there's direct color mapping - bits from attribute ram connected to r/g/b outputs without
    use of color prom or color table
- interrupt is generated by 8156 - very old combination chip containing 256*8 RAM (used as stack area),
  a timer and 8+8+6 ports. After game start all 8156 ports are set as inputs, timer is working in mode 3 - automatic reload.
  TC register is set to 0x7f. Timer input frequency is unknown. Output should be close to 1440 Hz.


PCB Layout
----------

CPU
2.81
GAT
  |------------------------------------------------------|
  |           74LS393                                    |
  |                         74LS00    TIP41A          CN1|
  |                                                      |
|-|           74LS32        74LS00    74LS161            |
|                                                        |
|                                     74LS10          CN2|
|    74LS139  74LS240                                    |
|                     |-----------|                      |
|    74LS240  74LS244 |  TMP8085  |   74LS04             |
|                     |-----------|6MHz                  |
|    74LS244  74LS373  74LS244  DSW(7)    74LS04         |
|                     |-----------|                      |
|    uPB8216  uPB8216 |  TMP8156  |                      |
|                     |-----------|                      |
|    2114  2114   74LS244  U32                           |
|                                                        |
|    2114  2114   74LS32   U33   U20                     |
|-|                                                      |
  |  2114  2114   74LS138  U34   U21                     |
  |                                                      |
  |  2114  2114   74LS138  U35   U22                     |
  |------------------------------------------------------|
Notes:
      All IC's shown.
      uPB8216 - NEC uPB8216 4 Bit Bi-directional Bus Driver
      2114    - Hitachi HM572114P-4 1k x4 SRAM
      TMP8156 - RIOT, 256 bytes Static RAM with I/O Ports and Timer
      TMP8085 - 8085 CPU, clock 3MHz [6/2]
      CN1/2   - 10 pin connectors
      U20-U22 - Hitachi HM462716G 2k x8 EPROMs
      U32-U35 /

      Note, this PCB plugs into a motherboard containing connectors. There is
      also a sound and graphics board that plugs into the motherboard as well.
      However those PCBs are not available and are therefore not documented.


 Set 2:
--------

Space Bugger
Game-A-Tron 1981


                       U20 U21 U22
                   U32 U33 U34 U35
             6MHz

          8085   8156
                       2114 2114 2114 2114
                       2114 2114 2114 2114

Main

-------


            GFX

            2114 2114 2114 2114

12.440MHz

Graphics PCB

-------



                         76489  76489


Sound PCB

-------

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "sound/sn76496.h"
#include "includes/sbugger.h"


/* memory maps */

static ADDRESS_MAP_START( sbugger_map, AS_PROGRAM, 8, sbugger_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0xc800, 0xcbff) AM_RAM_WRITE(sbugger_videoram_attr_w) AM_SHARE("videoram_attr")
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(sbugger_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xe0ff) AM_DEVREADWRITE("i8156", i8155_device, memory_r, memory_w) /* sp is set to e0ff */
	AM_RANGE(0xf400, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbugger_io_map, AS_IO, 8, sbugger_state )
	AM_RANGE(0xe0, 0xe7) AM_DEVREADWRITE("i8156", i8155_device, io_r, io_w)
	AM_RANGE(0xe8, 0xe8) AM_DEVWRITE("sn76489.1", sn76489_device, write)
	AM_RANGE(0xe9, 0xe9) AM_DEVWRITE("sn76489.2", sn76489_device, write)
ADDRESS_MAP_END


/* gfx decode */

static const gfx_layout char16layout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8
	},
	16*8
};

static GFXDECODE_START( sbugger )
	GFXDECODE_ENTRY( "gfx1", 0, char16layout,   0, 256  )
GFXDECODE_END


/* input ports */

static INPUT_PORTS_START( sbugger )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_DIPNAME( 0x02, 0x02, "PA 1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x80, "PA 7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "PB 0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "PB 1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "PB 2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "PB 3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "PB 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "PB 5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "PB 6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) ) //enemy speed
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) //bonus stage related (code at $f94 & $df8)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* machine driver */

WRITE_LINE_MEMBER(sbugger_state::sbugger_interrupt)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state ? CLEAR_LINE : ASSERT_LINE );
}

static MACHINE_CONFIG_START( sbugger, sbugger_state )

	MCFG_CPU_ADD("maincpu", I8085A, 6000000)        /* 3.00 MHz??? */
	MCFG_CPU_PROGRAM_MAP(sbugger_map)
	MCFG_CPU_IO_MAP(sbugger_io_map)

	MCFG_DEVICE_ADD("i8156", I8156, 200000)     /* freq is an approximation */
	MCFG_I8155_IN_PORTA_CB(IOPORT("INPUTS"))
	MCFG_I8155_IN_PORTB_CB(IOPORT("DSW1"))
	MCFG_I8155_IN_PORTC_CB(IOPORT("DSW2"))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(sbugger_state, sbugger_interrupt))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sbugger)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sbugger_state, screen_update_sbugger)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(sbugger_state, sbugger)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn76489.1", SN76489, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn76489.2", SN76489, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/* rom loading */

ROM_START( sbugger )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 8085 Code */

	ROM_LOAD( "spbugger.u35", 0x0000, 0x0800, CRC(7c2000a1) SHA1(01a60745ea8e9a70de37d1a785fad1d17eafc812) ) // seems to map at 0
	ROM_LOAD( "spbugger.u22", 0x0800, 0x0800, BAD_DUMP CRC(66e00c53) SHA1(49ca567a98978308306cdb8455c61c022668693b) ) // FIXED BITS (xxxx1111)  it jumps here .... bad rom?
	ROM_LOAD( "spbugger.u34", 0x1000, 0x0800, CRC(db357dde) SHA1(363392b971f48e9d99f4167aa17f0c885b0865ee) ) // seems to map at 1000
	ROM_LOAD( "spbugger.u21", 0x1800, 0x0800, CRC(618a5b2a) SHA1(aa7a40b1944f09c396f675d7dd3a8c3c35bf01f1) ) // seems to map at 1800
	ROM_LOAD( "spbugger.u20", 0x2000, 0x0800, CRC(8957563c) SHA1(b33a75fcf375d2a1a766105f87dd8e4d42db3d76) ) // seems to map at 2000
	ROM_LOAD( "spbugger.u33", 0x2800, 0x0800, CRC(f6cb1399) SHA1(53cb67e29a238c5ac20c6be9423d850e004212c1) ) // seems to map at 2800
	ROM_LOAD( "spbugger.u32", 0x3000, 0x0800, CRC(f49af2b3) SHA1(1519ee4786b78546767827d3a9508e7ddb646765) ) // seems to map at 3000

	ROM_REGION( 0x1000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "spbugger.gfx", 0x0000, 0x1000, CRC(d3f345b5) SHA1(a5082ffc3043352e9b731af95770bdd62fb928bf) )
ROM_END

ROM_START( sbuggera )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 8085 Code */
	ROM_LOAD( "bug_g10.u35", 0x0000, 0x0800, CRC(60a3044d) SHA1(5b2be551a84e4a7a35637208a19b3477629f20d9) )
	ROM_LOAD( "bug_c10.u22", 0x0800, 0x0800, CRC(34a829f7) SHA1(135ec2739879c2e47f3c6d4a5196c865b5940a84) )
	ROM_LOAD( "bug_f10.u34", 0x1000, 0x0800, CRC(e2f7a51c) SHA1(ee221f6697021d14838fd6c4aff41678ce62e4ba) )
	ROM_LOAD( "bug_b10.u21", 0x1800, 0x0800, CRC(3c9a467a) SHA1(be64fddc294be38ff4f997e6666aaea15ed7d0e9) )
	ROM_LOAD( "bug_a10.u20", 0x2000, 0x0800, CRC(f6f58aba) SHA1(95bf03b8876ff36f52d93f5f5f9b38021c8b7979) )
	ROM_LOAD( "bug_e10.u33", 0x2800, 0x0800, CRC(fefd9c5e) SHA1(1b0bbf462231c32014d45ec21b105a669665d90d) )
	ROM_LOAD( "bug_d10.u32", 0x3000, 0x0800, BAD_DUMP CRC(c807742f) SHA1(cb5c44ffd6dd184c6a0722003ff6674caf865bee) ) // missing in this set

	ROM_REGION( 0x4000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "spbugger.gfx", 0x0000, 0x1000, CRC(d3f345b5) SHA1(a5082ffc3043352e9b731af95770bdd62fb928bf) )
ROM_END

GAME( 1981, sbugger,  0,        sbugger,  sbugger, driver_device,  0, ROT270, "Game-A-Tron", "Space Bugger (set 1)", GAME_NOT_WORKING | GAME_WRONG_COLORS )
GAME( 1981, sbuggera, sbugger,  sbugger,  sbugger, driver_device,  0, ROT270, "Game-A-Tron", "Space Bugger (set 2)", GAME_WRONG_COLORS )
