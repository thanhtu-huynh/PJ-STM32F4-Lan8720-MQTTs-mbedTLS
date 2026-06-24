#include "MQTTInterface.h"
#include "stm32f4xx_hal.h"

#include MBEDTLS_CONFIG_FILE
#include "mbedtls/platform.h"

#include <string.h>
#include "lwip.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "mbedtls_config.h"
//#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
//#endif
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include <string.h>

#define SERVER_PORT "8883"
#define SERVER_NAME "axn804m5xm2wy-ats.iot.ap-southeast-1.amazonaws.com"

#define DEBUG_LEVEL 1

const char mbedtls_root_certificate[] =
		"-----BEGIN CERTIFICATE-----\n"
		"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
		"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
		"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
		"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
		"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
		"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
		"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
		"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
		"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
		"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
		"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
		"-----END CERTIFICATE-----\n";


const char client_cert_pem[] =
		"-----BEGIN CERTIFICATE-----\n"
		"MIIDWjCCAkKgAwIBAgIVAMBSF0AZeBZF/PSpv5vdZB2L1xYnMA0GCSqGSIb3DQEB\n"
		"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
		"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTA4MTUwODMw\n"
		"NTNaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"
		"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAI/m8R56dU/OXNxQK4lgIqlIa\n"
		"4iuTBQZj2hujcXIknaQcC4seJ7gK9Hs8USRMN86rgynRqe4XoYWacZf13smbFSF+\n"
		"KTcFYHV8BRK//IneJaGXhU60HjXivc8IZV3LBWux7Cwl4mC/+HN5owShia8BtYVo\n"
		"tmoQzApmrlC5iu8cz8+tP6nCVsaPlqkkheydzY0VB8bPOjT0D18n162//2+8sbvn\n"
		"50aLnPAXJGXh8hUb5vfuqo3KkjWKQINB+zLwmoOcuxO79NgTYRbt9XCsB0I/9dEN\n"
		"jgCqQDor5a7WQE78LkqMkte8ZsV1hgiN1WoeOpacz/M6flgRh+gvVJXzJm46Jg==\n"
		"-----END CERTIFICATE-----\n";
const char client_key_pem[] =

		"-----BEGIN RSA PRIVATE KEY-----\n"
		"MIIEpQIBAAKCAQEAxJmuKK430YwD/ipfFl0QxYgEJv1LY6RXf4KOj35LGs678YRt\n"
		"BJjqndRxKZ0YkcT4vcdNwxIJT9HSnekXTX/tuoM2zy1zrdBNa5MIWJYO6EOTSI0Q\n"
		"VNDTw+N+6ixGpYX2kY/nUtuBv2/56cDHtWAPn1fw/yC4Ivmw8NxfJdLA7erZ2e57\n"
		"FnVVcLPZ3qqjA0SnbqOK63hZ2LTICzUYM9WkbxpDIqgbKMkpJ87dI6vPwfM9Pp+s\n"
		"TdCks3HDE2UgAO4fND9Hh9pzlz1jqfTj/0MT3h+pA32piCQr96DGZ+IBckcKVJtB\n"
		"nw95kAYKsflvmuEL9GYVf4y4djjSKClYscOihwIDAQABAoIBADsr+gQac4hpOOZt\n"
		"9Z3DsMuvQhM1wI2iAw1uxOnReRiGzjqT56kK/SECgYEAmpvmxEkPb1yopBc+UrGv\n"
		"IOuvhvY99OzIZB2oFUxFk4pLdy0rcsv8Gbnyer1GCEJvaRRrn8tQMQOD+ah0jrdk\n"
		"SySc2ptTMLsMj3gn311v/uJVaPWtqS6QtIeAU1zHHupNmZxp76+9EaiUnS6wvAnI\n"
		"XtSsOqzcbf4HmQfrFLpR1q0CgYEApdM7eY7MzITrApVrhms0DzUDQgbjEBLqWRqf\n"
		"1BBkhuLnLfbSnvKBbBDrZoZmkmoLLXIxoaJ3o3/vJeD63LFPA/jPDPR0xAf7hzsn\n"
		"7W5GJ1ddQL3K0QkHnpkFZOgc+f4C6+pc5XlLvQEMFbJAD24+v0UDoLqDL5Fw8xKz\n"
		"dvXDLcECgYEAnaOjqRTlibwKMzXZoOa14q5/JTQKMfip/U5RCM0xO6LUc5tvvmJ+\n"
		"tdURZiKkWudrfcdwxfl6RYWodLGj7Z6E+aTbmE70YVuJDwWoyMTJlcOWhUtfL3zz\n"
		"dJx0f4dSxlqagssGBRVtZ5R85EoH3e/6RGAPAtgwNHLbsdfD8v4dVBo=\n"
		"-----END RSA PRIVATE KEY-----\n";
