/*
 * Copyright (C) 2015 Nikos Mavrogiannopoulos
 *
 * Author: Nikos Mavrogiannopoulos
 *
 * This file is part of GnuTLS.
 *
 * GnuTLS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GnuTLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GnuTLS; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)

int main()
{
	exit(77);
}

#else

#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <gnutls/gnutls.h>
#include <gnutls/dtls.h>
#include <signal.h>

#include "cert-common.h"
#include "utils.h"

/* This program tests that the client does not send the
 * status request extension if GNUTLS_NO_EXTENSIONS is set.
 */

static void server_log_func(int level, const char *str)
{
	fprintf(stderr, "server|<%d>| %s", level, str);
}

static void client_log_func(int level, const char *str)
{
	fprintf(stderr, "client|<%d>| %s", level, str);
}

static int handshake_callback(gnutls_session_t session, unsigned int htype,
	unsigned post, unsigned int incoming, const gnutls_datum_t *msg)
{
	fail("received status request\n");
	exit(1);
}
	

#define RESP "\x30\x82\x06\x8C\x0A\x01\x00\xA0\x82\x06\x85\x30\x82\x06\x81\x06\x09\x2B\x06\x01\x05\x05\x07\x30\x01\x01\x04\x82\x06\x72\x30\x82\x06\x6E\x30\x82\x01\x07\xA1\x69\x30\x67\x31\x0B\x30\x09\x06\x03\x55\x04\x06\x13\x02\x43\x48\x31\x19\x30\x17\x06\x03\x55\x04\x0A\x13\x10\x4C\x69\x6E\x75\x78\x20\x73\x74\x72\x6F\x6E\x67\x53\x77\x61\x6E\x31\x1F\x30\x1D\x06\x03\x55\x04\x0B\x13\x16\x4F\x43\x53\x50\x20\x53\x69\x67\x6E\x69\x6E\x67\x20\x41\x75\x74\x68\x6F\x72\x69\x74\x79\x31\x1C\x30\x1A\x06\x03\x55\x04\x03\x13\x13\x6F\x63\x73\x70\x2E\x73\x74\x72\x6F\x6E\x67\x73\x77\x61\x6E\x2E\x6F\x72\x67\x18\x0F\x32\x30\x31\x31\x30\x39\x32\x37\x30\x39\x35\x34\x32\x38\x5A\x30\x64\x30\x62\x30\x3A\x30\x09\x06\x05\x2B\x0E\x03\x02\x1A\x05\x00\x04\x14\x13\x9D\xA0\x9E\xF4\x32\xAB\x8F\xE2\x89\x56\x67\xFA\xD0\xD4\xE3\x35\x86\x71\xB9\x04\x14\x5D\xA7\xDD\x70\x06\x51\x32\x7E\xE7\xB6\x6D\xB3\xB5\xE5\xE0\x60\xEA\x2E\x4D\xEF\x02\x01\x1D\x80\x00\x18\x0F\x32\x30\x31\x31\x30\x39\x32\x37\x30\x39\x35\x34\x32\x38\x5A\xA0\x11\x18\x0F\x32\x30\x31\x31\x30\x39\x32\x37\x30\x39\x35\x39\x32\x38\x5A\xA1\x23\x30\x21\x30\x1F\x06\x09\x2B\x06\x01\x05\x05\x07\x30\x01\x02\x04\x12\x04\x10\x16\x89\x7D\x91\x3A\xB5\x25\xA4\x45\xFE\xC9\xFD\xC2\xE5\x08\xA4\x30\x0D\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x01\x05\x05\x00\x03\x82\x01\x01\x00\x4E\xAD\x6B\x2B\xF7\xF2\xBF\xA9\x23\x1E\x3A\x0B\x06\xDB\x55\x53\x2B\x64\x54\x11\x32\xBF\x60\xF7\x4F\xE0\x8E\x9B\xA0\xA2\x4C\x79\xC3\x2A\xE0\x43\xF7\x40\x1A\xDC\xB9\xB4\x25\xEF\x48\x01\x97\x8C\xF5\x1E\xDB\xD1\x30\x37\x73\x69\xD6\xA7\x7A\x2D\x8E\xDE\x5C\xAA\xEA\x39\xB9\x52\xAA\x25\x1E\x74\x7D\xF9\x78\x95\x8A\x92\x1F\x98\x21\xF4\x60\x7F\xD3\x28\xEE\x47\x9C\xBF\xE2\x5D\xF6\x3F\x68\x0A\xD6\xFF\x08\xC1\xDC\x95\x1E\x29\xD7\x3E\x85\xD5\x65\xA4\x4B\xC0\xAF\xC3\x78\xAB\x06\x98\x88\x19\x8A\x64\xA6\x83\x91\x87\x13\xDB\x17\xCC\x46\xBD\xAB\x4E\xC7\x16\xD1\xF8\x35\xFD\x27\xC8\xF6\x6B\xEB\x37\xB8\x08\x6F\xE2\x6F\xB4\x7E\xD5\x68\xDB\x7F\x5D\x5E\x36\x38\xF2\x77\x59\x13\xE7\x3E\x4D\x67\x5F\xDB\xA2\xF5\x5D\x7C\xBF\xBD\xB5\x37\x33\x51\x36\x63\xF8\x21\x1E\xFC\x73\x8F\x32\x69\xBB\x97\xA7\xBD\xF1\xB6\xE0\x40\x09\x68\xEA\xD5\x93\xB8\xBB\x39\x8D\xA8\x16\x1B\xBF\x04\x7A\xBC\x18\x43\x01\xE9\x3C\x19\x5C\x4D\x4B\x98\xD8\x23\x37\x39\xA4\xC4\xDD\xED\x9C\xEC\x37\xAB\x66\x44\x9B\xE7\x5B\x5D\x32\xA2\xDB\xA6\x0B\x3B\x8C\xE1\xF5\xDB\xCB\x7D\x58\xA0\x82\x04\x4B\x30\x82\x04\x47\x30\x82\x04\x43\x30\x82\x03\x2B\xA0\x03\x02\x01\x02\x02\x01\x1E\x30\x0D\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x01\x0B\x05\x00\x30\x45\x31\x0B\x30\x09\x06\x03\x55\x04\x06\x13\x02\x43\x48\x31\x19\x30\x17\x06\x03\x55\x04\x0A\x13\x10\x4C\x69\x6E\x75\x78\x20\x73\x74\x72\x6F\x6E\x67\x53\x77\x61\x6E\x31\x1B\x30\x19\x06\x03\x55\x04\x03\x13\x12\x73\x74\x72\x6F\x6E\x67\x53\x77\x61\x6E\x20\x52\x6F\x6F\x74\x20\x43\x41\x30\x1E\x17\x0D\x30\x39\x31\x31\x32\x34\x31\x32\x35\x31\x35\x33\x5A\x17\x0D\x31\x34\x31\x31\x32\x33\x31\x32\x35\x31\x35\x33\x5A\x30\x67\x31\x0B\x30\x09\x06\x03\x55\x04\x06\x13\x02\x43\x48\x31\x19\x30\x17\x06\x03\x55\x04\x0A\x13\x10\x4C\x69\x6E\x75\x78\x20\x73\x74\x72\x6F\x6E\x67\x53\x77\x61\x6E\x31\x1F\x30\x1D\x06\x03\x55\x04\x0B\x13\x16\x4F\x43\x53\x50\x20\x53\x69\x67\x6E\x69\x6E\x67\x20\x41\x75\x74\x68\x6F\x72\x69\x74\x79\x31\x1C\x30\x1A\x06\x03\x55\x04\x03\x13\x13\x6F\x63\x73\x70\x2E\x73\x74\x72\x6F\x6E\x67\x73\x77\x61\x6E\x2E\x6F\x72\x67\x30\x82\x01\x22\x30\x0D\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x01\x01\x05\x00\x03\x82\x01\x0F\x00\x30\x82\x01\x0A\x02\x82\x01\x01\x00\xBC\x05\x3E\x4B\xBE\xC6\xB1\x33\x48\x0E\xC3\xD4\x0C\xEF\x83\x0B\xBD\xBC\x57\x5F\x14\xEF\xF5\x6D\x0B\xFF\xFA\x01\x9C\xFA\x21\x6D\x5C\xAE\x79\x29\x74\xFE\xBD\xAB\x70\x87\x98\x6B\x48\x35\x79\xE3\xE0\xC1\x14\x41\x1F\x0A\xF7\xE7\xA3\xA6\xDA\x6B\xFF\xCD\x74\xE9\x95\x00\x38\xAA\xD6\x3A\x60\xC6\x64\xA1\xE6\x02\x39\x58\x4E\xFD\xF2\x78\x08\x63\xB6\xD7\x7A\x96\x79\x62\x18\x39\xEE\x27\x8D\x3B\xA2\x3D\x48\x88\xDB\x43\xD6\x6A\x77\x20\x6A\x27\x39\x50\xE0\x02\x50\x19\xF2\x7A\xCF\x78\x23\x99\x01\xD4\xE5\xB1\xD1\x31\xE6\x6B\x84\xAF\xD0\x77\x41\x46\x85\xB0\x3B\xE6\x6A\x00\x0F\x3B\x7E\x95\x7F\x59\xA8\x22\xE8\x49\x49\x05\xC8\xCB\x6C\xEE\x47\xA7\x2D\xC9\x74\x5B\xEB\x8C\xD5\x99\xC2\xE2\x70\xDB\xEA\x87\x43\x84\x0E\x4F\x83\x1C\xA6\xEB\x1F\x22\x38\x17\x69\x9B\x72\x12\x95\x48\x71\xB2\x7B\x92\x73\x52\xAB\xE3\x1A\xA5\xD3\xF4\x44\x14\xBA\xC3\x35\xDA\x91\x6C\x7D\xB4\xC2\x00\x07\xD8\x0A\x51\xF1\x0D\x4C\xD9\x7A\xD1\x99\xE6\xA8\x8D\x0A\x80\xA8\x91\xDD\x8A\xA2\x6B\xF6\xDB\xB0\x3E\xC9\x71\xA9\xE0\x39\xC3\xA3\x58\x0D\x87\xD0\xB2\xA7\x9C\xB7\x69\x02\x03\x01\x00\x01\xA3\x82\x01\x1A\x30\x82\x01\x16\x30\x09\x06\x03\x55\x1D\x13\x04\x02\x30\x00\x30\x0B\x06\x03\x55\x1D\x0F\x04\x04\x03\x02\x03\xA8\x30\x1D\x06\x03\x55\x1D\x0E\x04\x16\x04\x14\x34\x91\x6E\x91\x32\xBF\x35\x25\x43\xCC\x28\x74\xEF\x82\xC2\x57\x92\x79\x13\x73\x30\x6D\x06\x03\x55\x1D\x23\x04\x66\x30\x64\x80\x14\x5D\xA7\xDD\x70\x06\x51\x32\x7E\xE7\xB6\x6D\xB3\xB5\xE5\xE0\x60\xEA\x2E\x4D\xEF\xA1\x49\xA4\x47\x30\x45\x31\x0B\x30\x09\x06\x03\x55\x04\x06\x13\x02\x43\x48\x31\x19\x30\x17\x06\x03\x55\x04\x0A\x13\x10\x4C\x69\x6E\x75\x78\x20\x73\x74\x72\x6F\x6E\x67\x53\x77\x61\x6E\x31\x1B\x30\x19\x06\x03\x55\x04\x03\x13\x12\x73\x74\x72\x6F\x6E\x67\x53\x77\x61\x6E\x20\x52\x6F\x6F\x74\x20\x43\x41\x82\x01\x00\x30\x1E\x06\x03\x55\x1D\x11\x04\x17\x30\x15\x82\x13\x6F\x63\x73\x70\x2E\x73\x74\x72\x6F\x6E\x67\x73\x77\x61\x6E\x2E\x6F\x72\x67\x30\x13\x06\x03\x55\x1D\x25\x04\x0C\x30\x0A\x06\x08\x2B\x06\x01\x05\x05\x07\x03\x09\x30\x39\x06\x03\x55\x1D\x1F\x04\x32\x30\x30\x30\x2E\xA0\x2C\xA0\x2A\x86\x28\x68\x74\x74\x70\x3A\x2F\x2F\x63\x72\x6C\x2E\x73\x74\x72\x6F\x6E\x67\x73\x77\x61\x6E\x2E\x6F\x72\x67\x2F\x73\x74\x72\x6F\x6E\x67\x73\x77\x61\x6E\x2E\x63\x72\x6C\x30\x0D\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x01\x0B\x05\x00\x03\x82\x01\x01\x00\x6D\x78\xD7\x66\x90\xA6\xEB\xDD\xB5\x09\x48\xA4\xDA\x27\xFA\xAC\xB1\xBC\x8F\x8C\xBE\xCC\x8C\x09\xA2\x40\x0D\x6C\x4A\xAE\x72\x22\x1E\xC8\xAF\x6D\xF1\x12\xAF\xD7\x40\x51\x79\xD4\xDD\xB2\x0C\xDB\x97\x84\xB6\x24\xD5\xF5\xA8\xBB\xC0\x4B\xF9\x7F\x71\xF7\xB0\x65\x42\x4A\x7D\xFE\x76\x7E\x05\xD2\x46\xB8\x7D\xB3\x39\x4C\x5C\xB1\xFA\xB9\xEE\x3B\x70\x33\x39\x57\x1A\xB9\x95\x51\x33\x00\x25\x1B\x4C\xAA\xB4\xA7\x55\xAF\x63\x6D\x6F\x88\x17\x6A\x7F\xB0\x97\xDE\x49\x14\x6A\x27\x6A\xB0\x42\x80\xD6\xA6\x9B\xEF\x04\x5E\x11\x7D\xD5\x8E\x54\x20\xA2\x76\xD4\x66\x58\xAC\x9C\x12\xD3\xF5\xCA\x54\x98\xCA\x21\xEC\xC1\x55\xA1\x2F\x68\x0B\x5D\x04\x50\xD2\x5E\x70\x25\xD8\x13\xD9\x44\x51\x0E\x8A\x42\x08\x18\x84\xE6\x61\xCE\x5A\x7D\x7B\x81\x35\x90\xC3\xD4\x9D\x19\xB6\x37\xEE\x8F\x63\x5C\xDA\xD8\xF0\x64\x60\x39\xEB\x9B\x1C\x54\x66\x75\x76\xB5\x0A\x58\xB9\x3F\x91\xE1\x21\x9C\xA0\x50\x15\x97\xB6\x7E\x41\xBC\xD0\xC4\x21\x4C\xF5\xD7\xF0\x13\xF8\x77\xE9\x74\xC4\x8A\x0E\x20\x17\x32\xAE\x38\xC2\xA5\xA8\x62\x85\x17\xB1\xA2\xD3\x22\x9F\x95\xB7\xA3\x4C"
#define RESP_SIZE (sizeof(RESP)-1)

