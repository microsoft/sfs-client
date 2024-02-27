// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "CurlConnection.h"

#include "../ErrorHandling.h"
#include "HttpHeader.h"

#include <curl/curl.h>

#include <cstring>

#define THROW_IF_CURL_ERROR(curlCall, error)                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __curlCode = (curlCall);                                                                                  \
        std::string __message = "Curl error: " + std::string(curl_easy_strerror(__curlCode));                          \
        THROW_CODE_IF_LOG(error, __curlCode != CURLE_OK, m_handler, std::move(__message));                             \
    } while ((void)0, 0)

#define THROW_IF_CURL_SETUP_ERROR(curlCall) THROW_IF_CURL_ERROR(curlCall, ConnectionSetupFailed)
#define THROW_IF_CURL_UNEXPECTED_ERROR(curlCall) THROW_IF_CURL_ERROR(curlCall, ConnectionUnexpectedError)

// Setting a hard limit of 100k characters for the response to avoid rogue servers sending huge amounts of data
#define MAX_RESPONSE_CHARACTERS 100000

#define PROD_CERT                                                                                                      \
    "-----BEGIN CERTIFICATE-----\n"                                                                                    \
    "MIIGNjCCBB6gAwIBAgITMwAAAe3CmAr8P2aezgAAAAAB7TANBgkqhkiG9w0BAQsF"                                                 \
    "ADCBhDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcT"                                                 \
    "B1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEuMCwGA1UE"                                                 \
    "AxMlTWljcm9zb2Z0IFVwZGF0ZSBTZWN1cmUgU2VydmVyIENBIDIuMTAeFw0yMzA5"                                                 \
    "MDcxOTEzMjFaFw0yNDA5MDcxOTEzMjFaMG4xCzAJBgNVBAYTAlVTMQswCQYDVQQI"                                                 \
    "EwJXQTEQMA4GA1UEBxMHUmVkbW9uZDESMBAGA1UEChMJTWljcm9zb2Z0MQwwCgYD"                                                 \
    "VQQLEwNEU1AxHjAcBgNVBAMTFWFwaS5jZHAubWljcm9zb2Z0LmNvbTCCASIwDQYJ"                                                 \
    "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAMEnMlcU90Ds+jJvB9KKz8Xjm4eowzhg"                                                 \
    "SnN+MIc2pS0OqUo1861hgFbuxdqnNDsGufLjv1+jvg3bzt30zb/WkEjvJw1kMrM0"                                                 \
    "xjfG9odC6sNA+soGLbzkw9D9LPw4DnojDeeFxBYCQsHh7CKtckcSUCNmJ14uM84f"                                                 \
    "o/GEFmBOGzOKV/CJSJgUSI/LR/pbabAUoQ0xHNKbM6fmPOtei6Laef/+vB1kJ8Ys"                                                 \
    "hUTBX3+yDMCNHZJKZ+ZSrW/FMbdNe3K7sxHBtULKV9qPyBrEyS+3xpNZTf8YdMo0"                                                 \
    "JIbij0asBM1bxUHl2mH4X9bPxz+b1sd9sI4ZCqR53/mUCbHaVMT1950CAwEAAaOC"                                                 \
    "AbQwggGwMA4GA1UdDwEB/wQEAwIE8DATBgNVHSUEDDAKBggrBgEFBQcDATAMBgNV"                                                 \
    "HRMBAf8EAjAAMFoGA1UdEQRTMFGCFWFwaS5jZHAubWljcm9zb2Z0LmNvbYIXKi5h"                                                 \
    "cGkuY2RwLm1pY3Jvc29mdC5jb22CHyouYmFja2VuZC5hcGkuY2RwLm1pY3Jvc29m"                                                 \
    "dC5jb20wHQYDVR0OBBYEFFumBj2nJhIvwbBj1sGhgZpSK0JPMB8GA1UdIwQYMBaA"                                                 \
    "FNLyPYR0hhtQhapd5aUHmvBH0y5pMGgGA1UdHwRhMF8wXaBboFmGV2h0dHA6Ly93"                                                 \
    "d3cubWljcm9zb2Z0LmNvbS9wa2lvcHMvY3JsL01pY3Jvc29mdCUyMFVwZGF0ZSUy"                                                 \
    "MFNlY3VyZSUyMFNlcnZlciUyMENBJTIwMi4xLmNybDB1BggrBgEFBQcBAQRpMGcw"                                                 \
    "ZQYIKwYBBQUHMAKGWWh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2lvcHMvY2Vy"                                                 \
    "dHMvTWljcm9zb2Z0JTIwVXBkYXRlJTIwU2VjdXJlJTIwU2VydmVyJTIwQ0ElMjAy"                                                 \
    "LjEuY3J0MA0GCSqGSIb3DQEBCwUAA4ICAQAmLOeR2adGG5FWnGcjo+2eWW+046Rm"                                                 \
    "UP7visZeozI1h1Lhes8wiLIbuFEAeIT8DFbT/V8czCqQtWBC+oM2sZEajNOIN3f7"                                                 \
    "lMjZg7ql9ngz54J1LZOfiORHeD6kvHj7NUNp0Exk4Afm/ATMFMBsQrxCd8AB3HrF"                                                 \
    "YtZ/uaVQxCX9rG3DtWyK19PZKZXCMGqfb+aRYv+eB8wxY3pGLCKws10W5yjfZfcC"                                                 \
    "3BFSx6svgmuZ/0V7sOMcGpR61sAcPDd6vf65o3isZbdQ/aREE/YF8LZ3dgDI42Nu"                                                 \
    "i2v15qDaN9dXtM6KAuVxfzQA5AExcOo8rG5akphewCS4HSclP9CMOsE9+UvuUxsj"                                                 \
    "0BACXIN5yQ2m7o/LgrHeZ25P7zcDuZFMlja+ImmDlpmA9zHrNks7EK5Wm+MMGE8v"                                                 \
    "dJvdPL312bVdqSlHi33cW2DUTqbX9AbjuEuaXjOW2/w+yqnURVgVNm3U2l7Fa/LX"                                                 \
    "4ctiHEfo/rTEvFVdj3bsuqs8Ux/Djx87QgERgRl/Pi4x6Ve0RL5x8I0IpABHbm/e"                                                 \
    "rr1fC5xkB6x5eU/xYsRdPTLF/Jp8WwB1MUggcLr1as1kV0ME68i0FPhC+cqVQKbL"                                                 \
    "vRtKtCRZGonRXS2dpnoc1nOYPzvl3+hFurDFinZP2Fi7uNR7qB0GWnb1DEacmy4O"                                                 \
    "eK//hRjo6Vn0Mw==\n"                                                                                               \
    "-----END CERTIFICATE-----\n"