const size_t mbedtls_root_certificate_len = strlen(mbedtls_root_certificate) + 1 ;

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
//#define MEMORY_HEAP_SIZE      1000
//uint8_t alloc_buf[MEMORY_HEAP_SIZE];
__attribute__((section(".ccmram"))) uint8_t tls_heap[48*1024];

#endif


const char *pers = "aws_iot";
mbedtls_net_context server_fd;
extern mbedtls_entropy_context entropy;
extern mbedtls_ctr_drbg_context ctr_drbg;
extern mbedtls_ssl_context ssl;
extern mbedtls_ssl_config conf;


//------------------------//
 mbedtls_x509_crt cacert;


 mbedtls_x509_crt client_cert;
 mbedtls_pk_context client_key;
 //-----------------------//

// void my_debug(void *ctx, int level, const char *file, int line, const char *str)
// {
//     ((void) ctx);
//     printf("[mbedTLS][%s:%04d] %s", file, line, str);
// }
 static void my_debug( void *ctx, int level,
                       const char *file, int line,
                       const char *str )
 {
     ((void) level);
     printf("%s:%04d: %s", file, line, str );
 }


int net_init(Network *n)
{
	int ret;

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
	 mbedtls_memory_buffer_alloc_init(tls_heap, sizeof(tls_heap));

	//mbedtls_memory_buffer_alloc_init(alloc_buf, sizeof(alloc_buf));
#endif

#if defined(MBEDTLS_DEBUG_C)


#endif

	mbedtls_net_init(&server_fd); //MX_LWIP_Init() is called already
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);


//----------------------------------------//
	mbedtls_x509_crt_init(&cacert);

	mbedtls_x509_crt_init(&client_cert);
	mbedtls_pk_init(&client_key);


//----------------------------------------//

	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);


	if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*) pers,strlen(pers))) != 0)
	{
		return -1;
	}

	//register functions
	n->mqttread = net_read; //receive function
	n->mqttwrite = net_write; //send function
	n->disconnect = net_disconnect; //disconnection function
//
//	// Parse CA
	mbedtls_x509_crt_parse(&cacert, (const unsigned char *)mbedtls_root_certificate, mbedtls_root_certificate_len);
//
//	// Parse client cert
	mbedtls_x509_crt_parse(&client_cert, (const unsigned char *)client_cert_pem, strlen(client_cert_pem) + 1);
//
//	// Parse client key
	mbedtls_pk_parse_key(&client_key, (const unsigned char *)client_key_pem, strlen(client_key_pem) + 1, NULL, 0);
	return 0;
}