static int status_func(gnutls_session_t session, void *ptr, gnutls_datum_t *resp)
{
	resp->data = gnutls_malloc(RESP_SIZE);
	if (resp->data == NULL)
		return -1;

	memcpy(resp->data, RESP, RESP_SIZE);
	resp->size = RESP_SIZE;
	return 0;
}

#define MAX_BUF 1024

static void client(int fd, const char *prio)
{
	int ret;
	char buffer[MAX_BUF + 1];
	gnutls_certificate_credentials_t x509_cred;
	gnutls_session_t session;

	global_init();

	if (debug) {
		gnutls_global_set_log_function(client_log_func);
		gnutls_global_set_log_level(7);
	}

	gnutls_certificate_allocate_credentials(&x509_cred);

	/* Initialize TLS session
	 */
	gnutls_init(&session, GNUTLS_CLIENT|GNUTLS_NO_EXTENSIONS);

	assert(gnutls_priority_set_direct(session, prio, NULL)>=0);

	/* put the anonymous credentials to the current session
	 */
	gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, x509_cred);

	gnutls_transport_set_int(session, fd);

	/* Perform the TLS handshake
	 */
	do {
		ret = gnutls_handshake(session);
	}
	while (ret < 0 && gnutls_error_is_fatal(ret) == 0);

	if (ret == GNUTLS_E_UNSUPPORTED_SIGNATURE_ALGORITHM) {
		/* success */
		goto end;
	}

	if (ret < 0) {
		fail("client: Handshake failed: %s\n", gnutls_strerror(ret));
	} else {
		if (debug)
			success("client: Handshake was completed\n");
	}

	if (debug)
		success("client: TLS version is: %s\n",
			gnutls_protocol_get_name
			(gnutls_protocol_get_version(session)));

	do {
		do {
			ret = gnutls_record_recv(session, buffer, MAX_BUF);
		} while (ret == GNUTLS_E_AGAIN
			 || ret == GNUTLS_E_INTERRUPTED);
	} while (ret > 0);

	if (ret == 0) {
		if (debug)
			success
			    ("client: Peer has closed the TLS connection\n");
		goto end;
	} else if (ret < 0) {
		fail("client: Error: %s\n", gnutls_strerror(ret));
	}

	gnutls_bye(session, GNUTLS_SHUT_WR);

      end:

	close(fd);

	gnutls_deinit(session);

	gnutls_certificate_free_credentials(x509_cred);

	gnutls_global_deinit();
}


