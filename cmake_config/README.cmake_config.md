# Sampling

Sampling stuff.

Currently, I'm on version @SMPL_VERSION_STRING@.

## Install

Basic install...

```
git clone https://github.com/mooreryan/distance.git
cd sampling
mkdir build
cd build
cmake ..
make
make install
```

### Switching compilers

If you're on a Mac and want to use `gcc`, you could replace the cmake line above with this

```bash
CC=/usr/local/bin/gcc-7 cmake ..
```

## Tests

If you want to run the tests, you need to have `ceedling` installed.

```bash
gem install ceedling
```

Then run the tests.

```bash
ceedling
```