int net_connect(Network *n, char *ip, int port)
{
	int ret;

	printf("function net_connect...\r\n");

	// 1. Parse root certificate
//	ret = mbedtls_x509_crt_parse(&cacert,
//			(const unsigned char*) mbedtls_root_certificate,mbedtls_root_certificate_len);
//	if (ret < 0) {
//		printf("❌ mbedtls_x509_crt_parse failed.\n");
//		return -1;
//	}

	// 2. Init socket
	mbedtls_net_init(&server_fd);

	// 3. Connect TCP
	printf("🔌 Connecting to server...\n");
	int retries = 10;
	while (retries--) {
		ret = mbedtls_net_connect(&server_fd, SERVER_NAME, SERVER_PORT, MBEDTLS_NET_PROTO_TCP); //SEVERNAME
		printf("mbedtls_net_connect return: %d\n", ret);

		if (ret == 0) break;

		char err_buf[100];
		mbedtls_strerror(ret, err_buf, sizeof(err_buf));
		printf("❌ TCP connect failed: -0x%04X: %s\n", -ret, err_buf);

		printf("🔁 Retry after 3 seconds...\n");
		osDelay(3000);
	}
	if (ret != 0) return -1;

	printf("✅ TCP connected. FD = %d\n", server_fd.fd);

	// 4. TLS config
	ret = mbedtls_ssl_config_defaults(&conf,
			MBEDTLS_SSL_IS_CLIENT,
			MBEDTLS_SSL_TRANSPORT_STREAM,
			MBEDTLS_SSL_PRESET_DEFAULT);
//==============================CONFIG VERSION==============================//

	mbedtls_ssl_conf_min_version(&conf,
	MBEDTLS_SSL_MAJOR_VERSION_3,
	MBEDTLS_SSL_MINOR_VERSION_3); // 3.3 = TLS 1.2


//==================MBEDTLS built-in SSL/TLS debug logging==================//

	//	mbedtls_ssl_conf_dbg(&conf, my_debug, NULL);
	//	mbedtls_debug_set_threshold(5);

//==========================================================================//

	//MBEDTLS_SSL_VERIFY_REQUIRED
	//MBEDTLS_SSL_VERIFY_OPTIONAL
	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);


	mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
	mbedtls_ssl_conf_own_cert(&conf, &client_cert, &client_key);


	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	//mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
	if (ret < 0) {
			printf("❌ mbedtls_ssl_config_defaults failed.\n");
			return -1;
		}

	ret = mbedtls_ssl_setup(&ssl, &conf);
	if (ret < 0) {
		printf("❌ mbedtls_ssl_setup failed.\n");
		return -1;
	}

	// 5. Set hostname (must match certificate CN/SAN)
	//"171.244.35.168"
	//"api-stg.combros.tech"
	ret = mbedtls_ssl_set_hostname(&ssl,SERVER_NAME);
	if (ret < 0) {
		printf("❌ mbedtls_ssl_set_hostname failed.\n");
		return -1;
	}

	// 6. Set BIO
	mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	// 7. TLS Handshake
	uint32_t start_time = HAL_GetTick();
	while (1) {
		printf("🚀 Start TLS handshake\n");
		ret = mbedtls_ssl_handshake(&ssl);
		printf("🚀 END TLS handshake\n");
		if (ret == 0) break;

		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			char err_buf[100];
			mbedtls_strerror(ret, err_buf, sizeof(err_buf));
			printf("❌ TLS handshake failed: -0x%04X - %s\n", -ret, err_buf);
			printf("📎 errno = %d\n", errno);
			return -1;
		}

		if ((HAL_GetTick() - start_time) > 10000) {
			printf("❌ TLS handshake timeout\n");
			return -1;
		}
		osDelay(50);
	}

	printf("✅ TLS handshake completed!\n");

	// 8. Verify cert
	ret = mbedtls_ssl_get_verify_result(&ssl);
	if (ret != 0) {
		printf("⚠️ Certificate verification result: %d\n", ret);
	} else {
		printf("✅ Certificate verified.\n");
		printf("Gia tri cuoi = %d\r\n",ret);
	}

	return 0;
}

int net_read(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
	int ret;
	int received = 0;
	int error = 0;
	int complete = 0;

	//set timeout
	if (timeout_ms != 0) {
		mbedtls_ssl_conf_read_timeout(&conf, timeout_ms);
	}

	//read until received length is bigger than variable len
	do {
		ret = mbedtls_ssl_read(&ssl, buffer, len);
		if (ret > 0) {
			received += ret;
		} else if (ret != MBEDTLS_ERR_SSL_WANT_READ) {
			error = 1;
		}
		if (received >= len) {
			complete = 1;
		}
	} while (!error && !complete);

	return received;
}

int net_write(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
	int ret;
	int written;

	//check all bytes are written
	for (written = 0; written < len; written += ret) {
		while ((ret = mbedtls_ssl_write(&ssl, buffer + written, len - written)) <= 0) {
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				return ret;
			}
		}
	}

	return written;
}

void net_disconnect(Network *n)
{
	int ret;

	do {
		ret = mbedtls_ssl_close_notify(&ssl);
	} while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

	mbedtls_ssl_session_reset(&ssl);
	mbedtls_net_free(&server_fd);
}

void net_clear()
{
	mbedtls_net_free(&server_fd);
	mbedtls_x509_crt_free(&cacert);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
	mbedtls_memory_buffer_alloc_free();
#endif
}

uint32_t MilliTimer;

//Timer functions
char TimerIsExpired(Timer *timer) {
	long left = timer->end_time - MilliTimer;
	return (left < 0);
}

void TimerCountdownMS(Timer *timer, unsigned int timeout) {
	timer->end_time = MilliTimer + timeout;
}

void TimerCountdown(Timer *timer, unsigned int timeout) {
	timer->end_time = MilliTimer + (timeout * 1000);
}

int TimerLeftMS(Timer *timer) {
	long left = timer->end_time - MilliTimer;
	return (left < 0) ? 0 : left;
}

void TimerInit(Timer *timer) {
	timer->end_time = 0;
}

