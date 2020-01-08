#!/bin/bash

# Load configuration
BASE_DIR="$( cd "$(dirname "$0")" ; pwd -P )" # Script location directory
source ${BASE_DIR}/config-vars.sh  && \

echo "Removing stale source files..." && \
rm -rf ${BASE_DIR}/src && \

echo "Resetting dependencies..." && \
${BASE_DIR}/download-libs.sh && \

echo "Building json-c..." && \
${BASE_DIR}/build-json-c.sh && \

echo "Building OpenSSL..." && \
${BASE_DIR}/build-openssl.sh && \

echo "Building cURL..." && \
${BASE_DIR}/build-curl.sh

if [ "$?" -eq 0 ]; then
    echo -e "${LIGHT_GREEN}Completed dependencies build${NC}"
    exit 0
else
    echo -e "${RED}Dependencies build failed${NC}"
    exit 1
fi