// C = US, ST = Washington, L = Redmond, O = Microsoft Corporation, CN = Microsoft Update Secure Server CA 2.1
// NotBefore: Sep 14 18:45:10 2023 GMT; NotAfter: Sep 14 18:45:10 2024 GMT
#define PPE_CERT                                                                                                       \
    "-----BEGIN CERTIFICATE-----\n"                                                                                    \
    "MIIGmjCCBIKgAwIBAgITMwAAAfEVirI/Bh4lsgAAAAAB8TANBgkqhkiG9w0BAQsF"                                                 \
    "ADCBhDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcT"                                                 \
    "B1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEuMCwGA1UE"                                                 \
    "AxMlTWljcm9zb2Z0IFVwZGF0ZSBTZWN1cmUgU2VydmVyIENBIDIuMTAeFw0yMzA5"                                                 \
    "MTQxODQ1MTBaFw0yNDA5MTQxODQ1MTBaMHoxCzAJBgNVBAYTAlVTMQswCQYDVQQI"                                                 \
    "EwJXQTEQMA4GA1UEBxMHUmVkbW9uZDESMBAGA1UEChMJTWljcm9zb2Z0MQwwCgYD"                                                 \
    "VQQLEwNEU1AxKjAoBgNVBAMTIWFwaS1pbnQuZGNhdC5kc3AubXAubWljcm9zb2Z0"                                                 \
    "LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALyIYOjQsqNZT3O+"                                                 \
    "q16QJ5irY8nCC6aZto7FchOqRZ/HTyBYKLLMGaN9qa9DoU4m2B3K1WB3pHbt0IuY"                                                 \
    "VixjzRGXj5bOKeWrom7D4Kz4UCxmttUBEIU8cO4MsnZnt5NUm03IlX+BDcko1+pU"                                                 \
    "zsvCdlh2NsIJM1YPFy8tIHwJtGSb1i7PTwD6EDANi+s1n5sRSj6Pys+GRnHlSVSa"                                                 \
    "ROY5tZh2ia48NvuN6hvod+kK64Y2V1OjBTPRF5nPuJEInHwTljry7hb1rHXcF0uY"                                                 \
    "bE7Y/9F0zQ3ppOvyRLz+zn5D5AZs9at0OCx1VDhbcYvuCGBlwHKyEYkr2bIBKGWY"                                                 \
    "nhe7+/0CAwEAAaOCAgwwggIIMA4GA1UdDwEB/wQEAwIE8DATBgNVHSUEDDAKBggr"                                                 \
    "BgEFBQcDATAMBgNVHRMBAf8EAjAAMIGxBgNVHREEgakwgaaCIWFwaS1pbnQuZGNh"                                                 \
    "dC5kc3AubXAubWljcm9zb2Z0LmNvbYIjKi5hcGktaW50LmRjYXQuZHNwLm1wLm1p"                                                 \
    "Y3Jvc29mdC5jb22CKyouYmFja2VuZC5hcGktaW50LmRjYXQuZHNwLm1wLm1pY3Jv"                                                 \
    "c29mdC5jb22CLyouZ2xiLmJhY2tlbmQuYXBpLWludC5kY2F0LmRzcC5tcC5taWNy"                                                 \
    "b3NvZnQuY29tMB0GA1UdDgQWBBRM/c80wlXrTZZfVHfIly4flEW2QDAfBgNVHSME"                                                 \
    "GDAWgBTS8j2EdIYbUIWqXeWlB5rwR9MuaTBoBgNVHR8EYTBfMF2gW6BZhldodHRw"                                                 \
    "Oi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtpb3BzL2NybC9NaWNyb3NvZnQlMjBVcGRh"                                                 \
    "dGUlMjBTZWN1cmUlMjBTZXJ2ZXIlMjBDQSUyMDIuMS5jcmwwdQYIKwYBBQUHAQEE"                                                 \
    "aTBnMGUGCCsGAQUFBzAChllodHRwOi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtpb3Bz"                                                 \
    "L2NlcnRzL01pY3Jvc29mdCUyMFVwZGF0ZSUyMFNlY3VyZSUyMFNlcnZlciUyMENB"                                                 \
    "JTIwMi4xLmNydDANBgkqhkiG9w0BAQsFAAOCAgEAQ+VEGEMe3zDqRwyDF9wWbpDj"                                                 \
    "nbhuj19wgrWPCj0Gi6XyzOJuM0NHhRmzfIxsJV9R7ULDMGxekpvgqmNnbnJYeSFP"                                                 \
    "9X9oJ9+tKp6EVn8mfNc95fsYjld+QV0V/NF2pEWSA0fKvSUmuLyxx/SVfBuXyEV4"                                                 \
    "EdB3p3SHzFuCA0hxeKmPyKSokgQatsVk2qypSdUXjT4ypmTxYWfmMDr2gxUSx2lc"                                                 \
    "2St6NVQ+NOhb3ey58egnBEGAqjAoDOrsTkqCWUB5eJ9dCMwAuocj2pjM9vQ7JQPi"                                                 \
    "zAbZHjDLqptjp6VmQWelkZlX4Mcr1lbV5uyKvruuhhfGpd4c6MkWx2mUEGAx/OIR"                                                 \
    "kb6IxOd8hA9gnLd4BUcCkVL3qm2A6GGseu36oPVx3R8o6tRM/Zkn3R4tlFetb7Fr"                                                 \
    "KBAsS317RVydj2qqUrpbGSM2OHFF0c/aHoqI2woNnd/wf/NstTJ56JKSIGsA5Ty6"                                                 \
    "NwqO9ekDjJKzM2850RBInS9eNFA/EH4Lu4Cs9HToJnfKk/mRvxaZpb6/VMdokfvz"                                                 \
    "o1tJm/hMluTJX4ta3ZdZvYItd5VADhj6EMgeSOPlm6oI1/vJwB4uqhwJlkmkZmcf"                                                 \
    "G6uMEXQkLlwMRwwQrxQyDGg9O/58VVeFKaaoZPMJ0kqZGojUHUScxaJ8q1NG9AEF"                                                 \
    "aEbBwh1qYqWF1GwEa6I=\n"                                                                                           \
    "-----END CERTIFICATE-----\n"

