#!/bin/bash

# Load configuration
BASE_DIR="$( cd "$(dirname "$0")" ; pwd -P )" # Script location directory
source ${BASE_DIR}/config-vars.sh && \

export CFLAGS && \
export LDFLAGS && \

cd $OPENSSL_SRC_DIR && \

echo -e "${YELLOW}Will build OpenSSL ${OPENSSL_VERSION}${NC}" && \

echo -e "${GREEN}Configuring OpenSSL...${NC}" && \

./config $OPENSSL_CONFIGURATION \
    --prefix=$OPENSSL_BUILD_DIR && \

# Clear target directories in project for current arch
echo -e "Cleaning build directory: ${OPENSSL_BUILD_DIR}" && \
mkdir -p $OPENSSL_BUILD_DIR && \

echo -e "${YELLOW}Building OpenSSL...${NC}" && \
make clean && \
make depend -j${NJOBS} && \ # Rebuild dependencies to prevent incompatible target problems
make -j${NJOBS} && \
make install_sw
if [ "$?" -eq 0 ]; then
    echo -e "${LIGHT_GREEN}Completed OpenSSL ${OPENSSL_VERSION} build${NC}"
else
    echo -e "${RED}OpenSSL ${OPENSSL_VERSION} build failed${NC}"
    exit 1
fi

cd ..
exit 0
