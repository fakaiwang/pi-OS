//
// timer.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2016  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _circle_timer_h
#define _circle_timer_h

#include "../../../pi-OS/include/circle/ptrlist.h"
#include "../../../pi-OS/include/circle/string.h"
#include "../../../Pi-OS/include/circle/interrupt.h"
#include "../../../Pi-OS/include/circle/spinlock.h"
#include "../../../Pi-OS/include/circle/sysconfig.h"

#define HZ		100			// ticks per second

#define MSEC2HZ(msec)	((msec) * HZ / 1000)

typedef void TKernelTimerHandler (unsigned hTimer, void *pParam, void *pContext);

class CTimer	/// Manages the system clock, supports kernel timers and a calibrated delay loop
{
public:
	CTimer (CInterruptSystem *pInterruptSystem);
	~CTimer (void);

	boolean Initialize (void);

	/// \brief Sets the time zone (defaults to 0, GMT)
	/// \param nMinutesDiff	Minutes diff to UTC
	/// \return Operation successful?
	boolean SetTimeZone (int nMinutesDiff);
	/// \return Minutes diff to UTC
	int GetTimeZone (void) const;

	/// \param nTime	Seconds since 1970-01-01 00:00:00
	/// \param bLocal	Is time given according to our time zone? (UTC otherwise)
	/// \return Operation successful? May fail for very small values if bLocal == FALSE
	boolean SetTime (unsigned nTime, boolean bLocal = TRUE);

	/// \return Current clock ticks of an 1 MHz counter, may wrap
	static unsigned GetClockTicks (void);
#define CLOCKHZ	1000000

	/// \return 1/HZ seconds since system boot, may wrap
	unsigned GetTicks (void) const;
	/// \return Seconds since system boot (continous)
	unsigned GetUptime (void) const;

	/// \return Seconds since system boot\n
	/// or since 1970-01-01 00:00:00 (if time was set)\n
	/// Current time according to our time zone
	unsigned GetTime (void) const;
	/// \brief Same function as GetTime()
	unsigned GetLocalTime (void) const	{ return GetTime (); }

	/// \return Current time (UTC) in seconds since 1970-01-01 00:00:00\n
	/// may be 0 if time was not set and time zone diff is > 0
	unsigned GetUniversalTime (void) const;

	/// \return "[MMM dD ]HH:MM:SS.ss" or 0 if Initialize() was not called yet,\n
	/// resulting CString object must be deleted by caller\n
	/// Current time according to our time zone
	CString *GetTimeString (void);

	/// \brief Starts a kernel timer which elapses after a given delay,\n
	/// a timer handler gets called then
	/// \param nDelay	Timer elapses after nDelay/HZ seconds from now
	/// \param pHandler	The handler to be called when the timer elapses
	/// \param pParam	First user defined parameter to hand over to the handler
	/// \param pContext	Second user defined parameter to hand over to the handler
	/// \return Timer handle (cannot be 0)
	unsigned StartKernelTimer (unsigned nDelay,
				   TKernelTimerHandler *pHandler,
				   void *pParam   = 0,
				   void *pContext = 0);
	/// \brief Cancel a running kernel timer,\n
	/// The timer will not elapse any more.
	/// \param hTimer	Timer handle
	void CancelKernelTimer (unsigned hTimer);

	/// When a CTimer object is available better use this instead of SimpleMsDelay()\n
	/// \param nMilliSeconds Delay in milliseconds (<= 2000)
	void MsDelay (unsigned nMilliSeconds)	{ SimpleMsDelay (nMilliSeconds); }
	/// When a CTimer object is available better use this instead of SimpleusDelay()\n
	/// \param nMicroSeconds Delay in microseconds
	void usDelay (unsigned nMicroSeconds)	{ SimpleusDelay (nMicroSeconds); }
	
	/// \return Pointer to the only CTimer object in the system
	static CTimer *Get (void);

	/// Can be used before CTimer is constructed
	/// \param nMilliSeconds Delay in milliseconds
	static void SimpleMsDelay (unsigned nMilliSeconds);
	/// Can be used before CTimer is constructed
	/// \param nMicroSeconds Delay in microseconds
	static void SimpleusDelay (unsigned nMicroSeconds);

private:
	void PollKernelTimers (void);

	void InterruptHandler (void);
	static void InterruptHandler (void *pParam);

	void TuneMsDelay (void);

public:
	static int IsLeapYear (unsigned nYear);
	static unsigned GetDaysOfMonth (unsigned nMonth, unsigned nYear);

private:
	CInterruptSystem	*m_pInterruptSystem;

	volatile unsigned	 m_nTicks;
	volatile unsigned	 m_nUptime;
	volatile unsigned	 m_nTime;			// local time
	CSpinLock		 m_TimeSpinLock;

	int			 m_nMinutesDiff;		// diff to UTC

	CPtrList		 m_KernelTimerList;
	CSpinLock		 m_KernelTimerSpinLock;

	unsigned		 m_nMsDelay;
	unsigned		 m_nusDelay;

	static CTimer *s_pThis;

	static const unsigned s_nDaysOfMonth[12];
	static const char *s_pMonthName[12];
};

#endif