// C = US, ST = Washington, L = Redmond, O = Microsoft Corporation, CN = Microsoft Root Certificate Authority 2011
// PKEY: rsaEncryption, 4096 (bit); sigalg: RSA-SHA256
// NotBefore: Jun 21 17:33:35 2012 GMT; NotAfter: Jun 21 17:43:35 2027 GMT
#define INTERMEDIATE_CERT                                                                                              \
    "-----BEGIN CERTIFICATE-----\n"                                                                                    \
    "MIIHADCCBOigAwIBAgITMwAAAAq4kaLIClCl3wAAAAAACjANBgkqhkiG9w0BAQsF"                                                 \
    "ADCBiDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcT"                                                 \
    "B1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEyMDAGA1UE"                                                 \
    "AxMpTWljcm9zb2Z0IFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IDIwMTEwHhcN"                                                 \
    "MTIwNjIxMTczMzM1WhcNMjcwNjIxMTc0MzM1WjCBhDELMAkGA1UEBhMCVVMxEzAR"                                                 \
    "BgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1p"                                                 \
    "Y3Jvc29mdCBDb3Jwb3JhdGlvbjEuMCwGA1UEAxMlTWljcm9zb2Z0IFVwZGF0ZSBT"                                                 \
    "ZWN1cmUgU2VydmVyIENBIDIuMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoC"                                                 \
    "ggIBAIsV6r17t2cxpIcOFIqSCXjB1Wi28ppZ4H/IGmdG3jGaAqrI50dJ6ak6h0yF"                                                 \
    "/jwuG0cERatWEbtguFI2idurX8gorvMbOaC/BqJk2azmI0PNaZWQ5a+Ib5jb+yLC"                                                 \
    "ByxI8UyFA1pqzUBh4SIaIuObKz3s44ubK8xVZYEUuszhiv7QvDonFzpHQfu4MPEE"                                                 \
    "MtHVOW56RssyKuFzdos1OhXYjpSCZni+kXwLowGugOMJhCxpK9mMJNTyPzUHd+Gf"                                                 \
    "41RDX+yK/SRYT6NUYNIAX3VEK9Lvf7s+tl77d/vhnmalQB8xPNDjMgS4p+ulEt9w"                                                 \
    "Gmrwo1IQuFjDiH2kszH5f2FTri3Mgjr2SotDZPLMk93g1RQuSAZmEED2I+efOVC4"                                                 \
    "dyESKUB7/Hf0MNNeujezZyA7ih3/mXg5ppuFz61acjBRKlmNKBc1MnqUHbBSBX9K"                                                 \
    "BuBNfeqW1SsMoy2JWrVcKqvEtqbTX2mfEEMA/aecmMO6S7vo2CM8c7OBFjY9sbxh"                                                 \
    "msAS3TC0kLffSK3YF2oDMqdgW57PGm14ZVSP01KO5W6E8sq43xkd2rT6KZ7IHqPW"                                                 \
    "18QwPsHbffy5eQbgumeadF3cryR7ElIt1VccANw9mqA+km1DWoL3tYb+nlS0MMKd"                                                 \
    "YNFPT903Vx0chN5ej9CQXHBu4zq3Rkmz7wF5YJVEO9gZ0CJlAgMBAAGjggFjMIIB"                                                 \
    "XzAQBgkrBgEEAYI3FQEEAwIBADAdBgNVHQ4EFgQU0vI9hHSGG1CFql3lpQea8EfT"                                                 \
    "LmkwGQYJKwYBBAGCNxQCBAweCgBTAHUAYgBDAEEwCwYDVR0PBAQDAgGGMBIGA1Ud"                                                 \
    "EwEB/wQIMAYBAf8CAQAwHwYDVR0jBBgwFoAUci06AjGQQ7kUBU7h6qfHMdEjiTQw"                                                 \
    "WgYDVR0fBFMwUTBPoE2gS4ZJaHR0cDovL2NybC5taWNyb3NvZnQuY29tL3BraS9j"                                                 \
    "cmwvcHJvZHVjdHMvTWljUm9vQ2VyQXV0MjAxMV8yMDExXzAzXzIyLmNybDBeBggr"                                                 \
    "BgEFBQcBAQRSMFAwTgYIKwYBBQUHMAKGQmh0dHA6Ly93d3cubWljcm9zb2Z0LmNv"                                                 \
    "bS9wa2kvY2VydHMvTWljUm9vQ2VyQXV0MjAxMV8yMDExXzAzXzIyLmNydDATBgNV"                                                 \
    "HSUEDDAKBggrBgEFBQcDATANBgkqhkiG9w0BAQsFAAOCAgEAopvuA2tH2+meSVsn"                                                 \
    "VatGbRha0QgVj4Saq5ZlNJW0Qpyf4pRyuZT+KLK2/Ia6ZzUugrtLAECro+hIEB5K"                                                 \
    "29S+pnY1nzSMn+lSZJwGWfRZTWn466g21wKFMInPpO3QB8yfzr2zwniissTh8Jkn"                                                 \
    "8Uejsz/EkGU520E3FCT26dlU1YtzHnrcZ7d8qp4tLFEVeSsrxkqpYJQxalJIZ3HH"                                                 \
    "uhOG3BQLmLtJDs822W1knAR6c+iYuLDbJ9o8TnOY9/lIWy8Vv2z3i+LEn27O7QSl"                                                 \
    "vTZHyCgFJMgjhELOSLliGhA3411RX8kyCE9AJ1OLufdcejOYwMG0POpmrj3s/Q5n"                                                 \
    "+Bfm5JQHaGGCUy7XfKMRbyYJejjcC1YtLS5HVxBf/Smh7nruCYIpDimr8AIgJCVz"                                                 \
    "ekbVxTHzuSYdXLZgnpUzu71MgFGSBh5DAHaErUmwwztK/TIyak9zwodWouV+2clp"                                                 \
    "HYgCWASmHOkRysH6T3n46pDmexplqG5dGM5QNrCjn5u1ptgVdDPR1LGHTT+KGT+F"                                                 \
    "45owrOFOwUOuz2H6RFUPgwPkCOYnK4bM17tdlaQ4DrtghSqL03ZaPFdASXHa+fZB"                                                 \
    "1ro0E6c8fv9vqaf7NvhIQE+BepDH87/f3gCGCzZ0pTNnbRH2k2VCc4CbaWZRKWg8"                                                 \
    "5c95ZPsdlHeynrIjVZ76ubrfiuM=\n"                                                                                   \
    "-----END CERTIFICATE-----\n"

