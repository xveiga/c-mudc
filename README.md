# c-mudc
Basic file downloader for UDC's Moodle service, using cURL, OpenSSL and json-c

## Purpose and motivation
Download all files from each course the user is enrolled on, for archival purposes.

There are some old tools for this exact purpose written on different languages, including my own,
but they stopped working a while ago. The UDC's Moodle server uses the `TLS_RSA_WITH_3DES_EDE_CBC_SHA`
over `TLSv1.2` cipher, which was deprecated and deactivated on the latest versions of OpenSSL.
Attempting to connect to the server on high-level languages will fail on modern devices,
as they use the system's OpenSSL library (Java, Python, Android).
The easiest solution I've found is to get the latest version that supports that cipher by default,
which is `1.0.2t`, and statically link the program to the library.
Doing this on any high-level language will likely require a huge amount of work, which is why
I've decided to redo this program directly on C.

If you're interested on doing something similar for Android, check out my attempt: [CASLoginAndroid](https://github.com/xveiga/CASLoginAndroid) (login only example, has no useful functionality).

## Installation
**(Tested on linux only, Windows support planned).**

### 1. Clone repo
`git clone https://github.com/xveiga/c-mudc.git`

or

`git clone https://github.com/xveiga/c-mudc`

or

[Download ZIP](https://github.com/xveiga/c-mudc/archive/master.zip)

### 2. Compile dependencies
There are scripts provided to download and compile OpenSSl, cURL and c-json on the `gen-libs` directory.

To compile all in one go, run:
`./gen-libs/build-all.sh`

If build fails, is probably because you are missing a dependency. Check the script output for details.

### 3. Compile program
A makefile is included for this, just run `make`.
Compilation will fail if the dependencies were not built properly.

## Usage
Two executables are generated, `moodle-curl` and `check-versions`.

`check-versions` can be used to verify that you are linking with the compiled OpenSSL and cURL and not your system libraries.

`moodle-curl` is the main program. It needs your access token as the first argument.
Upon executing it, it will proceed to download all files for all courses for the owner of the token.
There's no file duplicate or timestamp checking implemented yet, so be careful as this tool generates a lot of traffic.

## Getting Moodle access token
The token API is deactivated on UDC's Moodle as they use a custom system ("CAS"). To get the token you'll have
to log in manually on a web browser with your account, navigate to `Preferences > User > Security Keys` and copy
the token named **"Moodle mobile web service"**.
