/*
 *  Copyright 2012 James Peach
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/* sslcert.cc: Dump the subject CN from a SSL certificate */

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>
#include <openssl/ts.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

// To create a self-signed certificate that contains subjectAltNames, most
// online resources assume that you need to set up your own CA. However,
// http://www.verynoisy.com/blog/2010/jan/25/create-self-signed-certificates-multiple-domains/
// has the most straigtforward trick, which is to use an extension file when
// you create the certificate.
//

// A basic, self-signed certificate for test.trafficserver.apache.org.
static const char basic_certificate[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDuDCCAyGgAwIBAgIJAJYJC9kk4SILMA0GCSqGSIb3DQEBBQUAMIGaMQswCQYD\n"
"VQQGEwJBVTETMBEGA1UECBMKU29tZS1TdGF0ZTEhMB8GA1UEChMYSW50ZXJuZXQg\n"
"V2lkZ2l0cyBQdHkgTHRkMSYwJAYDVQQDEx10ZXN0LnRyYWZmaWNzZXJ2ZXIuYXBh\n"
"Y2hlLm9yZzErMCkGCSqGSIb3DQEJARYcZGV2QHRyYWZmaWNzZXJ2ZXIuYXBhY2hl\n"
"Lm9yZzAeFw0xMjAzMDUwMTE4MzNaFw0xMzAzMDUwMTE4MzNaMIGaMQswCQYDVQQG\n"
"EwJBVTETMBEGA1UECBMKU29tZS1TdGF0ZTEhMB8GA1UEChMYSW50ZXJuZXQgV2lk\n"
"Z2l0cyBQdHkgTHRkMSYwJAYDVQQDEx10ZXN0LnRyYWZmaWNzZXJ2ZXIuYXBhY2hl\n"
"Lm9yZzErMCkGCSqGSIb3DQEJARYcZGV2QHRyYWZmaWNzZXJ2ZXIuYXBhY2hlLm9y\n"
"ZzCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAt5eKa5xvbMd8BT9zUQ6nqB18\n"
"LFULvLZJ9iYd5yc/t6+v7MGes/3sFTFouWhst9p4xglcO06FaN6S/l600zejZK8x\n"
"VRlCDtd+33C7nME4TYS4CoWzkVwFyj0CQj03TRS68yoV3+cx03ghB+95qj9x2F2E\n"
"NsHEc0XRNg0URupNtIECAwEAAaOCAQIwgf8wHQYDVR0OBBYEFBIhkIJ8qU3bFbTK\n"
"IRTduH08UEqxMIHPBgNVHSMEgccwgcSAFBIhkIJ8qU3bFbTKIRTduH08UEqxoYGg\n"
"pIGdMIGaMQswCQYDVQQGEwJBVTETMBEGA1UECBMKU29tZS1TdGF0ZTEhMB8GA1UE\n"
"ChMYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMSYwJAYDVQQDEx10ZXN0LnRyYWZm\n"
"aWNzZXJ2ZXIuYXBhY2hlLm9yZzErMCkGCSqGSIb3DQEJARYcZGV2QHRyYWZmaWNz\n"
"ZXJ2ZXIuYXBhY2hlLm9yZ4IJAJYJC9kk4SILMAwGA1UdEwQFMAMBAf8wDQYJKoZI\n"
"hvcNAQEFBQADgYEAMfZe5tXJhTPqjCJlhbzUPvDzCrwjRMnVSiyDuhJXOF72xlWC\n"
"g4ccKUAdXbmV5sQHrISaTJD4FbuKJmFOatHnm0ZBKIT9bxqvfelFy5kWatUY2+g1\n"
"JPLZRg0M4JbOYzPyPGhckFD1ZPpdXNgOJIQzoRzzcJsGULMlQUhJoC0zCFY=\n"
"-----END CERTIFICATE-----\n"
;

// A self-signed certificate with altSubjectNames.
static const char alt_certificate[] =
"-----BEGIN CERTIFICATE-----\n"
"MIICazCCAdSgAwIBAgIJAIszqFE3DLgXMA0GCSqGSIb3DQEBBQUAMF8xCzAJBgNV\n"
"BAYTAkFVMRMwEQYDVQQIEwpTb21lLVN0YXRlMSEwHwYDVQQKExhJbnRlcm5ldCBX\n"
"aWRnaXRzIFB0eSBMdGQxGDAWBgNVBAMTD2Zvby5leGFtcGxlLm9yZzAeFw0xMjAz\n"
"MDUwNDI0MTJaFw0zOTA3MjIwNDI0MTJaMF8xCzAJBgNVBAYTAkFVMRMwEQYDVQQI\n"
"EwpTb21lLVN0YXRlMSEwHwYDVQQKExhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQx\n"
"GDAWBgNVBAMTD2Zvby5leGFtcGxlLm9yZzCBnzANBgkqhkiG9w0BAQEFAAOBjQAw\n"
"gYkCgYEAtcgbFIdAdg0eKvznAUfvoiKlUkhK2TMBjlbwt3ej88vl0dE3G1swAgZj\n"
"NPw1U5P00olGdOE6nDS7g2RgCSBWblA23jVBc0Y2AHqB0On2wj126DHiY88uVOmz\n"
"L2mkvqrxuY1drWZTgtzxSXRNE6yNWBvrqZxFz5oXvs83AuLrNusCAwEAAaMvMC0w\n"
"KwYDVR0RBCQwIoIPYmFyLmV4YW1wbGUub3Jngg9iYXouZXhhbXBsZS5vcmcwDQYJ\n"
"KoZIhvcNAQEFBQADgYEAmXnezfpxh/04U2X+e8PvPEZhsCpk26FSrvWfV7LBBtZF\n"
"gmoKIj2s2dKKWrXSTqWtJ2xecz9A9rKKLY8qjYKc/f+q3ALDz940lwyT6Fjz3c8u\n"
"Fsz7z9d6+vWjTTW1snFPCVpVJl7uTZZgbqnYNYW/TVkbVd5aFNl6s5tJnP99lvw=\n"
"-----END CERTIFICATE-----\n"
;

struct scoped_file_bio
{
    scoped_file_bio(BIO * b) : bio(b) {
    }

    ~scoped_file_bio() {
	   (void)BIO_set_close(bio, BIO_CLOSE);
	   BIO_free(bio);
    }

    operator bool() const {
	   return bio != nullptr;
    }

    BIO * bio;

private:
    scoped_file_bio(const scoped_file_bio&);
    scoped_file_bio& operator=(const scoped_file_bio&);
};

char *
asn1_strdup(ASN1_STRING * s)
{
    // Make sure we have an 8-bit encoding.
    assert(ASN1_STRING_type(s) == V_ASN1_IA5STRING ||
		ASN1_STRING_type(s) == V_ASN1_UTF8STRING ||
		ASN1_STRING_type(s) == V_ASN1_PRINTABLESTRING);

    return strndup((const char *)ASN1_STRING_data(s), ASN1_STRING_length(s));
}

char *
x509_copy_subject_alt_name_cn(X509 * x509)
{
    GENERAL_NAMES * names;

    names = (GENERAL_NAMES *)X509_get_ext_d2i(x509, NID_subject_alt_name, nullptr, nullptr);
    if (!names) {
	   printf("no subjectAltName entries\n");
	   return NULL;
    }

    unsigned count = sk_GENERAL_NAME_num(names);
    for (unsigned i = 0; i < count; ++i) {
	   GENERAL_NAME * name;

	   name = sk_GENERAL_NAME_value(names, i);
	   switch (name->type) {
	   case GEN_DNS: {
		  char * dns;
		  dns = asn1_strdup(name->d.dNSName);
		  printf("alt name: %s\n", dns);
		  free(dns);
		  break;
	   }
	   default:
		  printf("ignoring subjectAltName type %d\n",
				name->type);
	   }

    }

    if (names) {
	   GENERAL_NAMES_free(names);
    }

    return NULL;
}

char *
x509_copy_subject_cn(X509 * x509)
{
    X509_NAME * subject;

    subject = X509_get_subject_name(x509);

    for (int pos = -1; ;) {
	   pos = X509_NAME_get_index_by_NID(subject, NID_commonName, pos);
	   if (pos == -1) {
		  break;
	   }

	   X509_NAME_ENTRY * e = X509_NAME_get_entry(subject, pos);
	   ASN1_STRING * cn = X509_NAME_ENTRY_get_data(e);

	   return asn1_strdup(cn);
    }

    return NULL;
}

static void
leaks(void)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "leaks %lu", (unsigned long)getpid());
    system(cmd);
}

int main(int argc, const char ** argv)
{
    for (int i = 1; i < argc; ++i) {
	   printf("checking %s\n", argv[i]);

	   scoped_file_bio bio(BIO_new_file(argv[i], "r"));
	   X509 * cert;
	   char * subject;

	   // Read the cert from the BIO.
	   cert = PEM_read_bio_X509_AUX(bio.bio, nullptr, nullptr, nullptr);

	   // Copy the subject.
	   subject = x509_copy_subject_cn(cert);
	   assert(subject);

	   // Extract the altSubjectNames.
	   x509_copy_subject_alt_name_cn(cert);
	   printf("subject: %s\n", subject);
	   X509_free(cert);
	   free(subject);
    }

    leaks();
    return 0;
}

/* vim: set sw=4 ts=5 cindent : */
