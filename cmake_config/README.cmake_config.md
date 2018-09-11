# Sampling

Sampling stuff.

Currently, I'm on version @SMPL_VERSION_STRING@.

## Install

Basic install (with 2 cpus)...

```
git clone https://github.com/mooreryan/sample_seqs.git
cd sample_seqs
mkdir build
cd build
cmake ..
make -j2
make install
```

### Switching compilers

If you're on a Mac and want to use `gcc`, you could replace the cmake line above with this

```bash
CC=/usr/local/bin/gcc-7 cmake ..
```

## Usage

Check the help banner like this

```
$ sample_seqs -h
```