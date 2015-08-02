// Time.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/Devices/RTC.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <StandardIO.hpp>

namespace Kernel {
namespace Time
{
	#define CYCLE1YN				24 * 60 * 60 * 365
	#define CYCLE1YL				24 * 60 * 60 * 366

	static constexpr const uint8_t DayOfYearTable[365][2] =
	{
		{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15},
		{1, 16}, {1, 17}, {1, 18}, {1, 19}, {1, 20}, {1, 21}, {1, 22}, {1, 23}, {1, 24}, {1, 25}, {1, 26}, {1, 27}, {1, 28}, {1, 29}, {1, 30}, {1, 31},

		{2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {2, 10}, {2, 11}, {2, 12}, {2, 13}, {2, 14}, {2, 15},
		{2, 16}, {2, 17}, {2, 18}, {2, 19}, {2, 20}, {2, 21}, {2, 22}, {2, 23}, {2, 24}, {2, 25}, {2, 26}, {2, 27}, {2, 28},

		{3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10}, {3, 11}, {3, 12}, {3, 13}, {3, 14}, {3, 15},
		{3, 16}, {3, 17}, {3, 18}, {3, 19}, {3, 20}, {3, 21}, {3, 22}, {3, 23}, {3, 24}, {3, 25}, {3, 26}, {3, 27}, {3, 28}, {3, 29}, {3, 30}, {3, 31},

		{4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9}, {4, 10}, {4, 11}, {4, 12}, {4, 13}, {4, 14}, {4, 15},
		{4, 16}, {4, 17}, {4, 18}, {4, 19}, {4, 20}, {4, 21}, {4, 22}, {4, 23}, {4, 24}, {4, 25}, {4, 26}, {4, 27}, {4, 28}, {4, 29}, {4, 30},

		{5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9}, {5, 10}, {5, 11}, {5, 12}, {5, 13}, {5, 14}, {5, 15},
		{5, 16}, {5, 17}, {5, 18}, {5, 19}, {5, 20}, {5, 21}, {5, 22}, {5, 23}, {5, 24}, {5, 25}, {5, 26}, {5, 27}, {5, 28}, {5, 29}, {5, 30}, {5, 31},

		{6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {6, 15},
		{6, 16}, {6, 17}, {6, 18}, {6, 19}, {6, 20}, {6, 21}, {6, 22}, {6, 23}, {6, 24}, {6, 25}, {6, 26}, {6, 27}, {6, 28}, {6, 29}, {6, 30},

		{7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8}, {7, 9}, {7, 10}, {7, 11}, {7, 12}, {7, 13}, {7, 14}, {7, 15},
		{7, 16}, {7, 17}, {7, 18}, {7, 19}, {7, 20}, {7, 21}, {7, 22}, {7, 23}, {7, 24}, {7, 25}, {7, 26}, {7, 27}, {7, 28}, {7, 29}, {7, 30}, {7, 31},

		{8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {8, 15},
		{8, 16}, {8, 17}, {8, 18}, {8, 19}, {8, 20}, {8, 21}, {8, 22}, {8, 23}, {8, 24}, {8, 25}, {8, 26}, {8, 27}, {8, 28}, {8, 29}, {8, 30}, {8, 31},

		{9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8}, {9, 9}, {9, 10}, {9, 11}, {9, 12}, {9, 13}, {9, 14}, {9, 15},
		{9, 16}, {9, 17}, {9, 18}, {9, 19}, {9, 20}, {9, 21}, {9, 22}, {9, 23}, {9, 24}, {9, 25}, {9, 26}, {9, 27}, {9, 28}, {9, 29}, {9, 30},

		{10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 5}, {10, 6}, {10, 7}, {10, 8}, {10, 9}, {10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14}, {10, 15}, {10, 16}, {10, 17}, {10, 18}, {10, 19}, {10, 20}, {10, 21}, {10, 22}, {10, 23}, {10, 24}, {10, 25}, {10, 26}, {10, 27}, {10, 28}, {10, 29}, {10, 30}, {10, 31},

		{11, 1}, {11, 2}, {11, 3}, {11, 4}, {11, 5}, {11, 6}, {11, 7}, {11, 8}, {11, 9}, {11, 10}, {11, 11}, {11, 12}, {11, 13}, {11, 14}, {11, 15}, {11, 16}, {11, 17}, {11, 18}, {11, 19}, {11, 20}, {11, 21}, {11, 22}, {11, 23}, {11, 24}, {11, 25}, {11, 26}, {11, 27}, {11, 28}, {11, 29}, {11, 30},

		{12, 1}, {12, 2}, {12, 3}, {12, 4}, {12, 5}, {12, 6}, {12, 7}, {12, 8}, {12, 9}, {12, 10}, {12, 11}, {12, 12}, {12, 13}, {12, 14}, {12, 15}, {12, 16}, {12, 17}, {12, 18}, {12, 19}, {12, 20}, {12, 21}, {12, 22}, {12, 23}, {12, 24}, {12, 25}, {12, 26}, {12, 27}, {12, 28}, {12, 29}, {12, 30}, {12, 31}
	};

	const uint16_t EpochYear	= 2000;
	uint64_t SecondsSinceEpoch	= 0;			// Epoch as defined as 1st January, 2000

	bool TimerOn = false;
	bool PrintSeconds = true;

	void GetTime()
	{
		// essentially, reverse the algorithm in RealTimeClock.cpp.
		uint64_t sse = SecondsSinceEpoch;
		uint16_t year = EpochYear;


		while(sse > (IsLeapYear(year + 1) ? CYCLE1YL : CYCLE1YN))
		{
			if(IsLeapYear(year))
				sse -= CYCLE1YL;

			else
				sse -= CYCLE1YN;

			year++;
		}

		uint16_t dayofyear = 0;

		while(sse > (24 * 60 * 60))
		{
			dayofyear++;
			sse -= 24 * 60 * 60;
		}

		// now the date's done, go do the time.
		// hours
		uint8_t hour = 0;
		while(sse > 60 * 60)
		{
			hour++;
			sse -= 60 * 60;
		}

		uint8_t minute = 0;
		while(sse > 60)
		{
			minute++;
			sse -= 60;
		}


		Kernel::SystemTime->Year	= year;
		Kernel::SystemTime->Month	= DayOfYearTable[dayofyear][0];
		Kernel::SystemTime->Day	= DayOfYearTable[dayofyear][1];
		Kernel::SystemTime->YearDay	= dayofyear;
		Kernel::SystemTime->Hour	= hour;
		Kernel::SystemTime->Hour12F	= (hour > 12 ? hour - 12 : hour);
		Kernel::SystemTime->AM	= !(hour >= 12);
		Kernel::SystemTime->Minute	= minute;
		Kernel::SystemTime->Second	= (uint8_t) sse;

		AdjustForTimezone();
	}



	bool IsLeapYear(uint16_t year)
	{
		if(year % 400 == 0)
			return true;

		else if(year % 100 != 0 && year % 4 == 0)
			return true;

		else
			return false;
	}

	uint8_t DaysInMonth(uint8_t month)
	{
		switch(month)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				return 31;

			case 4:
			case 6:
			case 9:
			case 11:
				return 30;

			case 2:
				return IsLeapYear(Kernel::SystemTime->Year) ? 29 : 28;
		}
		return 0;
	}

	void AdjustForTimezone()
	{
		if(Kernel::SystemTime->UTCOffset > 0)
		{
			Kernel::SystemTime->Hour += Kernel::SystemTime->UTCOffset;
			if(Kernel::SystemTime->Hour > 23)
			{
				if(Kernel::SystemTime->Day + 1 > Time::DaysInMonth(Kernel::SystemTime->Month))
				{
					if(Kernel::SystemTime->Month + 1 > 12)
					{
						Kernel::SystemTime->Year++;
						Kernel::SystemTime->Month = 1;
						Kernel::SystemTime->YearDay = 1;
						Kernel::SystemTime->Day = 1;
						Kernel::SystemTime->Hour -= 24;
					}
					else
					{
						Kernel::SystemTime->Month++;
						Kernel::SystemTime->YearDay = 1;
						Kernel::SystemTime->Day = 1;
						Kernel::SystemTime->Hour -= 24;
					}
				}
				else
				{
					Kernel::SystemTime->YearDay++;
					Kernel::SystemTime->Day++;
					Kernel::SystemTime->Hour -= 24;
				}
			}
		}
		else
		{
			if(-Kernel::SystemTime->UTCOffset > Kernel::SystemTime->Hour)
			{
				if(Kernel::SystemTime->Day <= 1)
				{
					if(Kernel::SystemTime->Month <= 1)
					{
						Kernel::SystemTime->Year--;
						Kernel::SystemTime->Month = 12;
						Kernel::SystemTime->YearDay = Time::IsLeapYear(Kernel::SystemTime->Year) ? 366 : 365;
						Kernel::SystemTime->Day = Time::DaysInMonth(Kernel::SystemTime->Month);
						Kernel::SystemTime->Hour = (uint8_t)(Kernel::SystemTime->Hour + 24 - Kernel::SystemTime->UTCOffset);
					}
					else
					{
						Kernel::SystemTime->Month--;
						Kernel::SystemTime->YearDay--;
						Kernel::SystemTime->Day = Time::DaysInMonth(Kernel::SystemTime->Month);
						Kernel::SystemTime->Hour = (uint8_t)(Kernel::SystemTime->Hour + 24 - Kernel::SystemTime->UTCOffset);
					}
				}
				else
				{
					Kernel::SystemTime->YearDay--;
					Kernel::SystemTime->Day--;
					Kernel::SystemTime->Hour = (uint8_t)(Kernel::SystemTime->Hour + 24 - Kernel::SystemTime->UTCOffset);
				}
			}
			else
			{
				Kernel::SystemTime->Hour += Kernel::SystemTime->UTCOffset;
			}
		}

		Kernel::SystemTime->Hour12F	= (Kernel::SystemTime->Hour > 12 ? Kernel::SystemTime->Hour - 12 : Kernel::SystemTime->Hour);
		Kernel::SystemTime->AM	= !(Kernel::SystemTime->Hour >= 12);
	}

	uint64_t Now()
	{
		return HardwareAbstraction::Devices::RTC::DidInitialise() ? Kernel::SystemTime->MillisecondsSinceEpoch : 100;
	}

	void GetHumanReadableTime(rde::string& output)
	{
		output.clear();
		// if(HardwareAbstraction::Devices::RTC::DidInitialise())
		// 	PrintToString(&output, "%d.%02d %s %02ds", Kernel::SystemTime->Hour12F, Kernel::SystemTime->Minute, Kernel::SystemTime->AM ? "am" : "pm", Kernel::SystemTime->Second);
	}

	bool IsAM()
	{
		// check whether we're am, taking utc offset into account.
		return (Kernel::SystemTime->Hour + Kernel::SystemTime->UTCOffset) > 12;
	}

	void IncrementAndRollover()
	{
		if(Kernel::SystemTime->Second == 59)
		{
			if(Kernel::SystemTime->Minute == 59)
			{
				if(Kernel::SystemTime->Hour == 23)
				{
					if(Kernel::SystemTime->Day == DaysInMonth(Kernel::SystemTime->Month))
					{
						if(Kernel::SystemTime->Month == 12)
						{
							Kernel::SystemTime->Year	++;
							Kernel::SystemTime->Month	= 1;
							Kernel::SystemTime->Day		= 1;
							Kernel::SystemTime->Hour	= 0;
							Kernel::SystemTime->Hour12F	= 0;
							Kernel::SystemTime->AM		= true;
							Kernel::SystemTime->Minute	= 0;
							Kernel::SystemTime->Second	= 0;
							Kernel::SystemTime->YearDay	= 1;
						}
						else
						{
							Kernel::SystemTime->Month	++;
							Kernel::SystemTime->Day		= 1;
							Kernel::SystemTime->Hour	= 0;
							Kernel::SystemTime->Hour12F	= 0;
							Kernel::SystemTime->AM		= true;
							Kernel::SystemTime->Minute	= 0;
							Kernel::SystemTime->Second	= 0;
							Kernel::SystemTime->YearDay	++;
						}
					}
					else
					{
						Kernel::SystemTime->Day		++;
						Kernel::SystemTime->Hour	= 0;
						Kernel::SystemTime->Hour12F	= 0;
						Kernel::SystemTime->AM		= true;
						Kernel::SystemTime->Minute	= 0;
						Kernel::SystemTime->Second	= 0;
						Kernel::SystemTime->YearDay	++;
					}
				}
				else
				{
					Kernel::SystemTime->Hour	++;
					Kernel::SystemTime->Hour12F	= (Kernel::SystemTime->Hour > 12 ? Kernel::SystemTime->Hour - 12 : Kernel::SystemTime->Hour);
					Kernel::SystemTime->AM		= IsAM();
					Kernel::SystemTime->Minute	= 0;
					Kernel::SystemTime->Second 	= 0;
				}
			}
			else
			{
				Kernel::SystemTime->Minute++;
				Kernel::SystemTime->Second = 0;
			}
		}
		else
		{
			if(Kernel::SystemTime->Second > 60)
				Kernel::SystemTime->Second = Kernel::SystemTime->Second - 60;

			Kernel::SystemTime->Second++;
		}

		if(Kernel::SystemTime->Hour12F == 0)
			Kernel::SystemTime->Hour12F = 12;
	}

	void ReSyncTime()
	{
		Kernel::HardwareAbstraction::Devices::RTC::ReadTime();
	}

	static uint64_t Counter = 0;
	const uint16_t ResyncRate = 10000;

	void UpdateTime()
	{
		Kernel::SystemTime->MillisecondsSinceEpoch += (GlobalMilliseconds / GlobalTickRate);

		if(!(Counter % GlobalTickRate))
		{
			IncrementAndRollover();
			Kernel::SystemTime->SecondsSinceEpoch++;
		}

		Counter++;
	}

	void TimeSyncService()
	{
		for(uint64_t c = 0; true; c++)
		{
			if(c == ResyncRate / (GlobalMilliseconds / GlobalTickRate))
			{
				c = 0;
				ReSyncTime();
				Log("sync");
			}

			SLEEP(500);
		}
	}

	extern "C" void RTCTimerHandler()
	{
		TimerOn = true;
		UpdateTime();

		using namespace Kernel::HardwareAbstraction::Devices;
		IOPort::WriteByte(0x70, 0x0C);
		IOPort::ReadByte(0x71);
	}

	void PrintTime()
	{
		#if 0
			uint16_t xpos = Console::GetCharsPerLine();

			// 0 = -, 1 = \, 2 = |, 3 = /
			uint64_t state = 0;
			while(true)
			{
				HardwareAbstraction::Multitasking::DisableScheduler();
				uint16_t x = Console::GetCursorX();
				uint16_t y = Console::GetCursorY();

				Console::MoveCursor(xpos, 0);

				if(state % 2 == 0)		PrintString("-");
				else if(state % 2 == 1)	PrintString("|");

				state++;
				Console::MoveCursor(x, y);

				HardwareAbstraction::Multitasking::EnableScheduler();

				SLEEP(500);
			}

			// ----------------------------------------------------------------------

			uint16_t xpos = Console::GetCharsPerLine() - 1 - (PrintSeconds ? 15 : 11);
			uint16_t x = 0, y = 0;
			rde::string spaces("                 ");

			while(true)
			{
				x = Console::GetCursorX();
				y = Console::GetCursorY();

				Console::MoveCursor(xpos, 0);
				PrintString(spaces.c_str());
				Console::MoveCursor(xpos, 0);

				PrintFmt("%s[ %d.%02d %s ", Kernel::SystemTime->Hour12F < 10 ? " " : "", Kernel::SystemTime->Hour12F + Kernel::SystemTime->UTCOffset, Kernel::SystemTime->Minute, IsAM() ? "am" : "pm");

				if(PrintSeconds)
				{
					PrintFmt("%02ds ]", Kernel::SystemTime->Second);
				}
				else
				{
					PrintString("]");
				}


				Console::MoveCursor(x, y);
				SLEEP(300);
			}
		#endif
	}
}
}












