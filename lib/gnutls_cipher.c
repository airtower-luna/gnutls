/*
 * Copyright (C) 2000,2001 Nikos Mavroyanopoulos
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

#include "gnutls_int.h"
#include "gnutls_errors.h"
#include "gnutls_compress.h"
#include "gnutls_cipher.h"
#include "gnutls_algorithms.h"
#include "gnutls_hash_int.h"
#include "gnutls_cipher_int.h"
#include "debug.h"
#include "gnutls_random.h"
#include "gnutls_num.h"
#include "gnutls_datum.h"
#include "gnutls_kx.h"
#include "gnutls_record.h"
#include "gnutls_constate.h"

/* returns ciphertext which contains the headers too. This also
 * calculates the size in the header field.
 * 
 * If random pad != 0 then the random pad data will be appended.
 */
int _gnutls_encrypt(GNUTLS_STATE state, const char* headers, int headers_size,
		const char *data, size_t data_size,
		uint8 ** ciphertext, ContentType type, int random_pad)
{
	gnutls_datum plain = { (char*)data, data_size };
	gnutls_datum comp, ciph;
	int err;

	if (plain.size == 0) comp = plain;
	else {
		err = _gnutls_plaintext2TLSCompressed(state, &comp, plain);
		if (err < 0) {
			gnutls_assert();
			return err;
		}
	}

	err = _gnutls_compressed2TLSCiphertext(state, &ciph, comp, type, headers_size, random_pad);
	if (err < 0) {
		gnutls_assert();
		return err;
	}

	if (plain.size != 0) /* in that case it is not allocated */
		gnutls_free_datum(&comp);

	/* copy the headers */
	memcpy( ciph.data, headers, headers_size);
	WRITEuint16( ciph.size - headers_size, &ciph.data[3]);
	
	*ciphertext = ciph.data;

	return ciph.size;
}

int _gnutls_decrypt(GNUTLS_STATE state, char *ciphertext,
		    size_t ciphertext_size, uint8 ** data,
		    ContentType type)
{
	gnutls_datum gtxt;
	gnutls_datum gcomp;
	gnutls_datum gcipher;
	int ret;

	*data = NULL;

	if (ciphertext_size == 0)
		return 0;

	gcipher.size = ciphertext_size;
	gcipher.data = ciphertext;

	ret = _gnutls_ciphertext2TLSCompressed(state, &gcomp, gcipher, type);
	if (ret < 0) {
		return ret;
	}

	if (gcomp.size==0) gtxt = gcomp;
	else {
		ret = _gnutls_TLSCompressed2plaintext(state, &gtxt, gcomp);
		if (ret < 0) {
			gnutls_free_datum(&gcomp);
			return ret;
		}

		gnutls_free_datum(&gcomp);
	}
	
	ret = gtxt.size;

	*data = gtxt.data;
	
	return ret;
}




/* This is the actual encryption 
 * (and also keeps some space for headers (RECORD_HEADER_SIZE) in the 
 * encrypted data)
 */
