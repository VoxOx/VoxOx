/*
 * WengoPhone, a voice over Internet phone
 * Copyright (c) Dudi Avramov's
 * Copyright (C) 2004-2005  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <windows.h>
#include "CpuUsage.h"

int main(int argc, char* argv[]) {
	int processID=0;
	CCpuUsage usageA;
	CCpuUsage usageB;
	
	printf("SystemWide Cpu Usage	 Wengophone cpu usage\n");
	printf("====================	 ====================\n");
	while (true) {
		// Display the system-wide cpu usage and the "wengophone" cpu usage
		int SystemWideCpuUsage = usageA.GetCpuUsage();
		int ProcessCpuUsageByName = usageB.GetCpuUsage("wengophone");
		printf("%19d%% %22d%% \r",SystemWideCpuUsage, ProcessCpuUsageByName);
		Sleep(1000);
	}
	return 0;
}
