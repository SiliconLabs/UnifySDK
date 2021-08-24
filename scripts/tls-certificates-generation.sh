#!/bin/bash
set -e

BROKER_HOST=$1
if [ -z "$BROKER_HOST" ]
then
    BROKER_HOST="localhost"
fi

echo "Creating TLS certificates with broker hostname/IP:${BROKER_HOST}"
BROKER_CN="/CN=${BROKER_HOST}/"

if [ -d "certs" ]
then 
  echo "ERROR: Directory certs/ exists. Exiting"
  exit 1
fi
mkdir -p certs/{ca,broker,client,pfx}
cd certs

echo "Generating CA certificate and Key"
echo "---------------------------------"
openssl req -nodes -subj "/CN=silicon-ca/O=Silabs/OU=UIC" -new -x509 -days 365 -extensions v3_ca -keyout ca/ca.key -out ca/ca.crt


echo "Generating Key for Broker"
echo "---------------------------------"
openssl genrsa -out broker/broker.key 2048

echo "Generating Certificate for Broker"
echo "---------------------------------"
openssl req -subj ${BROKER_CN} -out broker/broker.csr -key broker/broker.key -new

echo "Self-Signing Broker certificate with above CA"
echo "---------------------------------"
openssl x509 -req  -in broker/broker.csr -CA ca/ca.crt -CAkey ca/ca.key -CAcreateserial -out broker/broker.crt -days 360

#################################################################"
######         STEP 3  Client certificates	    ###############"

echo "Generating Client Key"
echo "---------------------------------"
openssl genrsa -out client/client.key 2048

echo "Generating Client Certificate"
echo "---------------------------------"
openssl req -subj "/" -out client/client.csr -key client/client.key -new

#################################################################"
######         STEP 3.1 Signing of Client certificates	#######"
echo "Self-Signing Client Certificate with above CA"
echo "---------------------------------"
openssl x509 -req -in client/client.csr -CA ca/ca.crt -CAkey ca/ca.key -CAcreateserial -out client/client.crt -days 360


echo "Making PFX Client certificate"
echo "---------------------------------"

openssl pkcs12 -export -out pfx/client.pfx -inkey client/client.key -in client/client.crt -passout pass:
