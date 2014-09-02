/*
===============================================================================
 Name        : osek_example_app.c
 Author      : Pablo Ridolfi
 Version     :
 Copyright   : (c)2014, Pablo Ridolfi, DPLab@UTN-FRBA
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <stdio.h>
#include <cr_section_macros.h>

#include "os.h"               /* <= operating system header */

#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include "board.h"
#include "lpc_phy.h"
#include "arch/lpc17xx_40xx_emac.h"
#include "arch/lpc_arch.h"
#include "echo.h"

/* NETIF data */
static struct netif lpc_netif;
uint32_t physts;
static int prt_ip = 0;

/*
 * See the OIL configuration file in
 * etc/config.oil
 */

/* Comment this line in order to use LPC-Link based printf()
 * If uncommented, all printf() calls will be disabled
 */
//#define printf(...)

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	/* Setup a 1mS sysTick for the primary time base */
	SysTick_Enable(1);
}

void lwipInit(void)
{
	ip_addr_t ipaddr, netmask, gw;

	prvSetupHardware();

	/* Initialize LWIP */
	lwip_init();

	LWIP_DEBUGF(LWIP_DBG_ON, ("Starting LWIP TCP echo server...\n"));

	/* Static IP assignment */
#if LWIP_DHCP
	IP4_ADDR(&gw, 0, 0, 0, 0);
	IP4_ADDR(&ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&netmask, 0, 0, 0, 0);
#else
	IP4_ADDR(&gw, 192, 168, 0, 1);
	IP4_ADDR(&ipaddr, 192, 168, 0, 123);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
#endif

	/* Add netif interface for lpc17xx_8x */
	netif_add(&lpc_netif, &ipaddr, &netmask, &gw, NULL, lpc_enetif_init,
			  ethernet_input);
	netif_set_default(&lpc_netif);
	netif_set_up(&lpc_netif);

#if LWIP_DHCP
	dhcp_start(&lpc_netif);
#endif

	/* Initialize and start application */
	echo_init();
}

void lwipLoop(void)
{
	/* Handle packets as part of this loop, not in the IRQ handler */
	lpc_enetif_input(&lpc_netif);

	/* lpc_rx_queue will re-qeueu receive buffers. This normally occurs
	   automatically, but in systems were memory is constrained, pbufs
	   may not always be able to get allocated, so this function can be
	   optionally enabled to re-queue receive buffers. */
#if 0
	while (lpc_rx_queue(&lpc_netif)) {}
#endif

	/* Free TX buffers that are done sending */
	lpc_tx_reclaim(&lpc_netif);

	/* LWIP timers - ARP, DHCP, TCP, etc. */
	sys_check_timeouts();

	/* Call the PHY status update state machine once in a while
	   to keep the link status up-to-date */
	physts = lpcPHYStsPoll();

	/* Only check for connection state when the PHY status has changed */
	if (physts & PHY_LINK_CHANGED) {
		if (physts & PHY_LINK_CONNECTED) {
			Board_LED_Set(0, true);
			prt_ip = 0;

			/* Set interface speed and duplex */
			if (physts & PHY_LINK_SPEED100) {
				Chip_ENET_Set100Mbps(LPC_ETHERNET);
				NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 100000000);
			}
			else {
				Chip_ENET_Set10Mbps(LPC_ETHERNET);
				NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 10000000);
			}
			if (physts & PHY_LINK_FULLDUPLX) {
				Chip_ENET_SetFullDuplex(LPC_ETHERNET);
			}
			else {
				Chip_ENET_SetHalfDuplex(LPC_ETHERNET);
			}

			netif_set_link_up(&lpc_netif);
		}
		else {
			Board_LED_Set(0, false);
			netif_set_link_down(&lpc_netif);
		}
	}
}

int main(void)
{
#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, false);
#endif
#endif

    printf("Starting OSEK-OS in AppMode1\n");
    StartOS(AppMode1);

    /* we shouldn't return here */
    while(1);

    return 0 ;
}


void ErrorHook(void)
{
	/* kernel panic :( */
	ShutdownOS(0);
}

TASK(TaskInit)
{
	/* Start lwIP */
	lwipInit();

	/* Set 500 tick alarm for TaskPeriodic */
	printf("InitTask: SetRelAlarm for TaskPeriodic.\n");
	SetRelAlarm(ActivateTaskPeriodic, 0, 500);

	/* Set 1000 tick callback alarm */
	printf("InitTask: SetRelAlarm for AppCallback.\n");
	SetRelAlarm(AppCallbackAlarm, 100, 1000);

	/* Activate TaskBlink */
	printf("InitTask: Activate TaskBlink.\n");
	ActivateTask(TaskBlink);

	/* Activate TaskBackground */
	ActivateTask(TaskBackground);

	/* end InitTask */
	printf("InitTask: TerminateTask().\n");
	TerminateTask();
}

/*
 * This task waits for an event to be set in order
 * to continue execution.
 */
TASK(TaskBlink)
{
	printf("TaskBlink: Init.\n");
	while(1)
	{
		printf("TaskBlink: Waiting for event...\n");
		WaitEvent(evBlink);
		ClearEvent(evBlink);
		printf("TaskBlink: LED Toggle.\n");
		Board_LED_Toggle(0);
	}
	TerminateTask();
}

/*
 * This is a periodic task.
 */
TASK(TaskPeriodic)
{
	printf("TaskPeriodic: Event set.\n");
	SetEvent(TaskBlink, evBlink);

	/* end TaskPeriodic */
	TerminateTask();
}

/*
 * Just a background task with an infinite loop,
 * it has to be defined with the minimum priority!!!
 */
TASK(TaskBackground)
{
	volatile int i = 0;
	printf("TaskBackground: Running!\n");
	while(1)
	{
		i++;
		lwipLoop();
	}
}

/*
 * Alarm Callback example.
 */
ALARMCALLBACK(AppCallback)
{
	printf("AppCallback.\n");
}