int _gnutls_compressed2TLSCiphertext(GNUTLS_STATE state,
					gnutls_datum*
					cipher,
					gnutls_datum compressed, ContentType _type, 
					int headers_size,
					int random_pad)
{
	uint8 MAC[MAX_HASH_SIZE];
	uint16 c_length;
	uint8 *data;
	uint8 pad;
	uint8 rand;
	uint64 seq_num;
	int length,ret;
	GNUTLS_MAC_HANDLE td;
	uint8 type = _type;
	uint8 major, minor;
	int hash_size = _gnutls_mac_get_digest_size(state->security_parameters.write_mac_algorithm);
	GNUTLS_Version ver;
	int blocksize =
	    _gnutls_cipher_get_block_size(state->security_parameters.
					  write_bulk_cipher_algorithm);

	ver = gnutls_protocol_get_version( state);
	minor = _gnutls_version_get_minor( ver);
	major = _gnutls_version_get_major( ver);

	if ( ver == GNUTLS_SSL3) { /* SSL 3.0 */
		td =
		    gnutls_mac_init_ssl3(state->security_parameters.
					  write_mac_algorithm,
					  state->connection_state.
					  write_mac_secret.data,
					  state->connection_state.
					  write_mac_secret.size);
	} else { /* TLS 1 */
		td =
		    gnutls_hmac_init(state->security_parameters.
				     write_mac_algorithm,
				     state->connection_state.
				     write_mac_secret.data,
				     state->connection_state.
				     write_mac_secret.size);
	}
	if (td == GNUTLS_MAC_FAILED
	    && state->security_parameters.write_mac_algorithm != GNUTLS_MAC_NULL) {
		gnutls_assert();
		return GNUTLS_E_UNKNOWN_MAC_ALGORITHM;
	}

	c_length = CONVuint16(compressed.size);
	seq_num =
	    CONVuint64(&state->connection_state.write_sequence_number);

	if (td != GNUTLS_MAC_FAILED) {	/* actually when the algorithm in not the NULL one */
		gnutls_hmac(td, UINT64DATA(seq_num), 8);
		
		gnutls_hmac(td, &type, 1);
		if ( ver != GNUTLS_SSL3) { /* TLS 1.0 only */
			gnutls_hmac(td, &major, 1);
			gnutls_hmac(td, &minor, 1);
		}
		gnutls_hmac(td, &c_length, 2);
		gnutls_hmac(td, compressed.data, compressed.size);
		if ( ver == GNUTLS_SSL3) { /* SSL 3.0 */
			gnutls_mac_deinit_ssl3(td, MAC);
		} else {
			gnutls_hmac_deinit(td, MAC);
		}
	}
	switch (_gnutls_cipher_is_block(state->security_parameters.write_bulk_cipher_algorithm)) {
	case CIPHER_STREAM:
		length =
		    compressed.size + hash_size;

		data = gnutls_malloc(length+headers_size);
		if (data==NULL) {
			gnutls_assert();
			return GNUTLS_E_MEMORY_ERROR;
		}

		break;
	case CIPHER_BLOCK:
		if (_gnutls_get_random(&rand, 1, GNUTLS_WEAK_RANDOM) < 0) {
			gnutls_assert();
			return GNUTLS_E_MEMORY_ERROR;
		}

		/* make rand a multiple of blocksize */
		if ( state->security_parameters.version == GNUTLS_SSL3 ||
			random_pad==0) {
			rand = 0;
		} else {
			rand = (rand / blocksize) * blocksize;
			/* added to avoid the case of pad calculated 0
			 * seen below for pad calculation.
			 */
			if (rand > blocksize) rand-=blocksize;
		}

		length =
		    compressed.size +
		    hash_size;

		pad = (uint8) (blocksize - (length % blocksize)) + rand;

		length += pad;

		data = gnutls_malloc(length+headers_size);
		if (data==NULL) {
			gnutls_assert();
			return GNUTLS_E_MEMORY_ERROR;
		}
		
		memset(&data[headers_size + length - pad], pad - 1, pad);

		break;
	default:
		gnutls_assert();
		return GNUTLS_E_UNKNOWN_CIPHER_TYPE;
	}

	memcpy(&data[headers_size], compressed.data, compressed.size);
	memcpy(&data[compressed.size+headers_size], MAC, hash_size);

	if ( (ret = gnutls_cipher_encrypt(state->connection_state.
			      write_cipher_state, &data[headers_size], 
			      length)) < 0) {
		gnutls_free( data);
		return ret;
	}

	cipher->data = data;
	cipher->size = length + headers_size;
	return 0;

}