using namespace SFS;
using namespace SFS::details;

namespace
{
// Curl callback for writing data to a std::string. Must return the number of bytes written.
// This callback may be called multiple times for a single request, and will keep appending
// to userData until the request is complete. The data received is not null-terminated.
// For SFS, this data will likely be a JSON string.
size_t WriteCallback(char* contents, size_t sizeInBytes, size_t numElements, void* userData)
{
    auto readBufferPtr = static_cast<std::string*>(userData);
    if (readBufferPtr)
    {
        size_t totalSize = sizeInBytes * numElements;

        // Checking final response size to avoid unexpected amounts of data
        if ((readBufferPtr->length() + totalSize) > MAX_RESPONSE_CHARACTERS)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        readBufferPtr->append(contents, totalSize);
        return totalSize;
    }
    return CURL_WRITEFUNC_ERROR;
}

struct CurlErrorBuffer
{
  public:
    CurlErrorBuffer(CURL* handle, const ReportingHandler& reportingHandler)
        : m_handle(handle)
        , m_reportingHandler(reportingHandler)
    {
        m_errorBuffer[0] = '\0';
        SetBuffer();
    }

    ~CurlErrorBuffer()
    {
        LOG_IF_FAILED(UnsetBuffer(), m_reportingHandler);
    }

