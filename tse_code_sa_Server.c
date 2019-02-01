#include <altera_avalon_sgdma.h>
#include <altera_avalon_sgdma_descriptor.h>//include the sgdma descriptor
#include <altera_avalon_sgdma_regs.h> //include the sgdma registers
#include <altera_avalon_PIO_regs.h>//include the PIO registers
#include <unistd.h>
#include <stdio.h>
#include <sys/alt_cache.h>
#include "sys/alt_stdio.h"
#include "sys/alt_irq.h"
#include "system.h"


// Function Prototypes
void rx_ethernet_isr (void *context);



// Create a receive frame vector
unsigned char frame_rx[1024] = { 0 };

// Create a transmit frame vector
unsigned char frame_tx[1024] = { 0 };

// Create a Payload frame vector
unsigned char string[10] = {0};

// Triple-speed Ethernet MegaCore base address
volatile int * tse = (int *) TSE_BASE;

//Other variables
int in=0;
		
// Defining the structure for TRANSPORT LAYER(UDP) for Transmitter.

struct Udp_transport_Transmitter
		{
	char DestUdp[2];
	char SourUdp[2];
	char length1[2];
	char UdpPayload[10];
		};
// Defining the structure for NETWORK LAYER(IP) for Transmitter.

struct Ip_network_Transmitter
		{
	char DestIp[4];
	char SourIp[4];
	char version[1];
	char IpPayload[16];
		};
// Defining the structure for MAC LAYER for Transmitter.

struct Mac_Transmitter
        {
	char Preamble[2];
	char DestMac[6];
	char SourMac[6];
    char length2[2];
    char Padding[23];
    char MacPayload[25];
        };



// Defining the structure for TRANSPORT LAYER(UDP) for Receiver.

struct Udp_transport_Receiver
		{
		char UDPHeader[6];
		char UDPPayload[10];
		};
// Defining the structure for NETWORK LAYER(IP) for Receiver.

struct Ip_network_Receivers
		{
		char IPHeader[9];
		char IPPayload[17];
		};
// Defining the structure for MAC LAYER for Receiver.

struct Mac_Receivers
        {
		char MACHeader[16];
		char MACPayload[25];
        };

struct Udp_transport_Transmitter UDP(char* segment);
struct Ip_network_Transmitter IP(struct Udp_transport_Transmitter packet);
struct Mac_Transmitter MAC(struct Ip_network_Transmitter frame);
struct Udp_transport_Receiver UDPrec(char* udp_rec);
struct Ip_network_Receivers Ip_network_Receiver(char* ip_rec);
struct Mac_Receivers Mac_Receiver(char* mac_rec);



// Create sgdma transmit and receive devices
alt_sgdma_dev * sgdma_tx_dev;
alt_sgdma_dev * sgdma_rx_dev;

// Allocate descriptors in the descriptor_memory (onchip memory)
alt_sgdma_descriptor tx_descriptor		__attribute__ (( section ( ".descriptor_memory" )));
alt_sgdma_descriptor tx_descriptor_end	__attribute__ (( section ( ".descriptor_memory" )));

alt_sgdma_descriptor rx_descriptor  	__attribute__ (( section ( ".descriptor_memory" )));
alt_sgdma_descriptor rx_descriptor_end  __attribute__ (( section ( ".descriptor_memory" )));

