/*
 * vpnconf
 *
 * Dynamically create a new tinc client for the serviced VPN
 *
 * Copyright (C) 2015-2016 Terence Kelleher. All rights reserved
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/stat.h>

void error(const char *msg);
#define PORT	62665
#define BUF_MAX	4096

// Step 1: Install tinc: apt-get install tinc
// Step 2: Create directories for nfvpn: mkdir -p /etc/tinc/nfvpn/hosts
// Step 3: Generate config files: vpnconf -c MUPD-<mac6>
// Step 4: Generate keys: tincd -n nfvpn -K4096
// Step 5: Exchange keys with Server: vpnconf -g MUPD-<mac6>

void
tinc_cfg(char *host )
{
	// Create the tinc file for the host. 
	char filename[256];
	FILE *fd;
	
	sprintf(filename, "/etc/tinc/nfvpn/tinc.conf" );

	// Create the file, overwrite any previously existing file
	fd = fopen(filename, "w" );
	if ( fd != NULL )
	{
		fprintf(fd, "Name = %s\n", host  );
		fprintf(fd, "AddressFamily = ipv4\n"  );
		fprintf(fd, "Interface = tun0\n"  );
		fprintf(fd, "ConnectTo = newforcevpn\n"  );
	}
	else
	{
		error("fdopen tinc.conf" );
	}
	fclose(fd );
	
	sprintf(filename, "/etc/tinc/nfvpn/hosts/%s", host );

	// Do not overwrite existing file
	fd = fopen(filename, "r" );
	if ( fd == NULL )
	{
		// Does not exist, Create the file
		fd = fopen(filename, "w" );
		if ( fd != NULL )
		{
			fprintf(fd, "Subnet = 10.122.122.254/32\n" );
		}
		else
		{
			error("fdopen Host Key File for write" );
		}
	}
	fclose(fd );
}

void
tinc_update(int id, char *host, char *addr, char *content )
{
	// Create the tinc file for the host. 
	char dest_fname[256];
	char src_fname[256];
	char buffer[4096];
	int sts;
	FILE *sfd;
	FILE *dfd;
	char *ptr;
	
	// Create the file, overwrite any previously existing file
	sprintf(dest_fname, "/etc/tinc/nfvpn/tinc-up" );
	dfd = fopen(dest_fname, "w" );
	if ( dfd != NULL )
	{
		fprintf(dfd, "#!/bin/sh\n"  );
		fprintf(dfd, "ifconfig $INTERFACE %s netmask 255.255.255.0\n", addr );
		fclose(dfd );
		chmod(dest_fname, 0744 );
	}
	else
	{
		error("fdopen tinc-up" );
	}
	sprintf(dest_fname, "/etc/tinc/nfvpn/tinc-down" );
	dfd = fopen(dest_fname, "w" );
	if ( dfd != NULL )
	{
		fprintf(dfd, "#!/bin/sh\n"  );
		fprintf(dfd, "ifconfig $INTERFACE down\n" );
		fclose(dfd );
		chmod(dest_fname, 0744 );
	}
	else
	{
		error("fdopen tinc-down" );
	}	
	
	// Replace the Subnet line in the hosts file  to update the new address
	sprintf(dest_fname, "/etc/tinc/nfvpn/hosts/%s.new", host );
	dfd = fopen(dest_fname, "w" );
	if ( dfd != NULL )
	{
		fprintf(dfd, "Subnet = %s/32\n", addr );
		
		sprintf(src_fname, "/etc/tinc/nfvpn/hosts/%s", host );
		sfd = fopen(src_fname, "r" );
		if ( sfd != NULL )
		{
			while ( ( ptr = fgets(buffer, 4095, sfd ) ) != NULL )
			{
				if ( strncmp(buffer, "Subnet =", 8 ) != 0 )
				{
					fputs(buffer, dfd );
				}
			}
			fclose(sfd );
		}
		else
		{
			printf("Failed to open %s\n", dest_fname );
			perror("fopen" );
		}
		fclose(dfd );
	}
	else
	{
		printf("Failed to open %s\n", dest_fname );
		perror("fopen" );
	}
	
	// Delete the original name and rename the new file to the original 
	if ( ( dfd != NULL ) && ( sfd != NULL ) )
	{
		sts = remove(src_fname );
		if ( sts )
		{
			perror("remove" );
		}
		else
		{
			sts = rename(dest_fname, src_fname );
		}
	}
	else
	{
		printf("Problem: dfd is %p and sfd is %p\n", dfd, sfd );
	}
	
	// Return the key file content
	sfd = fopen(src_fname, "r" );
	if ( sfd == NULL )
	{
		printf("Could not open %s\n", src_fname );
		error("fdopen Key File for read" );
	}
	memset(content, 0, BUF_MAX );
	fread(content, BUF_MAX, 1, sfd );
	if ( ferror(sfd) )
	{
		perror("fread");
		exit ( -2 );
	}
	fclose(sfd );
}

void
tinc_save_server(char *content )
{
	// Create the tinc file for the host. 
	char filename[256];
	FILE *fd;
	
	sprintf(filename, "/etc/tinc/nfvpn/hosts/newforcevpn" );

	fd = fopen(filename, "w" );
	if ( fd != NULL )
	{
		fwrite(content, strlen(content), 1, fd );
	}
	fclose(fd );
}



int main(int argc, char *argv[])
{
    int fd;
	int len;
	char portNum[] = "62665";
	char hostAddr[] = "vpn.newforce.us";
    char buffer[4096];
	char hostname[32];
	struct addrinfo hints;
	struct addrinfo *results;
	struct addrinfo *rp;
	int sts;
	char vpnhostname[32];
	char vpnaddress[32];
	int vpnId;
	int vpnPort;
	int i;
	
	if ( argc < 3 )
	{
		fprintf(stderr, "usage: %s -[gc] <hostname>\n", argv[0] );
		exit ( -1 );
	}
	if ( strlen(argv[2] ) > 30 )
	{
		fprintf(stderr, "hostname is too long\n" );
		exit ( -1 );
	}
	sprintf(hostname, "%s", argv[2] );
	
	if ( strncmp(argv[1], "-c", 2 ) == 0 )
	{
		// Configure local files
		tinc_cfg(hostname );
	}
	else if ( strncmp(argv[1], "-g", 2 ) == 0 )
	{
		// Get Address from server and exchange key files
	
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if ( fd < 0 )
		{
			error("socket open");
		}

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
		hints.ai_flags = 0;
		hints.ai_protocol = 0;          /* Any protocol */
		
		printf("Addr: %s, Port: %s\n", hostAddr, portNum );
		sts = getaddrinfo(hostAddr, portNum, &hints, &results );
		if ( sts != 0 )
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(sts));
			error("getaddrinfo");
		}
		for ( rp = results; rp != NULL ; rp = rp->ai_next )
		{
			printf("RP: Flags %d, Family %d, Socktype %d, Protocol %d, addrLen %d, canonname %s\n",
				rp->ai_flags,
				rp->ai_family,
				rp->ai_socktype,
				rp->ai_protocol,
				rp->ai_addrlen,
				rp->ai_canonname );
			
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
			char ipAddress[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);
			printf("The IP address is: %s\n", ipAddress);

			fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if ( fd == -1 )
			{
				printf("No Connect\n" );
				continue;
			}
			printf("Connect: %d %s %d\n", fd, ipAddress, rp->ai_addrlen );
			sts = connect(fd, rp->ai_addr, rp->ai_addrlen);
			printf("   sts = %d\n", sts );
			if ( sts == 0 )
			{
				break;                  /* Success */
			}

			close(fd);
		}
		if ( rp == NULL )
		{
			perror("Connect failed" );
			exit ( -1 );
		}

		len = write(fd, hostname, strlen(hostname) );
		if ( len < 0)
		{
			error("socket write");
		}
		bzero(buffer, 256 );
		len = read(fd, buffer, 255 );
		if ( len < 0)
		{
			error("ERROR reading from socket");
		}
		printf("%s\n", buffer );
		if ( strncmp(buffer, "bad", 3 ) == 0  )
		{
			fprintf(stderr, "Bad returned from server.\n" );
			exit ( -1 );
		}
		sts = sscanf(buffer, "name:%s , addr:%s , id:%d , port:%d", vpnhostname, vpnaddress, &vpnId, &vpnPort );
		if ( sts != 4 )
		{
			fprintf(stderr, "Could not parse returned string. %d returned\n", sts );
			exit ( -1 );
		}
		
		// tinc_cfg will create the tinc configuration files and return the Key file content
		tinc_update(vpnId, vpnhostname, vpnaddress, buffer );
		// Check the last few chars
		len = strlen(buffer);
		for ( i = (len - 10) ; i < len ; i++ )
		{
			printf("%d: %02xh\n", i, buffer[i] );
			if ( ! isprint(buffer[i]) )
			{
				buffer[i] = 0x20;
			}
		}
		
		len = write(fd, buffer, len );

		buffer[0] = 0;
		
		// Get the server file
		len = read(fd, buffer, BUF_MAX );
		if ( len < 0 )
		{
			perror("read" );
		}
		else if ( len )
		{
			// Write the server file
			tinc_save_server(buffer );
			printf("Wrote Server: len %d\n%s\n", len, buffer );
		}
		close(fd);
		
		// Restart the tinc deamon
		system("service tinc restart" );
	}
	return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