    void SetBuffer()
    {
        THROW_CODE_IF_LOG(ConnectionSetupFailed,
                          curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_errorBuffer) != CURLE_OK,
                          m_reportingHandler,
                          "Failed to set up error buffer for curl");
    }

    Result UnsetBuffer()
    {
        return curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, nullptr) == CURLE_OK
                   ? Result::Success
                   : Result(Result::ConnectionSetupFailed, "Failed to unset curl error buffer");
    }

    char* Get()
    {
        return m_errorBuffer;
    }

  private:
    CURL* m_handle;
    const ReportingHandler& m_reportingHandler;

    char m_errorBuffer[CURL_ERROR_SIZE];
};

Result CurlCodeToResult(CURLcode curlCode, char* errorBuffer)
{
    Result::Code code;
    std::string message;
    switch (curlCode)
    {
    case CURLE_OPERATION_TIMEDOUT:
        code = Result::HttpTimeout;
        break;
    case CURLE_PEER_FAILED_VERIFICATION:
        code = Result::HttpSSLVerificationError;
        message = "SSL Verification Failed. The server certificate is not trusted";
        break;
    default:
        code = Result::ConnectionUnexpectedError;
        break;
    }

    if (message.empty())
    {
        const bool isErrorStringRegistered = errorBuffer && errorBuffer[0] != '\0';
        message = isErrorStringRegistered ? errorBuffer : "Curl error";
    }

    return Result(code, std::move(message));
}

