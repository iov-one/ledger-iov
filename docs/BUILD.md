# IOV App for Ledger Nano S / X

## Get source
Apart from cloning, be sure you get all the submodules, by calling:
```
git submodule update --init --recursive
```

## Dependencies

#### Ledger Nano S

This project requires ledger firmware 1.5.5

The current repository keeps track of Ledger's SDK but it is possible to override it by changing the git submodule.

#### Docker CE

Please install docker CE. The instructions can be found here: https://docs.docker.com/install/

#### Ubuntu Dependencies
Install the following packages:
```
sudo apt update && apt-get -y install build-essential git wget cmake libssl-dev libgmp-dev autoconf libtool
```

#### OSX Dependencies
It is recommended that you install brew and xcode.

Additionally you will need to:

```
brew install libusb
```

#### Other dependencies

You will need to have python 3. In most cases, `make deps` should be able to install all dependencies:

```bash
make deps
```

(Ledger firmware 1.5.5 requires python 3 and ledgerblue >= 0.1.21. )

# Building the Ledger App
In order to keep builds reproducible, a Makefile is provided.

The Makefile will build the firmware in a docker container and leave the binary in the correct directory.

**Build**

The following command will build the app firmware inside a container. All output will be available to the host.
```
make                # Builds the app
```

**Upload the app to the device**
The following command will upload the application to the ledger. _Warning: The application will be deleted before uploading._
```
make load          # Builds and loads the app to the device
```
### Developers (building C++ Code / Tests)

This is useful when you want to make changes to libraries, run unit tests, etc. It will build all common libraries and unit tests.

**Compile**
```
cmake -DDISABLE_DOCKER_BUILDS=ON . && make
```
**Run unit tests**
```
export GTEST_COLOR=1 && ctest -VV
```
