# Sampling

Sample reads randomly....paired, single, or both at the same time!

See [below](#usage) for usage info.

## Version

Currently, I'm on version 0.1.3.

## Install

### Dependencies

If you don't have the `rya` C library, first go [here](TODO) and follow the install instructions.

### Install instructions

Make a directory to hold the source code.

```bash
mkdir sample_seqs_source
cd sample_seqs_source
```

Then download the code (use `curl` or `wget` but not both!).  

If you have `curl` run:

```bash
\curl -L 'https://github.com/mooreryan/sample_seqs/archive/v0.1.3.tar.gz' > v0.1.3.tar.gz
```

If you have `wget`, run

```bash
wget 'https://github.com/mooreryan/sample_seqs/archive/v0.1.3.tar.gz'
```

Unzip the tar file and enter the resulting directory.

```bash
tar xzf v0.1.3.tar.gz
cd sample_seqs-0.1.3
```

Now we can build and install the library.

```bash
mkdir build
cd build
cmake ..
make
make install
```

### Run tests

There are some tests that you can run if you want to check that everything is working.  Right after the `make install` command, run

```bash
make test
```

If it passes you're good to go!

### Potential problems

#### Can't find the `rya` library

* Did you install the `rya` library?  If not go [here](TODO) and follow the install instructions.
* If you installed the `rya` library in a non-standard location, (e.g., `$HOME/my_cool_libraries`), you need to specify that path to `cmake` like so:

```bash
cmake -DCMAKE_PREFIX_PATH=$HOME/my_cool_libraries ..
```

Then you can continue on with the `make` and `make install` commands as above.

#### Switching compilers

If you're on a Mac you may need to use an actual `gcc` rather than the default apple compiler.  If you're having issues, try running the `cmake` command like this (as opposed to what is shown above):

```bash
CC=/usr/local/bin/gcc-7 cmake ..
```

Then continue on with the `make` and `make install` commands.

*Note:  depending on which version of `gcc` you have installed, you may need to change `gcc-7` (version 7), to `gcc-8` (version 8), for example.*


#### Changing the install directory 

If you want to change the location where the program is installed (for example, if you don't have sudo privileges), you can run `cmake` like this

```bash
cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/location ..
```

#### No super user privileges

You may need to run `sudo make install` rather than just `make install` depending on where you want the library installed.


## Usage

### Help banner

Check the help banner like this

```
$ sample_seqs -h
```

Among other things, you'll see this usage banner.

```
  usage:
    sample_seqs -p sampling_percent -n num_samples -o outdir -b basename [-r random_seed] [-1 forward_seqs] [-2 reverse_seqs] [-s single/unpaired seqs] [-h help]
```

### Usage notes

#### FastA files work too

- For what it's worth, I can sample from fastA files as well as fastQ files.  
  - However, I always append `.fq` to the output file names regardless of whether or not you passed in a fastA file or a fastQ file.  (It's something I'll fix at some point!)

#### Paired read sampling

- If you want to sample from paired read files, I will not break up pairs (e.g., I will sample both the forward and reverse read so that you can keep pairs together!)

- I will not check to ensure that your paired reads are properly paired.  
  - In other words, I will assume that they are in fact properly paired.
  - What I mean by this is that the first read in the forward file should be paired with the first read in the reverse file, and so on.
  - Normally this is not a problem, unless you are trying to sample from reads that have been broken up by something (like maybe QC) and not been properly repaired.
  - If you're worried that your reads might not be properly paired, you could try running [this program](https://github.com/mooreryan/FixPairs) to check.

#### Paired reads, single reads, or both

There are a couple options for sampling reads.

- You can sample from paired files
  - To do so you must specify both the `-1` and the `-2` arguments (with the file names).

- You can sample from single (non-paired files)
  - To do so, you should only specify the `-s` flag with the file to sample from.

- You can sample from paired files as well as single files
  - Sometimes, people like to merge paired reads.  In such cases, you might have both paired and unpaired sequences after running quality control on your library.
  - If you what to take a subsample of your library and you want to include reads from both the paired and the merged files, you can do that!
  - To do so, you should specify all three flags (`-1`, `-2`, and `-s`). 

#### Random sampling

- I use a really snazzy random number generator.  See [here](http://www.pcg-random.org) for more details.

- I probably won't give you an exact N percent sample.
  - To be speedy, I give every read or read pair an N percent chance of being sampled.
  
