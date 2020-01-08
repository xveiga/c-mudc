#!/bin/bash

# Load configuration
BASE_DIR="$( cd "$(dirname "$0")" ; pwd -P )" # Script location directory
source ${BASE_DIR}/config-vars.sh

create_tar_dir() {
    if [ ! -d {$BASE_DIR}/tar ]; then
        mkdir -p ${BASE_DIR}/tar
    fi
}

#TODO: Checksum of downladed files, redownload if corrupt.

download_openssl() {
    if [ ! -f "${BASE_DIR}/tar/openssl-${OPENSSL_VERSION}.tar.gz" ]; then
        echo -e "${ORANGE}-> Downloading OpenSSL ${OPENSSL_VERSION} sources${NC}"
        curl -Lo "${BASE_DIR}/tar/openssl-${OPENSSL_VERSION}.tar.gz" "https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz"
    else
        echo -e "${GREEN}-> OpenSSL $OPENSSL_VERSION archive found${NC}"
    fi
}

download_curl() {
    if [ ! -f "${BASE_DIR}/tar/curl-${CURL_VERSION}.tar.gz" ]; then
        echo -e "${ORANGE}-> Downloading cURL ${CURL_VERSION} sources${NC}"
        curl -Lo "${BASE_DIR}/tar/curl-${CURL_VERSION}.tar.gz" "https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz"
    else
        echo -e "${GREEN}-> cURL ${CURL_VERSION} archive found${NC}"
    fi
}

download_jsonc() {
    if [ ! -f "${BASE_DIR}/tar/json-c-${JSONC_VERSION}.tar.gz" ]; then
        echo -e "${ORANGE}-> Downloading json-c ${JSONC_VERSION} sources${NC}"
        curl -Lo "${BASE_DIR}/tar/json-c-${JSONC_VERSION}.tar.gz" "https://s3.amazonaws.com/json-c_releases/releases/json-c-${JSONC_VERSION}.tar.gz"
    else
        echo -e "${GREEN}-> json-c ${JSONC_VERSION} archive found${NC}"
    fi
}

download_mozilla_certs() {
    # Create directories if they do not exist
    if [ ! -d $(dirname ${CA_STORE_FILE}) ]; then
        mkdir -p $(dirname ${CA_STORE_FILE})
    fi
    if [ ! -f ${CA_STORE_FILE} ]; then
        echo -e "${ORANGE}-> Downloading Mozilla CA certificate store${NC}"
        curl -Lo ${CA_STORE_FILE} "https://curl.haxx.se/ca/cacert.pem"
    else
        echo -e "${GREEN}-> Mozilla CA certificate store found, checking for updates.${NC}"
        # Check if the shasum of the latest cert store matches and download if necessary.
        echo -e "-> Checking sha256sums of local and remote files"
        REMOTE_SHASUM=$(curl -L https://curl.haxx.se/ca/cacert.pem.sha256 2>/dev/null | cut -d ' ' -f 1)
        LOCAL_SHASUM=$(sha256sum ${CA_STORE_FILE} | cut -d ' ' -f 1)
        #echo "  Local  ${LOCAL_SHASUM}"
        #echo "  Remote ${REMOTE_SHASUM}"
        if [ "${LOCAL_SHASUM}" != "${REMOTE_SHASUM}" ]; then
            echo -e "${ORANGE}-> Mozilla CA certificate store is outdated, updating...${NC}"
            curl -Lo ${CA_STORE_FILE} "https://curl.haxx.se/ca/cacert.pem"
            echo "${REMOTE_SHASUM}" > ${BASE_DIR}/tar/cacert.pem.sha256
            echo -e "${GREEN}-> Mozilla CA certificate store is up to date${NC}"
        else
            echo -e "${GREEN}-> Mozilla CA certificate store is up to date${NC}"
        fi
    fi
}

clean_sources() {
    if [ -d {$BASE_SRC_DIR} ]; then
        echo -e "-> Removing old source files..."
        rm -rf ${BASE_SRC_DIR}
    fi
    mkdir -p ${BASE_SRC_DIR}
}

unpack_openssl() {
    echo -e "-> Unpacking OpenSSL source..."
    tar xzf "${BASE_DIR}/tar/openssl-${OPENSSL_VERSION}.tar.gz" -C "${BASE_SRC_DIR}"
}

unpack_curl() {
    echo -e "-> Unpacking cURL source..."
    tar xzf "${BASE_DIR}/tar/curl-${CURL_VERSION}.tar.gz" -C "${BASE_SRC_DIR}"
}

unpack_jsonc() {
    echo -e "-> Unpacking json-c source..."
    tar xzf "${BASE_DIR}/tar/json-c-${JSONC_VERSION}.tar.gz" -C "${BASE_SRC_DIR}"
}

# Main

echo -e "Checking if original sources exist..."

create_tar_dir
clean_sources

download_openssl
unpack_openssl

download_curl
unpack_curl

download_jsonc
unpack_jsonc

#download_mozilla_certs

echo -e "${LIGHT_CYAN}Done.${NC}"
