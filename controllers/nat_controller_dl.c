#include "controller.h"
#include "messages.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

	#define MAX_MACS 1000000

	controller c;

	uint8_t macs[MAX_MACS][6];
	uint8_t portmap[MAX_MACS];
	uint8_t ips[MAX_MACS][4];
	uint8_t ipd[MAX_MACS][4];
	uint8_t dtcp_txt[MAX_MACS][2];
	//uint16_t dtcp_txt[MAX_MACS];
	uint8_t dtcp_new_txt[MAX_MACS][2];

	int mac_count = -1;

	int read_macs_and_ports_from_file(char *filename) {
		FILE *f;
		char line[200];
		int values[6];
		int values_ip[4];
		int values_dtcp[2];
		int values_dtcp_nw[2];
		int port;
		int values_stcp;
		int i;

		f = fopen(filename, "r");
		if (f == NULL) return -1;

		while (fgets(line, sizeof(line), f)) {
			line[strlen(line)-1] = '\0';
			//TODO why %c?
			if (13 == sscanf(line, "%x:%x:%x:%x:%x:%x %d.%d.%d.%d %d %d %d",
						&values[0], &values[1], &values[2],
						&values[3], &values[4], &values[5],
						&values_ip[0], &values_ip[1], &values_ip[2], &values_ip[3],
					    &values_dtcp[1], &values_dtcp_nw[1],
						&port
			) )
			{
				if (mac_count==MAX_MACS-1)
				{
					printf("Too many entries...\n");
					break;
				}

				++mac_count;
				for( i = 0; i < 6; ++i )
					macs[mac_count][i] = (uint8_t) values[i];
				for( i = 0; i < 4; ++i )
					ips[mac_count][i] = (uint8_t) values_ip[i];
				dtcp_txt[mac_count][1] = (uint8_t) values_dtcp[1];
				dtcp_txt[mac_count][0] = 0;
				dtcp_new_txt[mac_count][1] = (uint8_t) values_dtcp_nw[1];
				dtcp_new_txt[mac_count][0] = 0;
				portmap[mac_count] = (uint8_t) port;

			} else {
				printf("Wrong format error in line %d : %s\n", mac_count+2, line);
				fclose(f);
				return -1;
			}

		}

		fclose(f);
		return 0;
	}


	//void set_default_action_ipv4_lpm()
	//{
	//	char buffer[2048];
	//	struct p4_header* h;
	//	struct p4_set_default_action* sda;
	//	struct p4_action* a;
	//
	//	h = create_p4_header(buffer, 0, sizeof(buffer));
	//	sda = create_p4_set_default_action(buffer,0,sizeof(buffer));
	//        strcpy(sda->table_name, "ipv4_lpm");
	//
	//	a = &(sda->action);
	//	strcpy(a->description.name, "drop");
	//
	//	netconv_p4_header(h);
	//	netconv_p4_set_default_action(sda);
	//	netconv_p4_action(a);
	//	send_p4_msg(c, buffer, sizeof(buffer));
	//
	//	printf("########## \n");
	//	printf("\n");
	//	printf("Table name: %s\n", sda->table_name);
	//	printf("default action: %s\n", a->description.name);
	//}
	void set_default_action_nat_up()
	{
		char buffer[2048]; /* TODO: ugly */
		struct p4_header* h;
		struct p4_set_default_action* sda;
		struct p4_action* a;

		h = create_p4_header(buffer, 0, sizeof(buffer));

		sda = create_p4_set_default_action(buffer,0,sizeof(buffer));
		strcpy(sda->table_name, "nat_up");

		a = &(sda->action);
		strcpy(a->description.name, "natTcp_learn");

		netconv_p4_header(h);
		netconv_p4_set_default_action(sda);
		netconv_p4_action(a);

		send_p4_msg(c, buffer, sizeof(buffer));

		printf("\n");
		printf("Table name: %s\n", sda->table_name);
		printf("### Default action: %s\n", a->description.name);
	}
	void set_default_action_nat_dw()
	{
		char buffer[2048]; /* TODO: ugly */
		struct p4_header* h;
		struct p4_set_default_action* sda;
		struct p4_action* a;

		h = create_p4_header(buffer, 0, sizeof(buffer));

		sda = create_p4_set_default_action(buffer,0,sizeof(buffer));
		strcpy(sda->table_name, "nat_dw");

		a = &(sda->action);
		strcpy(a->description.name, "drop");

		netconv_p4_header(h);
		netconv_p4_set_default_action(sda);
		netconv_p4_action(a);

		send_p4_msg(c, buffer, sizeof(buffer));

		printf("\n");
		printf("Table name: %s\n", sda->table_name);
		printf("### Default action: %s\n", a->description.name);
	}


	void fill_if_info(uint8_t if_info, uint8_t is_int  )
	{
	    char buffer[2048]; /* TODO: ugly */
	    struct p4_header* h;
	    struct p4_add_table_entry* te;

	    struct p4_action* a;
	    struct p4_action_parameter* ap;

	    struct p4_field_match_exact* exact;

	    h = create_p4_header(buffer, 0, 2048);
	    te = create_p4_add_table_entry(buffer,0,2048);
	    strcpy(te->table_name, "if_info");

	    exact = add_p4_field_match_exact(te, 2048);
	    //strcpy(exact->header.name, "meta.routing_metadata.if_index"); // key
	    strcpy(exact->header.name, "standard_metadata.ingress_port"); // key
	    exact->bitmap[0] = if_info;
	    exact->bitmap[1] = 0;
	    exact->length = 2*8+0;
	    a = add_p4_action(h, 2048);
	    strcpy(a->description.name, "set_if_info");

		ap = add_p4_action_parameter(h, a, 2048);
		strcpy(ap->name, "is_int");
		ap->bitmap[0] = is_int;
		ap->bitmap[1] = 0;
		ap->length = 2*8+0;


	    netconv_p4_header(h);
	    netconv_p4_add_table_entry(te);
	    netconv_p4_field_match_exact(exact);
	    netconv_p4_action(a);
	    netconv_p4_action_parameter(ap);

	    send_p4_msg(c, buffer, 2048);

	    printf("########## fill info table \n");
	    //printf("\n");
	    //printf("Table name: %s\n", te->table_name);
	    //printf("Action: %s\n", a->description.name);
		//printf("%s ->", ap->name);
		//printf("%d",ap->bitmap[0]);
		//printf("\n");
	}


	void fill_smac(uint8_t mac[6] )
	{
	    char buffer[2048]; /* TODO: ugly */
	    struct p4_header* h;
	    struct p4_add_table_entry* te;
	    struct p4_action* a;

	    struct p4_field_match_exact* exact;
	    //printf("fill_smac table update \n");

	    h = create_p4_header(buffer, 0, 2048);
	    te = create_p4_add_table_entry(buffer,0,2048);
	    strcpy(te->table_name, "smac");


	    exact = add_p4_field_match_exact(te, 2048);
	    strcpy(exact->header.name, "hdr.ethernet.srcAddr"); // key
	    memcpy(exact->bitmap, mac, 6);
	    exact->length = 6*8+0;

	    a = add_p4_action(h, 2048);
	    strcpy(a->description.name, "generate_learn_notify");

	    netconv_p4_header(h);
	    netconv_p4_add_table_entry(te);
	    netconv_p4_field_match_exact(exact);
	    netconv_p4_action(a);
	    send_p4_msg(c, buffer, 2048);
	    //printf("########## fill smac table \n");
	    //printf("\n");
	    //printf("Table name: %s\n", te->table_name);
	    //printf("Action: %s\n", a->description.name);

	}

	void fill_nat_up(uint8_t ip_inn[4], uint8_t ip[4], uint8_t srctcp[2])
	{
		char buffer[2048];
		struct p4_header* h;
		struct p4_add_table_entry* te;
		struct p4_action* a;
		struct p4_action_parameter* ap,* ap2;

		struct p4_field_match_exact* exact;

		h = create_p4_header(buffer, 0, 2048);
		te = create_p4_add_table_entry(buffer,0,2048);
		strcpy(te->table_name, "nat_up");

	       // printf("Fill Table name: %s", te->table_name);

		exact = add_p4_field_match_exact(te, 2048);
		strcpy(exact->header.name, "hdr.ipv4.srcAddr");
		memcpy(exact->bitmap, ip_inn, 4);
		exact->length = 4*8+0;

	       // printf("Table match: %s \n -> ", exact->bitmap);
		//printf("%s -> ", exact->header.name);
		//for(int i = 0; i < 4; i++){
	      //          printf("%d.",exact->bitmap[i]);
	       // }	
		//printf("\n");

		a = add_p4_action(h, 2048);
		strcpy(a->description.name, "nat_hit_int_to_ext");

		//printf("action: %s -> ", a->description.name );

		ap = add_p4_action_parameter(h, a, 2048);
		strcpy(ap->name, "srcAddr");
		memcpy(ap->bitmap, ip, 4);
		ap->length = 4*8+0;

	       // printf("%s ->", ap->name);
		//for(int i = 0; i < 4; i++){ printf("%d.",ap->bitmap[i]);  }	
	    //    printf("\n");


		ap2 = add_p4_action_parameter(h, a, 2048);
		strcpy(ap2->name, "srcPort");
		memcpy(ap2->bitmap, srctcp, 2);
		ap2->length = 2*8+0;

	      //  printf("%s ->", ap2->name);
		//for(int i = 0; i < 2 ; i++){printf("%d.",ap2->bitmap[i]);  }	
		//printf("\n");



		netconv_p4_header(h);
		netconv_p4_add_table_entry(te);
		netconv_p4_field_match_exact(exact);
		netconv_p4_action(a);
		netconv_p4_action_parameter(ap);
		netconv_p4_action_parameter(ap2);



		send_p4_msg(c, buffer, 2048);

		//printf("########## \n");
		//printf("\n");
		printf("Table name: %s\n", te->table_name);
		//printf("Action: %s\n", a->description.name);
	}
	 void fill_nat_dw(uint8_t dtcp0, uint8_t ip[4], uint8_t dst_tcp)
	{
		char buffer[2048];
		struct p4_header* h;
		struct p4_add_table_entry* te;
		struct p4_action* a;
		struct p4_action_parameter* ap,* ap2;

		struct p4_field_match_exact* exact;

		h = create_p4_header(buffer, 0, 2048);
		te = create_p4_add_table_entry(buffer,0,2048);
		strcpy(te->table_name, "nat_dw");

		printf("#### Filling DW :..... %d", te->table_name);

		exact = add_p4_field_match_exact(te, 2048);
		//strcpy(exact->header.name, "hdr.ipv4.dstAddr");
		strcpy(exact->header.name, "hdr.tcp.dstPort");
	    exact->bitmap[1] = dtcp0;
	    exact->bitmap[0] = 0;
	    exact->length = 2*8+0;
		//memcpy(exact->bitmap, dtcp0, 2);
		//exact->length = 2*8+0;
		//for(int i = 0; i < 2; i++){
		//  printf("%d.",exact->bitmap[i]);
		printf("%s.",exact->bitmap);
		//}	

		a = add_p4_action(h, 2048);
		strcpy(a->description.name, "nat_hit_ext_to_int");
		//printf("action: %s -> ", a->description.name );

		ap = add_p4_action_parameter(h, a, 2048);
		strcpy(ap->name, "dstAddr");
		memcpy(ap->bitmap, ip, 4);
		ap->length = 4*8+0;

		/*ap2 = add_p4_action_parameter(h, a, 2048);
		strcpy(ap2->name, "dstPort");
		memcpy(ap2->bitmap, dst_tcp, 2);
		ap2->length = 2*8+0;*/
		
		//ap2 = add_p4_action_parameter(h, a, 2048);
		//strcpy(ap2->name, "dstPort");
		//memcpy(ap2->bitmap, dst_tcp, 2);
		//ap2->length = 2*8+0;

		ap2 = add_p4_action_parameter(h, a, 2048);
		strcpy(ap->name, "dstPort");
		ap2->bitmap[0] = dst_tcp;
		ap2->bitmap[1] = 0;
		ap2->length = 2*8+0;
		/*printf("%s ->", ap2->name);
		for(int i = 0; i < 4; i++){
			printf("%d.",ap2->bitmap[i]);
		}	
		printf("\n");*/

		netconv_p4_header(h);
		netconv_p4_add_table_entry(te);
		netconv_p4_field_match_exact(exact);
		netconv_p4_action(a);
		netconv_p4_action_parameter(ap);
		netconv_p4_action_parameter(ap2);



		send_p4_msg(c, buffer, 2048);

		//printf("########## \n");
		//printf("\n");
		//printf("fill Table nat dw: %s\n", te->table_name);
		//printf("%s ->", ap->name);
		//for(int i = 0; i < 4; i++){ printf("%d.",ap->bitmap[i]); }	
		printf("\n");
		//printf("Action: %s\n", a->description.name);
	}

	void fill_ipv4_lpm_up(uint8_t dst_ip[4], uint8_t port, uint8_t nhop [4])
	{
		char buffer[2048];
		struct p4_header* h;
		struct p4_add_table_entry* te;
		struct p4_action* a;
		struct p4_action_parameter* ap,* ap2;
		struct p4_field_match_exact* exact;

		h = create_p4_header(buffer, 0, 2048);
		te = create_p4_add_table_entry(buffer,0,2048);
		strcpy(te->table_name, "ipv4_lpm");

		exact = add_p4_field_match_exact(te, 2048);
		strcpy(exact->header.name, "meta.routing_metadata.dst_ipv4");
		//strcpy(exact->header.name, "hdr.inner_ipv4.dstAddr");
		memcpy(exact->bitmap, dst_ip, 4);
		exact->length = 4*8+0;

		a = add_p4_action(h, 2048);
		strcpy(a->description.name, "set_nhop");

		ap = add_p4_action_parameter(h, a, 2048);
		strcpy(ap->name, "port");
		ap->bitmap[0] = port;
		ap->bitmap[1] = 0;
		ap->length = 2*8+0;

		netconv_p4_header(h);
		netconv_p4_add_table_entry(te);
		netconv_p4_field_match_exact(exact);
		netconv_p4_action(a);
		netconv_p4_action_parameter(ap);
		send_p4_msg(c, buffer, 2048);

		//printf("##########\n");
		printf("Table name: %s\n", te->table_name);
		//printf("Match: %s\n", exact->header.name);
		//printf("Action: %s\n", a->description.name);
		//printf("Parameters: %s -> %d\n", ap->name, ap->bitmap[0]);
	}

	void fill_sendout_table(uint8_t port, uint8_t smac[6])
	{
	    char buffer[2048]; /* TODO: ugly */
	    struct p4_header* h;
	    struct p4_add_table_entry* te;
	    struct p4_action* a;
	    struct p4_action_parameter* ap;
	    struct p4_field_match_exact* exact;

	    h = create_p4_header(buffer, 0, 2048);
	    te = create_p4_add_table_entry(buffer,0,2048);
	    strcpy(te->table_name, "sendout");

	    exact = add_p4_field_match_exact(te, 2048);
	    strcpy(exact->header.name, "standard_metadata.egress_port");
	    exact->bitmap[0] = port;
	    exact->bitmap[1] = 0;
	    exact->length = 2*8+0;

		a = add_p4_action(h, 2048);
		strcpy(a->description.name, "rewrite_src_mac");

		ap = add_p4_action_parameter(h, a, 2048);
		strcpy(ap->name, "src_mac");
		memcpy(ap->bitmap, smac, 6);
		ap->length = 6*8+0;

	    netconv_p4_header(h);
	    netconv_p4_add_table_entry(te);
	    netconv_p4_field_match_exact(exact);
	    netconv_p4_action(a);
	    netconv_p4_action_parameter(ap);
	    send_p4_msg(c, buffer, 2048);

		//printf("##########\n");
		//printf("\n");
		printf("Table name: %s\n", te->table_name);
		//printf("Match: %s\n", exact->header.name);
		//printf("Action: %s\n", a->description.name);
		//printf("Parameters: %s ->", ap->name);
		
		for(int i = 0; i < 6; i++){
			printf("%d",ap->bitmap[i]);
		}

			
	}



	void dhf(void* b) {
		printf("Unknown digest received\n");
	}

	void init() {
		 printf("Set default actions.\n");

		int i;
		uint8_t port1 = 1;
		uint8_t port0 = 0;
		uint8_t tn_id0 = 100;

		uint8_t ip2[4] = {192,168,0,1};

		uint8_t ip_dst_up[4] = {192,168,0,10};
		uint8_t ip_dst_dw[4] = {10,0,0,10};     //2
		uint8_t stcp[2] = {10};
		uint8_t dtcp[2] = {20};   // 3
		//uint8_t mac2[6] = {0xa0, 0x36, 0x9f, 0x3e, 0x94, 0xe8};
		//uint8_t smac[6] = {0x00, 0x44, 0x00, 0x00, 0x00, 0x00};  //a0:36:9f:3e:94:ea  00:44:00:00:00:00 46:d5:f0:90:4c:58
		int8_t smac_2[6] = {0x00, 0x55, 0x00, 0x00, 0x00, 0x00};  //
		//uint8_t smac_veth1[6] = {0x46, 0xd5, 0xf0, 0x90, 0x4c, 0x58};  //a0:36:9f:3e:94:ea  00:44:00:00:00:00 46:d5:f0:90:4c:58
		uint8_t mac_if1[6] = {0xaa, 0xbb, 0xcc, 0xaa, 0xdd, 0xee};
		uint8_t mac_if0[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x55};
		//uint8_t nfpa_mac[6] = {0xa0, 0x36, 0x9f, 0x3e, 0x94, 0xe8};  //

		uint8_t is_ext = 0;
		uint8_t is_int = 1;
		
        uint8_t destport = 25;
        uint8_t destport1 = 50;

        set_default_action_nat_up();
        set_default_action_nat_dw();
        //set_default_action_ipv4_forward();

        //fill_smac(smac_veth1);
        fill_smac(smac_2);  // esta smac del paquete
        //fill_smac(mac_if0);  // esta smac del paquete
        //fill_smac(nfpa_mac);  // esta smac del paquete
        //fill_smac(nfpa_mac2);  // esta smac del paquete
        //fill_smac(nfpa_mac3);  // esta smac del paquete

        //fill_if_info(port0,is_int); //  set this to up use case    *UP nat
        //fill_if_info(port1,is_ex); //  set this to up use case
        fill_if_info(port0,is_ext); //  set this to dw use case   *DL

        //fill_nat_up(ip_dst_dw,ip2,stcp); //  set this to up use case, src ip inner  NAT
        fill_nat_dw(destport,ip_dst_dw,destport1 );   //  set this to dw use case *DL

        fill_ipv4_lpm_up(ip_dst_dw, port1, ip_dst_dw);   //this case for UP NAT
          //fill_ipv4_lpm_up(end_gre, port1, ip_dst_up);     // this case for DW *DL

        fill_sendout_table(port1, mac_if1);  // port 1 fijo para nat up, set 1 for up case *
        //fill_sendout_table(port0, mac_if0);



        for (i=0;i<=mac_count;++i)
        {

               printf("\n Filling tables smac/decap/nat_up/lpm_up MAC: %02x:%02x:%02x:%02x:%02x:%02x src_IP: %d.%d.%d.%d dst_port: %d dstTcpPort %d port %d \n", macs[i][0],macs[i][1],macs[i][2],macs[i][3],macs[i][4],macs[i][5], ips[i][0],ips[i][1],ips[i][2],ips[i][3] ,dtcp_txt[i][1], dtcp_new_txt[i][1],portmap[i] );

               //printf("****dtcp 0000 %d \n", dtcp_txt[i]);
               printf("****dtcp %d \n", dtcp_txt[i][1]);
               fill_smac(macs[i]);  // esta smac del paquete
               //printf("inside sleep first \n");
               //sleep(1);
               //fill_nat_up(ips[i],ipd[i],dtcp); // nat up
               //printf("inside sleep two \n");
               //sleep(1);
               //fill_nat_dw(is_ext,ips[i],dtcp);   //  ips this case is ip_dst_dw:10.0.0.10
               fill_nat_dw(dtcp_txt[i][1],ipd[i],dtcp_new_txt[i][1]); // nat up
               //printf("inside sleep 0 \n");
               //fill_ipv4_lpm_up(ipd[i],port1, ipd[i]);
               fill_ipv4_lpm_up(ipd[i],port1 , ipd[i]);


               if(0 == (i%500)){ printf("inside sleep \n");sleep(1);;}
               //printf("inside sleep \n");


               //sleep(1);
        }




         printf("\n");

 }




 int main(int argc, char* argv[])
 {
         if (argc>1) {
                 if (argc!=2) {
                         printf("Too many arguments...\nUsage: %s <filename(optional)>\n", argv[0]);
                         return -1;
                 }
                 printf("Command line argument is present...\nLoading configuration data...\n");
                 if (read_macs_and_ports_from_file(argv[1])<0) {
                         printf("File cannnot be opened...\n");
                         return -1;
                 }
         }

         printf("Create and configure controller...\n");
         c = create_controller_with_init(11111, 3, dhf, init);
         printf("MACSAD controller started...\n");
         execute_controller(c);

         printf("MACSAD controller terminated\n");
         destroy_controller(c);
         return 0;
 }




