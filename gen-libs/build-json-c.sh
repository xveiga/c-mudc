#!/bin/bash

# Load configuration
BASE_DIR="$( cd "$(dirname "$0")" ; pwd -P )" # Script location directory
source ${BASE_DIR}/config-vars.sh && \

echo -e "${YELLOW}Will build json-c ${JSONC_VERSION}${NC}" && \
cd ${JSONC_SRC_DIR} && \

echo -e "${GREEN}Configuring json-c...${NC}" && \
sh autogen.sh && \
./configure --prefix=${JSONC_BUILD_DIR} && \

echo -e "Cleaning build directory: ${JSONC_BUILD_DIR}" && \
mkdir -p ${JSONC_BUILD_DIR} && \

echo -e "${YELLOW}Building json-c...${NC}" && \
make -j$NJOBS && \
make check && \
make install
if [ "$?" -eq 0 ]; then
    echo -e "${LIGHT_GREEN}Completed json-c ${JSONC_VERSION} build${NC}"
else
    echo -e "${RED}json-c ${JSONC_VERSION} build failed${NC}"
    exit 1
fi

cd ..
exit 0