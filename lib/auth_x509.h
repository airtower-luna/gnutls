#ifndef AUTH_X509_H
# define AUTH_X509_H
# include "gnutls_cert.h"
# include "gnutls_auth.h"

/* This structure may be complex but, it's the only way to
 * support a server that has multiple certificates
 */
typedef struct {
	gnutls_cert ** cert_list; 
			/* contains a list of a list of certificates.
			 * eg:   [0] certificate1, certificate11, certificate111 
			 * (if more than one, one certificate certifies the one before)
			 *       [1] certificate2, certificate22, ...
			 */
	int * cert_list_length;
			/* contains the number of the certificates in a
			 * row.
			 */
	int ncerts;     /* contains the number of columns in cert_list.
			 */
	gnutls_private_key * pkey;   /* private keys. It contains ncerts private
				* keys. pkey[i] corresponds to certificate in
				* cert_list[i][0].
				*/
	gnutls_cert * ca_list;
	int ncas;	/* number of CAs in the ca_list 
			 */

	/* this is a callback function to call if no appropriate
	 * client certificates were found.
	 */
	x509_cert_callback_func*	client_cert_callback;
	int 				dh_bits;
} X509PKI_CREDENTIALS_INT;

/* typedef X509PKI_CREDENTIALS_INT * X509PKI_CREDENTIALS; */
#define X509PKI_CREDENTIALS X509PKI_CREDENTIALS_INT*

typedef struct X509PKI_CLIENT_AUTH_INFO_INT {
	gnutls_DN	  peer_dn;
	gnutls_DN	  issuer_dn;
	CertificateStatus peer_certificate_status;
	int		  peer_certificate_version;
	time_t		  peer_certificate_activation_time;
	time_t		  peer_certificate_expiration_time;
	char		  subjectAltDNSName[X509_CN_SIZE];
	unsigned char	  keyUsage;
	int		  certificate_requested;
	int		  dh_bits; /* bits of the DH (if DHE_RSA is used) */
} *X509PKI_CLIENT_AUTH_INFO;

typedef struct X509PKI_CLIENT_AUTH_INFO_INT X509PKI_CLIENT_AUTH_INFO_INT;
typedef X509PKI_CLIENT_AUTH_INFO X509PKI_SERVER_AUTH_INFO;
typedef X509PKI_CLIENT_AUTH_INFO_INT X509PKI_SERVER_AUTH_INFO_INT;

void _gnutls_copy_x509_client_auth_info( X509PKI_CLIENT_AUTH_INFO info, gnutls_cert* cert, CertificateStatus verify);

/* AUTH X509 functions */
int _gnutls_gen_x509_server_certificate(GNUTLS_STATE, opaque **);
int _gnutls_gen_x509_client_certificate(GNUTLS_STATE, opaque **);
int _gnutls_gen_x509_client_cert_vrfy(GNUTLS_STATE, opaque **);
int _gnutls_gen_x509_server_cert_req(GNUTLS_STATE, opaque **);
int _gnutls_proc_x509_cert_req(GNUTLS_STATE, opaque *, int);
int _gnutls_proc_x509_client_cert_vrfy(GNUTLS_STATE, opaque *, int);
int _gnutls_proc_x509_server_certificate(GNUTLS_STATE, opaque *, int);
#define _gnutls_proc_x509_client_certificate _gnutls_proc_x509_server_certificate

#endif