static void server(int fd, const char *prio)
{
	int ret;
	char buffer[MAX_BUF + 1];
	gnutls_session_t session;
	gnutls_certificate_credentials_t x509_cred;

	/* this must be called once in the program
	 */
	global_init();
	memset(buffer, 0, sizeof(buffer));

	if (debug) {
		gnutls_global_set_log_function(server_log_func);
		gnutls_global_set_log_level(4711);
	}

	gnutls_certificate_allocate_credentials(&x509_cred);
	gnutls_certificate_set_x509_key_mem(x509_cred, &server_cert,
					    &server_key,
					    GNUTLS_X509_FMT_PEM);

	gnutls_certificate_set_ocsp_status_request_function(x509_cred, status_func, NULL);

	gnutls_init(&session, GNUTLS_SERVER);

	gnutls_handshake_set_hook_function(session, GNUTLS_HANDSHAKE_CERTIFICATE_STATUS,
					   GNUTLS_HOOK_PRE,
					   handshake_callback);

	assert(gnutls_priority_set_direct(session, prio, NULL)>=0);

	gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, x509_cred);

	gnutls_transport_set_int(session, fd);

	do {
		ret = gnutls_handshake(session);
	} while (ret < 0 && gnutls_error_is_fatal(ret) == 0);
	if (ret < 0) {
		/* failure is expected here */
		goto end;
	}

	if (debug) {
		success("server: Handshake was completed\n");
	}

	if (debug)
		success("server: TLS version is: %s\n",
			gnutls_protocol_get_name
			(gnutls_protocol_get_version(session)));

	/* do not wait for the peer to close the connection.
	 */
	gnutls_bye(session, GNUTLS_SHUT_WR);

 end:
	close(fd);
	gnutls_deinit(session);

	gnutls_certificate_free_credentials(x509_cred);

	gnutls_global_deinit();

	if (debug)
		success("server: finished\n");
}

static void ch_handler(int sig)
{
	return;
}

static
void start(const char *prio)
{
	pid_t child;
	int fd[2];
	int ret, status = 0;

	success("trying %s\n", prio);

	signal(SIGCHLD, ch_handler);
	signal(SIGPIPE, SIG_IGN);

	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	if (ret < 0) {
		perror("socketpair");
		exit(1);
	}

	child = fork();
	if (child < 0) {
		perror("fork");
		fail("fork");
		exit(1);
	}

	if (child) {
		/* parent */
		close(fd[1]);
		client(fd[0], prio);
		waitpid(child, &status, 0);
		check_wait_status(status);
	} else {
		close(fd[0]);
		server(fd[1], prio);
		exit(0);
	}
}

void doit(void)
{
	start("NORMAL:-VERS-ALL:+VERS-TLS1.2");
	start("NORMAL:-VERS-ALL:+VERS-TLS1.3");
	start("NORMAL");
}
#endif				/* _WIN32 */