int _gnutls_ciphertext2TLSCompressed(GNUTLS_STATE state,
					gnutls_datum *
					compress,
					gnutls_datum ciphertext, uint8 type)
{
	uint8 MAC[MAX_HASH_SIZE];
	uint16 c_length;
	uint8 *data;
	uint8 pad;
	uint64 seq_num;
	uint16 length;
	GNUTLS_MAC_HANDLE td;
	int blocksize, ret, i;
	uint8 major, minor;
	GNUTLS_Version ver;
	int hash_size = _gnutls_mac_get_digest_size(state->security_parameters.read_mac_algorithm);

	ver = gnutls_protocol_get_version( state);
	minor = _gnutls_version_get_minor(ver);
	major = _gnutls_version_get_major(ver);

	blocksize = _gnutls_cipher_get_block_size(state->security_parameters.
					  read_bulk_cipher_algorithm);

	if ( ver == GNUTLS_SSL3) {
		td =
		    gnutls_mac_init_ssl3(state->security_parameters.
					  read_mac_algorithm,
					  state->connection_state.
					  read_mac_secret.data,
					  state->connection_state.
					  read_mac_secret.size);
	} else {
		td =
		    gnutls_hmac_init(state->security_parameters.
				     read_mac_algorithm,
				     state->connection_state.
				     read_mac_secret.data,
				     state->connection_state.
				     read_mac_secret.size);
	}

	if (td == GNUTLS_MAC_FAILED
	    && state->security_parameters.read_mac_algorithm != GNUTLS_MAC_NULL) {
		gnutls_assert();
		return GNUTLS_E_UNKNOWN_MAC_ALGORITHM;
	}

	switch (_gnutls_cipher_is_block(state->security_parameters.read_bulk_cipher_algorithm)) {
	case CIPHER_STREAM:
		if ( (ret = gnutls_cipher_decrypt(state->connection_state.
			      read_cipher_state, ciphertext.data,
			      ciphertext.size)) < 0) {
			gnutls_assert();
			return ret;
		}

		length =
		    ciphertext.size - hash_size;

		break;
	case CIPHER_BLOCK:
		if ((ciphertext.size < blocksize)
		    || (ciphertext.size % blocksize != 0)) {
			gnutls_assert();
			return GNUTLS_E_DECRYPTION_FAILED;
		}

		if ( (ret = gnutls_cipher_decrypt(state->connection_state.
			      read_cipher_state, ciphertext.data,
			      ciphertext.size)) < 0) {
			gnutls_assert();
			return ret;
		}

		pad = ciphertext.data[ciphertext.size - 1] + 1;	/* pad */

		length =
		    ciphertext.size - hash_size - pad;

		if (pad >
		    ciphertext.size - hash_size) {
			gnutls_assert();
			return GNUTLS_E_DECRYPTION_FAILED;
		}
		
		/* Check the pading bytes (TLS 1.0 only)
		 */
		if ( ver == GNUTLS_TLS1)
		for (i=2;i<pad;i++) {
			if (ciphertext.data[ciphertext.size-i] != ciphertext.data[ciphertext.size - 1]) {
				gnutls_assert();
				return GNUTLS_E_DECRYPTION_FAILED;
			}
		}
		
		break;
	default:
		gnutls_assert();
		return GNUTLS_E_UNKNOWN_CIPHER_TYPE;
	}

	if (length > 0) {
		data = gnutls_malloc(length);
		if (data==NULL) {
			gnutls_assert();
			return GNUTLS_E_MEMORY_ERROR;
		}
	} else data = NULL;

	if (data!=NULL) {
		memcpy(data, ciphertext.data, length);
	}
	compress->data = data;
	compress->size = length;

	c_length = CONVuint16((uint16) compress->size);
	seq_num = CONVuint64( &state->connection_state.read_sequence_number);

	if (td != GNUTLS_MAC_FAILED) {
		gnutls_hmac(td, UINT64DATA(seq_num), 8);
		
		gnutls_hmac(td, &type, 1);
		if ( ver != GNUTLS_SSL3) { /* TLS 1.0 only */
			gnutls_hmac(td, &major, 1);
			gnutls_hmac(td, &minor, 1);
		}
		gnutls_hmac(td, &c_length, 2);
		
		if (data!=NULL)
			gnutls_hmac(td, data, compress->size);

		if ( ver == GNUTLS_SSL3) { /* SSL 3.0 */
			gnutls_mac_deinit_ssl3(td, MAC);
		} else {
			gnutls_hmac_deinit(td, MAC);
		}
	}
	/* HMAC was not the same. */
	if (memcmp
	    (MAC, &ciphertext.data[compress->size], hash_size) != 0) {
		gnutls_free( data);
		gnutls_assert();
		return GNUTLS_E_DECRYPTION_FAILED;
	}

	return 0;
}

