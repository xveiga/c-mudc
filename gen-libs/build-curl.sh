#!/bin/bash

# Load configuration
BASE_DIR="$( cd "$(dirname "$0")" ; pwd -P )" # Script location directory
source ${BASE_DIR}/config-vars.sh && \

echo -e "${YELLOW}Will build cURL ${CURL_VERSION}${NC}" && \

cd ${CURL_SRC_DIR} && \
./buildconf && \

echo -e "${GREEN}Configuring cURL...${NC}" && \
echo -e "${YELLOW}Will link with OpenSSL from '$OPENSSL_BUILD_DIR${NC}''" && \

export CFLAGS && \
export CPPFLAGS="${CFLAGS} -I${OPENSSL_BUILD_DIR}/include/openssl" && \
export LDFLAGS="${LDFLAGS} -L${OPENSSL_BUILD_DIR}/lib" && \

# Clear target directories in project for current arch
echo -e "Cleaning build directory: ${CURL_BUILD_DIR}" && \
mkdir -p ${CURL_BUILD_DIR} && \

LIBS="-ldl" ./configure --prefix=${CURL_BUILD_DIR} \
            --with-ssl=${OPENSSL_BUILD_DIR} \
            --disable-shared \
            --disable-verbose \
            --disable-manual \
            --disable-crypto-auth \
            --disable-unix-sockets \
            --disable-ares \
            --disable-rtsp \
            --disable-proxy \
            --disable-versioned-symbols \
            --enable-hidden-symbols \
            --without-libidn \
            --without-librtmp \
            --without-zlib \
            --without-libssh2 \
            --without-libpsl \
            --without-nghttp2 \
            --disable-dict \
            --disable-file \
            --disable-ftp \
            --disable-ftps \
            --disable-gopher \
            --disable-imap \
            --disable-imaps \
            --disable-ldap \
            --disable-pop3 \
            --disable-pop3s \
            --disable-smb \
            --disable-smbs \
            --disable-smtp \
            --disable-smtps \
            --disable-telnet \
            --disable-tftp  && \

echo -e "${YELLOW}Building cURL...${NC}"  && \
make -j$NJOBS && \
make install && \
make clean
if [ "$?" -eq 0 ]; then
    echo -e "${LIGHT_GREEN}Completed cURL ${CURL_VERSION} build${NC}"
else
    echo -e "${RED}cURL ${CURL_VERSION} build failed${NC}"
    exit 1
fi

cd ..
exit 0