/*
	NTDLL timing exports for VarastoRobo Project 2019-11-15 by Santtu Nyman.
*/

#ifndef NTDLL_TIME_H
#define NTDLL_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

// export from ntdd.dll
typedef struct _TIME_FIELDS
{
	USHORT Year;
	USHORT Month;
	USHORT Day;
	USHORT Hour;
	USHORT Minute;
	USHORT Second;
	USHORT Milliseconds;
	USHORT Weekday;
} TIME_FIELDS, *PTIME_FIELDS;

NTSYSAPI ULONG NTAPI NtGetTickCount();
NTSYSAPI NTSTATUS NTAPI NtQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceCounter, OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL);
NTSYSAPI NTSTATUS NTAPI NtQuerySystemTime(OUT PLARGE_INTEGER SystemTime);
NTSYSAPI NTSTATUS NTAPI NtQueryTimerResolution(OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);
NTSYSAPI NTSTATUS NTAPI NtSetSystemTime(IN PLARGE_INTEGER SystemTime, OUT PLARGE_INTEGER PreviousTime OPTIONAL);
NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(IN ULONG DesiredResolution, IN BOOLEAN SetResolution, OUT PULONG CurrentResolution);
NTSYSAPI BOOLEAN NTAPI RtlTimeFieldsToTime(IN PTIME_FIELDS TimeFields, OUT PLARGE_INTEGER Time);
NTSYSAPI VOID NTAPI RtlTimeToTimeFields(IN PLARGE_INTEGER Time, OUT PTIME_FIELDS TimeFields);

#ifdef __cplusplus
}
#endif

#endif