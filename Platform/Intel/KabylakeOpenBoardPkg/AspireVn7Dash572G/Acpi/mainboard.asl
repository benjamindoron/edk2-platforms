/* SPDX-License-Identifier: BSD-2-Clause-Patent */

#include "thermal.asl"

// TODO: Need hooks from BoardAcpiDxe

Scope (_SB)
{
	Method (MPTS, 1, NotSerialized)  // _PTS: Prepare To Sleep
	{
		^PCI0.LPCB.EC0.ECPS (Arg0)
	}

	Method (MWAK, 1, Serialized)  // _WAK: Wake
	{
		^PCI0.LPCB.EC0.ECWK (Arg0)

		If ((Arg0 == 3) || (Arg0 == 4))
		{
			Notify (LID0, 0x80) // Status Change
		}
	}

	Method (MS0X, 1, Serialized)	// S0ix hook. Porting "GUAM" method - "Global User Absent Mode"
	{
		If (Arg0 == 0)
		{
			/* Exit "Connected Standby" */
#if 1	// EC Notification
			^PCI0.LPCB.EC0.EOSS = 0
#endif
			/* TODO: P-state capping, PL setting? */
		}
		ElseIf (Arg0 == 1)
		{
			/* Enter "Connected Standby" */
#if 1	// EC Notification
			^PCI0.LPCB.EC0.ECSS = 0x08
#endif
			/* TODO: P-state capping, PL setting? */
		}
	}

	Device (LID0)
	{
		Name (_HID, EisaId ("PNP0C0D") /* Lid Device */)  // _HID: Hardware ID
		Method (_LID, 0, NotSerialized)  // _LID: Lid Status
		{
			Return (^^PCI0.LPCB.EC0.ELID)
		}

		Method (_PSW, 1, NotSerialized)  // _PSW: Power State Wake
		{
			^^PCI0.LPCB.EC0.EIDW = Arg0
		}

		Name (_PRW, Package () { 0x0A, 3 })  // _PRW: Power Resources for Wake
	}

	// FIXME: Collides with BoardAcpiDxe, but GPI in _PRW is board-specific.
	// Perhaps _INI method fills variable inside BoardAcpiDxe?
	Device (SLPB)
	{
		Name (_HID, EisaId ("PNP0C0E") /* Sleep Button Device */)  // _HID: Hardware ID
		Name (_PRW, Package () { 0x0A, 3 })  // _PRW: Power Resources for Wake
	}
}

Scope (_GPE)
{
	/* TODO - Remaining Level-Triggered GPEs: PCH GPE, PCIe PME, TBT, DTS, GFX SCI and tier-2 (RTD3) */
	Method (_L0A, 0, NotSerialized)
	{
		Notify (\_SB.SLPB, 0x02) // Device Wake
	}
}