Result HttpCodeToResult(long httpCode)
{
    switch (httpCode)
    {
    case 200:
    {
        return Result::Success;
    }
    case 400:
    {
        return Result(Result::HttpBadRequest, "400 Bad Request");
    }
    case 404:
    {
        return Result(Result::HttpNotFound, "404 Not Found");
    }
    case 405:
    {
        return Result(Result::HttpBadRequest, "405 Method Not Allowed");
    }
    case 503:
    {
        return Result(Result::HttpServiceNotAvailable, "503 Service Unavailable");
    }
    default:
    {
        return Result(Result::HttpUnexpected, "Unexpected HTTP code " + std::to_string(httpCode));
    }
    }
}
} // namespace

namespace SFS::details
{
struct CurlHeaderList
{
  public:
    CurlHeaderList() = default;

    ~CurlHeaderList()
    {
        curl_slist_free_all(m_slist);
    }

    /**
     * @throws SFSException if the header cannot be added to the list.
     */
    void Add(HttpHeader header, const std::string& value)
    {
        const std::string data = ToString(header) + ": " + value;
        const auto ret = curl_slist_append(m_slist, data.c_str());
        if (!ret)
        {
            throw SFSException(Result::ConnectionSetupFailed, "Failed to add header " + data + " to CurlHeaderList");
        }
        m_slist = ret;
    }

    struct curl_slist* m_slist{nullptr};
};
} // namespace SFS::details

CurlConnection::CurlConnection(const ReportingHandler& handler) : Connection(handler)
{
    m_handle = curl_easy_init();
    THROW_CODE_IF_LOG(ConnectionSetupFailed, !m_handle, m_handler, "Failed to init curl connection");

    // Turning timeout signals off to avoid issues with threads
    // See https://curl.se/libcurl/c/threadsafe.html
    THROW_CODE_IF_LOG(ConnectionSetupFailed,
                      curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L) != CURLE_OK,
                      m_handler,
                      "Failed to set up curl");

    // TODO #40: Allow passing user agent in the header
    // TODO #41: Pass AAD token in the header if it is available
    // TODO #42: Cert pinning with service
}

CurlConnection::~CurlConnection()
{
    if (m_handle)
    {
        curl_easy_cleanup(m_handle);
    }
}

std::string CurlConnection::Get(const std::string& url)
{
    THROW_CODE_IF_LOG(InvalidArg, url.empty(), m_handler, "url cannot be empty");

    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1L));
    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, nullptr));

    CurlHeaderList headers;
    return CurlPerform(url, headers);
}

std::string CurlConnection::Post(const std::string& url, const std::string& data)
{
    THROW_CODE_IF_LOG(InvalidArg, url.empty(), m_handler, "url cannot be empty");

    CurlHeaderList headerList;
    headerList.Add(HttpHeader::ContentType, "application/json");

    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_POST, 1L));
    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_COPYPOSTFIELDS, data.c_str()));

    CurlHeaderList headers;
    headers.Add(HttpHeader::ContentType, "application/json");
    return CurlPerform(url, headers);
}
#include "../ReportingHandler.h"
std::string CurlConnection::CurlPerform(const std::string& url, CurlHeaderList& headers)
{
    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()));
    // THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_VERBOSE, 1));

    char strpem[] = INTERMEDIATE_CERT PPE_CERT;

    struct curl_blob blob;
    blob.data = strpem;
    blob.len = strlen(strpem);
    blob.flags = CURL_BLOB_COPY;

    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_CAINFO_BLOB, &blob));

    headers.Add(HttpHeader::MSCV, m_cv.IncrementAndGet());
    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, headers.m_slist));

    // Setting up error buffer where error messages get written - this gets unset in the destructor
    CurlErrorBuffer errorBuffer(m_handle, m_handler);

    std::string readBuffer;
    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, WriteCallback));
    THROW_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &readBuffer));

    auto result = curl_easy_perform(m_handle);
    if (result != CURLE_OK)
    {
        LOG_INFO(m_handler, "result %d", result);
        THROW_LOG(CurlCodeToResult(result, errorBuffer.Get()), m_handler);
    }

    // TODO #43: perform retry logic according to response errors
    // The retry logic should also be opt-out-able by the user

    long httpCode = 0;
    THROW_IF_CURL_UNEXPECTED_ERROR(curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &httpCode));
    THROW_IF_FAILED_LOG(HttpCodeToResult(httpCode), m_handler);

    return readBuffer;
}
