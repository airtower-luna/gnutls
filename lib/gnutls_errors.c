/*
 *      Copyright (C) 2000 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "defines.h"
#include "gnutls_errors.h"

void tolow(char *str, int size);

#define GNUTLS_ERROR_ENTRY(name, fatal) \
	{ #name, name, fatal }

struct gnutls_error_entry {
	char *name;
	int  number;
	int  fatal;
};
typedef struct gnutls_error_entry gnutls_error_entry;

static gnutls_error_entry error_algorithms[] = {
	GNUTLS_ERROR_ENTRY( GNUTLS_E_MAC_FAILED, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_CIPHER, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_CIPHER_SUITE, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_COMPRESSION_ALGORITHM, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_MAC_ALGORITHM, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_ERROR, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_CIPHER_TYPE, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNWANTED_ALGORITHM, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_LARGE_PACKET, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNSUPPORTED_VERSION_PACKET, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNEXPECTED_PACKET_LENGTH, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_INVALID_SESSION, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNABLE_SEND_DATA, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_FATAL_ALERT_RECEIVED ,1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_RECEIVED_BAD_MESSAGE, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_RECEIVED_MORE_DATA, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNEXPECTED_PACKET, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_WARNING_ALERT_RECEIVED, 0),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_CLOSURE_ALERT_RECEIVED, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_ERROR_IN_FINISHED_PACKET, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNEXPECTED_HANDSHAKE_PACKET, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNKNOWN_KX_ALGORITHM, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_MPI_SCAN_FAILED, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_DECRYPTION_FAILED, 1),
	GNUTLS_ERROR_ENTRY( GNUTLS_E_UNIMPLEMENTED_FEATURE, 1),
	{0}
};

#define GNUTLS_ERROR_LOOP(b) \
        gnutls_error_entry *p; \
                for(p = error_algorithms; p->name != NULL; p++) { b ; }

#define GNUTLS_ERROR_ALG_LOOP(a) \
                        GNUTLS_ERROR_LOOP( if(p->number == error) { a; break; } )



int gnutls_is_fatal_error(int error)
{
	int ret = 0;
	GNUTLS_ERROR_ALG_LOOP(ret = p->fatal);
	return ret;
}

void gnutls_perror(int error)
{
	char *ret = NULL;
	char *pointerTo_;

	/* avoid prefix */
	GNUTLS_ERROR_ALG_LOOP(ret =
			      strdup(p->name + sizeof("GNUTLS_E_") - 1));


	if (ret != NULL) {
		tolow(ret, strlen(ret));
		pointerTo_ = strchr(ret, '_');

		while (pointerTo_ != NULL) {
			*pointerTo_ = ' ';
			pointerTo_ = strchr(ret, '_');
		}
	}
	fprintf(stderr, "GNUTLS ERROR: %s\n", ret);
	
	free( ret);
}

char* gnutls_strerror(int error)
{
	char *ret = NULL;
	char *pointerTo_;

	/* avoid prefix */
	GNUTLS_ERROR_ALG_LOOP(ret =
			      strdup(p->name + sizeof("GNUTLS_E_") - 1));


	if (ret != NULL) {
		tolow(ret, strlen(ret));
		pointerTo_ = strchr(ret, '_');

		while (pointerTo_ != NULL) {
			*pointerTo_ = ' ';
			pointerTo_ = strchr(ret, '_');
		}
	}
	
	return ret;
}