/********************************************************************************
 *Defining the 4 domain names using 4 functions for Application Layer.
 ******************************************************************************/
		int Ip_Address_1()
		{
			char ip[] = "192.156.03.04";
			int n1, n2, n3, n4;
			scanf(ip, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
			UDP(ip);
        }
		int Ip_Address_2()
		{
			char ip[] = "178.168.11.11";
			int n1, n2, n3, n4;
			scanf(ip, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
			UDP(ip);
		}
		int Ip_Address_3()
		{
			char ip[] = "168.168.68.68";
			int n1, n2, n3, n4;
			scanf(ip, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
			UDP(ip);
		}
		int Ip_Address_4()
		{
			char ip[] = "190.133.111.10";
			int n1, n2, n3, n4;
			scanf(ip, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
			UDP(ip);
		}
/********************************************************************************
 *Defining the Transport, Network, MAC Layer Protocols for server side Transmitter.
 ******************************************************************************/
struct Udp_transport_Transmitter UDP(char* string)
       {
	struct Udp_transport_Transmitter segment;
	char Dest_Udp_Add [2] = {0x48, 0x96};
	memcpy(segment.DestUdp, Dest_Udp_Add, 2);
	char Sour_Udp_Add [2] = {0x00,0x53};
	memcpy(segment.SourUdp, Sour_Udp_Add, 2);
	char len_type_1 [2] = {0x00, 0x10};
	memcpy(segment.length1, len_type_1, 2);
	memcpy(segment.UdpPayload, string, sizeof(string));
	IP(segment);
	return segment;
		};


struct Ip_network_Transmitter IP(struct Udp_transport_Transmitter pkt)
	   {
	struct Ip_network_Transmitter packet;
	char Dest_Ip_Add [4] = {0xC0,0xC0,0xC0,0xD0};
	memcpy(packet.DestIp, Dest_Ip_Add, 4);
	char Sour_Ip_Add [4] = {0xC0,0xC0,0xC0,0xC0};
	memcpy(packet.SourIp, Sour_Ip_Add, 4);
	char version[1] = {0x04};
	memcpy(packet.version, version, 1);
    memcpy(packet.IpPayload, &pkt, sizeof(struct Udp_transport_Transmitter));
	MAC(packet);
	return packet;
		};

struct Mac_Transmitter MAC(struct Ip_network_Transmitter frm)
		{
	struct Mac_Transmitter frame;
	char Preamble[2]= {0x00, 0x00};
	memcpy(frame.Preamble, Preamble, 2);
	char Dest_Mac_Add [6] = {0xA0,0xA0,0xA0,0xA0,0xA0,0xB0};
	memcpy(frame.DestMac, Dest_Mac_Add, 6);
	char Sour_Mac_Add [6] = {0xA0,0xA0,0xA0,0xA0,0xA0,0xA0};
	memcpy(frame.SourMac, Sour_Mac_Add, 8);
	char len_type_2[2] = {0x00,0x19};
	memcpy(frame.length2, len_type_2, 2);
	char Padding [23]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memcpy(frame.Padding, Padding, 23);
	memcpy(frame.MacPayload, &frm, sizeof(struct Ip_network_Transmitter));
    memcpy(frame_tx, &frame, sizeof(struct Mac_Transmitter));
	return frame;


	// Create sgdma transmit descriptor
	alt_avalon_sgdma_construct_mem_to_stream_desc(&tx_descriptor, &tx_descriptor_end, (alt_u32 *)frame_tx, 64, 0, 1, 1, 0);

	// Set up non-blocking transfer of sgdma receive descriptor
	alt_avalon_sgdma_do_async_transfer(sgdma_tx_dev, &tx_descriptor);
	// Wait until transmit descriptor transfer is complete
	while (alt_avalon_sgdma_check_descriptor_status(&tx_descriptor) != 0);
         };



/********************************************************************************
 *Defining the Application, Transport, Network, MAC Layer Protocols for server side Receiver.
********************************************************************************/
char  App_Receiver(unsigned char* received_data)
		   {
	alt_printf("Data from Client : %s \n",*received_data);
	if(*received_data == ('Google'))
	//alt_printf("Data from Client : %s \n",received_data);
	    {
					alt_printf("Ip Address is : 192.156.03.04");
					Ip_Address_1();
		}
					else if(*received_data == ('Facebook'))
		{
					alt_printf("Ip Address is : 178.168.11.11");
					Ip_Address_2();

	    }
					else if(*received_data == ('Yahoo'))
					    {
					    	alt_printf("Ip Address is : 168.168.68.68");
					    	Ip_Address_3();
					    }
					    	  else if(*received_data == ('UB'))
					    	  {
					    		  alt_printf("Ip Address is : 190.133.111.10 ");
					    		  Ip_Address_4();
					    	  }
					    		  	else
					    		  	{
					    		  		alt_printf("Invalid IP Address");
					    		  	}
		};


struct Udp_transport_Receiver UDPrec(char* udp_rec)
{
	struct Udp_transport_Receiver rec_segment;
	memcpy(rec_segment.UDPHeader, udp_rec, 6 );
	memcpy(rec_segment.UDPPayload, udp_rec + 6 , sizeof(udp_rec)-6 );
	App_Receiver(rec_segment.UDPPayload);
	return rec_segment;
}

struct Ip_network_Receivers Ip_network_Receiver(char* ip_rec)
{
	struct Ip_network_Receivers rec_packet;
	memcpy(rec_packet.IPHeader, ip_rec, 9);
	memcpy(rec_packet.IPPayload, ip_rec + 9, sizeof(ip_rec)-9);
	UDPrec(rec_packet.IPPayload);
	return rec_packet;
}

struct Mac_Receivers Mac_Receiver(char* mac_rec)
{
	struct Mac_Receivers rec_frame;
	memcpy(rec_frame.MACHeader, mac_rec, 39);
	memcpy(rec_frame.MACPayload,mac_rec + 39, sizeof(mac_rec)-39 );
	Ip_network_Receiver(rec_frame.MACPayload);
	return rec_frame;
}



/********************************************************************************
 * This program demonstrates use of the Ethernet in the DE2i-150 board.
********************************************************************************/
int main(void){

	// Open the sgdma transmit device
	sgdma_tx_dev = alt_avalon_sgdma_open ("/dev/sgdma_tx");
	if (sgdma_tx_dev == NULL)
	{
		alt_printf ("Error: could not open scatter-gather dma transmit device\n");
		//return -1;
	} else alt_printf ("Opened scatter-gather dma transmit device\n");

	// Open the sgdma receive device
	sgdma_rx_dev = alt_avalon_sgdma_open ("/dev/sgdma_rx");
	if (sgdma_rx_dev == NULL) {
		alt_printf ("Error: could not open scatter-gather dma receive device\n");
		//return -1;
	} else alt_printf ("Opened scatter-gather dma receive device\n");

	// Set interrupts for the sgdma receive device
	alt_avalon_sgdma_register_callback( sgdma_rx_dev, (alt_avalon_sgdma_callback) rx_ethernet_isr, 0x00000014, NULL );

	// Create sgdma receive descriptor
	alt_avalon_sgdma_construct_stream_to_mem_desc( &rx_descriptor, &rx_descriptor_end, (alt_u32 *)frame_rx, 0, 0 );

	// Set up non-blocking transfer of sgdma receive descriptor
	alt_avalon_sgdma_do_async_transfer( sgdma_rx_dev, &rx_descriptor );

	// Create sgdma transmit descriptor
	alt_avalon_sgdma_construct_mem_to_stream_desc(&tx_descriptor, &tx_descriptor_end, (alt_u32 *)frame_tx, 64, 0, 1, 1, 0);

	 // Set up non-blocking transfer of sgdma transmit descriptor
	alt_avalon_sgdma_do_async_transfer(sgdma_tx_dev, &tx_descriptor);

	// Specify the addresses of the PHY devices to be accessed through MDIO interface
	*(tse + 0x0F) = 0x10;

	// Disable read and write transfers and wait
	*(tse + 0x02) = *(tse + 0x02) | 0x00800220;
	while ( *(tse + 0x02 ) != ( *(tse + 0x02) | 0x00800220 ) );


	//MAC FIFO Configuration
	*(tse + 0x09) = TSE_TRANSMIT_FIFO_DEPTH-16;
	*(tse + 0x0E ) = 3 ;
	*(tse + 0x0D ) = 8;
	*(tse + 0x07 ) =TSE_RECEIVE_FIFO_DEPTH-16;
	*(tse + 0x0C ) = 8;
	*(tse + 0x0B ) = 8;
	*(tse + 0x0A ) = 0;
	*(tse + 0x08 ) = 0;

	// Initialize the MAC address
	*(tse + 0x18) = 0x19241C00 ;
	*(tse + 0x19) = 0x0000AB4D;

	// MAC function configuration
	*(tse + 0x05) = 1518;
	*(tse + 0x17) = 12;
	*(tse + 0x06) = 0xFFFF;
	*(tse + 0x02) = 0x00800220;


	// Software reset the PHY chip and wait
	*(tse + 0x02) =  0x00802220;
	while ( *(tse + 0x02 ) != ( 0x00800220 ) ) alt_printf("software reset complete ");

	// Enable read and write transfers, Gigabit Ethernet operation and promiscuous mode
	
	*(tse + 0X02) = *(tse + 0X02 ) | 0x0080023B;

	while ( *(tse + 0X02) != ( *(tse + 0X02 ) | 0x0080023B ) );


	while (1) {
		int delay = 0;
		int in = IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE);//read the input from the switch
		//tx_ethernet(in);
		while (delay < 5000000)
				{
					delay++;
				}
		alt_printf("Data from Client : %s \n", frame_rx[35]);

		// Create sgdma transmit descriptor
				alt_avalon_sgdma_construct_mem_to_stream_desc(&tx_descriptor, &tx_descriptor_end, (alt_u32 *)frame_tx, 64, 0, 1, 1, 0);

				// Set up non-blocking transfer of sgdma receive descriptor
				alt_avalon_sgdma_do_async_transfer(sgdma_tx_dev, &tx_descriptor);

				// Wait until transmit descriptor transfer is complete
			  while (alt_avalon_sgdma_check_descriptor_status(&tx_descriptor) != 0);

	}
	return 0;
	}

/****************************************************************************************
 * Subroutine to read incoming Ethernet frames
****************************************************************************************/
void rx_ethernet_isr (void *context)
{

	printf("ethernet isr function");
// Wait until receive descriptor transfer is complete
	while (alt_avalon_sgdma_check_descriptor_status(&rx_descriptor) != 0);
	alt_printf("Frame received \n");
	//Include your code to show the values of the source and destination addresses of the received frame. For example:

		Mac_Receiver(frame_rx);
		//struct Mac_Receivers mac_frame= Mac_Receiver(frame_rx);
		alt_printf("Preamble : %x,%x\n", frame_rx[0],frame_rx[1]);
		alt_printf( "Destination MAC address: %x,%x,%x,%x,%x,%x\n", frame_rx[2], frame_rx[3],frame_rx[4],frame_rx[5],frame_rx[6],frame_rx[7]);
		alt_printf( "Source MAC address: %x,%x,%x,%x,%x,%x\n", frame_rx[8], frame_rx[9],frame_rx[10],frame_rx[11],frame_rx[12],frame_rx[13]);
		alt_printf(" Length: %x,%x\n", frame_rx[14], frame_rx[15]);
		alt_printf("Destination IP Address: %x.%x.%x.%x.%x.%x.%x.%x.\n",frame_rx[16], frame_rx[17],frame_rx[18],frame_rx[19]);
		alt_printf("Source IP Address: %x.%x.%x.%x.%x.%x.%x.%x.\n",frame_rx[20],frame_rx[21],frame_rx[22],frame_rx[23]);
		alt_printf("Version: %x\n", frame_rx[24]);
		alt_printf("Destination UDP Address: %d%d\n",frame_rx[25],frame_rx[26]);
		alt_printf("Source UDP Addresss: %d%d\n",frame_rx[27],frame_rx[28]);
		alt_printf("Length: %d%d\n",frame_rx[29],frame_rx[30]);
		alt_printf("Data from Client : %c%c%c%c%c%c%c%c%c%c%c%c\n",frame_rx[31],frame_rx[32],frame_rx[33],frame_rx[34],frame_rx[35],frame_rx[36],frame_rx[37],frame_rx[38],frame_rx[39],frame_rx[40]);


    // Create new receive sgdma descriptor
	alt_avalon_sgdma_construct_stream_to_mem_desc( &rx_descriptor, &rx_descriptor_end, (alt_u32 *)frame_rx, 0, 0 );

	// Set up non-blocking transfer of sgdma receive descriptor
	alt_avalon_sgdma_do_async_transfer( sgdma_rx_dev, &rx_descriptor );
}


