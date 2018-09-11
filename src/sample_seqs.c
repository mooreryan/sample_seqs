// TODO clean up mem on errors

#include <zlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <math.h> // for ldexp

#include <rya.h>
#include <rya_file.h>
#include <rya_format.h>

#include "sampling.h"
#include "kseq_helper.h"
#include "pcg_variants.h"
#include "entropy.h"


// See http://www.pcg-random.org/using-pcg-c.html#generating-doubles
// Returns a random double from [0, 1)
double smpl_random_double(pcg32_random_t* rng)
{
  // TODO check return val
  return ldexp(pcg32_random_r(rng), -32);
}

typedef struct smpl_outf_t
{
  char* fname;
  FILE* fstream;
} smpl_outf;

smpl_outf* smpl_outf_new(char* fname)
{
  if (fname == NULL) {
    return RYA_ERROR_PTR;
  }

  smpl_outf* outf = malloc(sizeof(*outf));
  if (outf == NULL) {
    return RYA_ERROR_PTR;
  }

  outf->fname = strdup(fname);
  if (outf->fname == NULL) {
    rya_free(outf);

    return RYA_ERROR_PTR;
  }

  outf->fstream = fopen(outf->fname, "w");
  if (outf->fstream == NULL) {
    fprintf(stderr, "ERROR -- Couldn't open '%s' for writing\n", outf->fname);

    rya_free(outf->fname);
    rya_free(outf);

    return RYA_ERROR_PTR;
  }

  return outf;
}

// This function can handle NULL input.
void smpl_outf_free(smpl_outf* outf)
{
  if (outf != NULL) {
    rya_free(outf->fname);

    if (outf->fstream != NULL) {
      fclose(outf->fstream);
    }

    rya_free(outf);
  }
}


int
main(int argc, char* argv[])
{
  // Help screen stuff
  char* version_banner = rya_format(
      "  # Version:   %s\n"
      "  # Copyright: %s\n"
      "  # Contact:   %s\n"
      "  # Website:   %s\n"
      "  # License:   %s\n",
      SMPL_VERSION_STRING,
      SMPL_COPYRIGHT,
      SMPL_CONTACT,
      SMPL_WEBSITE,
      SMPL_LICENSE
  );

  if (version_banner == RYA_ERROR_PTR) {
    fprintf(stderr, "Couldn't make version banner\n");
    return 1;
  }

  char* intro =
          "  Use me to sample from (big) sequences files.  I'm pretty speedy!\n"
          "\n"
          "  You must pass in either both -1 and -2, OR -s OR -1, -2, and -s.\n"
          "\n"
          "  I will keep the paired reads together if you give me sequences using -1 and -2.  "
          "  HOWEVER, I will not check to ensure they are properly paired.  "
          "  In other words, I will ASSUME that they are in fact properly paired.\n"
          "\n"
          "  I probably won't give you an exact N percent sample.  "
          "  To be speedy, I give every read or read pair an N percent chance of being sampled.  There's a slight difference there.\n";



  char* usage =
          "-p sampling_percent "
          "-n num_samples "
          "-o outdir "
          "-b basename "
          "[-r random_seed] "
          "[-1 forward_seqs] "
          "[-2 reverse_seqs] "
          "[-s single/unpaired seqs] "
          "[-h help]\n";

  char* options =
          "    -h           Display help.\n"
          "\n"

          "    -p <float>   Percent to sample.  Must be > 0 and < 100.\n"
          "    -n <integer> Number of samples to take.  Must be >= 1.\n"
          "    -r <integer> (Optional) Random seed for sampling.  Must be >= 1.\n"
          "\n"

          "    -o <string>  The output directory.  I will create if it does not exist.\n"
          "    -b <string>  Basename for output files.\n"
          "\n"

          "    -1 <string>  (Optional) Path to forward reads.\n"
          "    -2 <string>  (Optional) Path to reverse reads.\n"
          "    -s <string>  (Optional) Path to single/unpaired reads.\n";

  char* help_banner = rya_format(
      "\n\n"
      "~~~~ Help ~~~~\n"
      "\n"
      "%s"
      "\n"
      "%s"
      "\n\n"
      "  usage: \n"
      "    %s %s"
      "\n"
      "  options:\n"
      "%s"
      "\n",
      version_banner,
      intro,
      argv[0],
      usage,
      options
  );

  pcg32_random_t rng;

  // For int return values
  int ret_val = 0;
  int i       = 0;
  int c       = 0;

  rya_bool rbool = 0;

  double sampling_percent = 0.0;

  long num_samples = 0;
  long random_seed = 0;

  char     * tmp_fname = NULL;
  smpl_outf* tmp_outf  = NULL;

  // For reading input seqs
  kseq_t* forward_seq;
  kseq_t* reverse_seq;
  kseq_t* single_seq;

  gzFile instream_forward;
  gzFile instream_reverse;
  gzFile instream_single;

  smpl_outf** outfiles_forward = NULL;
  smpl_outf** outfiles_reverse = NULL;
  smpl_outf** outfiles_single  = NULL;

  // These hold command line options
  char* opt_sampling_percent = NULL;
  char* opt_num_samples      = NULL;
  char* opt_outdir           = NULL;
  char* opt_basename         = NULL;
  char* opt_random_seed      = NULL;
  char* opt_forward_reads    = NULL;
  char* opt_reverse_reads    = NULL;
  char* opt_single_reads     = NULL;

  while ((c = getopt(argc, argv, "p:n:o:b:r:1:2:s:h")) != -1) {
    switch (c) {
      case 'p':
        opt_sampling_percent = optarg;
        break;
      case 'n':
        opt_num_samples = optarg;
        break;
      case 'o':
        opt_outdir = optarg;
        break;
      case 'b':
        opt_basename = optarg;
        break;
      case 'r':
        opt_random_seed = optarg;
        break;
      case '1':
        opt_forward_reads = optarg;
        break;
      case '2':
        opt_reverse_reads = optarg;
        break;
      case 's':
        opt_single_reads = optarg;
        break;
      case 'h':
        fprintf(stderr, "%s", help_banner);
        return 1;
      default:
        fprintf(stderr, "%s", help_banner);
        return 1;
    }
  }

  if (opt_outdir == NULL) {
    fprintf(stderr, "ERROR -- -o is a required arg\n");
    fprintf(stderr, "%s", help_banner);


    return 1;
  }
  else {
    // Set up outdir
    rbool = rya_file_exist(opt_outdir);
    if (rbool == rya_true) {
      fprintf(stderr, "ERROR -- outdir ('%s') already exists!\n", opt_outdir);

      return 1;
    }
    else if (rbool == rya_false) {
      // Make directory
      // read/write/search permissions for owner and group.
      // read/search permissions for others.
      ret_val = mkdir(opt_outdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      if (ret_val != 0) {
        fprintf(stderr, "ERROR -- Cannot create outdir '%s'", opt_outdir);

        return 1;
      }
    }
    else {
      fprintf(stderr, "ERROR -- something went wrong when checking outdir\n");

      return 1;
    }
  }

  if (opt_basename == NULL) {
    fprintf(stderr, "ERROR -- -b is a required arg\n");
    fprintf(stderr, "%s", help_banner);

    return 1;
  }

  // Check all the args
  if (opt_sampling_percent == NULL) {
    fprintf(stderr, "ERROR -- -p is a required arg\n");
    fprintf(stderr, "%s", help_banner);

    return 1;
  }
  else {
    // TODO check function call
    sampling_percent = strtod(opt_sampling_percent, NULL);

    if (sampling_percent <= 0 || sampling_percent >= 100) {
      fprintf(stderr, "ERROR -- sampling percent must be > 0 and < 100");
      fprintf(stderr, "%s", help_banner);

      return 1;
    }

    sampling_percent /= 100.0;
  }

  if (opt_num_samples == NULL) {
    fprintf(stderr, "ERROR -- -n is a required arg\n");
    fprintf(stderr, "%s", help_banner);

    return 1;
  }
  else {
    // TODO check function call
    num_samples = strtol(opt_num_samples, NULL, 10);

    if (num_samples < 1) {
      fprintf(stderr, "-n must be >= 1\n");
      fprintf(stderr, "%s", help_banner);

      return 1;
    }

    // Allocate outfiles storage
    outfiles_forward = calloc((size_t)num_samples, sizeof(smpl_outf));
    if (outfiles_forward == NULL) {
      fprintf(stderr, "ERROR -- couldn't allocate outfiles_forward\n");

      return 1;
    }

    outfiles_reverse = calloc((size_t)num_samples, sizeof(smpl_outf));
    if (outfiles_reverse == NULL) {
      fprintf(stderr, "ERROR -- couldn't allocate outfiles_reverse\n");

      return 1;
    }

    outfiles_single = calloc((size_t)num_samples, sizeof(smpl_outf));
    if (outfiles_single == NULL) {
      fprintf(stderr, "ERROR -- couldn't allocate outfiles_single\n");

      return 1;
    }
  }

  // Either both -1 and -2 or -s or all three must be given
  if (opt_forward_reads == NULL && opt_reverse_reads == NULL && opt_single_reads == NULL) {
    fprintf(stderr, "ERROR -- no read files were given\n");
    fprintf(stderr, "%s", help_banner);

    return 1;
  }

  // Forward and reverse need be both given or neither given.
  if ((opt_forward_reads == NULL && opt_reverse_reads != NULL) ||
      (opt_forward_reads != NULL && opt_reverse_reads == NULL)) {
    fprintf(stderr, "ERROR -- -1 and -2 must both be given or neither be given\n");
    fprintf(stderr, "%s", help_banner);

    return 1;
  }

  // If we're given a seed, use that, else, set one "randomly"
  if (opt_random_seed != NULL) {
    random_seed = strtol(opt_random_seed, NULL, 10);
    // TODO check that random_seed falls within uint64_t max

    if (random_seed < 1) {
      fprintf(stderr, "ERROR -- -r must be > 0\n");
      fprintf(stderr, "%s", help_banner);

      return 1;
    }

    // Use the given seed to set up rng
    pcg32_srandom_r(&rng, (uint64_t)random_seed, (uint64_t)random_seed);
  }
  else {
    // Seed with external entropy
    uint64_t seeds[2];
    entropy_getbytes((void*)seeds, sizeof(seeds));
    pcg32_srandom_r(&rng, seeds[0], seeds[1]);
  }

  if (opt_forward_reads != NULL && opt_reverse_reads != NULL) {
    // Set up the forward reads file for reading
    rbool = rya_file_exist(opt_forward_reads);
    if (rbool != rya_true) {
      fprintf(stderr, "-1 %s does not exist!\n", opt_forward_reads);

      return 1;
    }

    instream_forward = gzopen(opt_forward_reads, "r");
    if (instream_forward == Z_NULL) {
      fprintf(stderr,
              "ERROR -- couldn't open '%s' for reading\n",
              opt_forward_reads);

      return 1;
    }

    // TODO check init call
    forward_seq = kseq_init(instream_forward);

    // Set up the reverse reads file for reading
    rbool = rya_file_exist(opt_reverse_reads);
    if (rbool != rya_true) {
      fprintf(stderr, "-2 %s does not exist!\n", opt_reverse_reads);

      return 1;
    }

    instream_reverse = gzopen(opt_reverse_reads, "r");
    if (instream_reverse == Z_NULL) {
      fprintf(stderr,
              "ERROR -- couldn't open '%s' for reading\n",
              opt_reverse_reads);

      return 1;
    }

    // TODO check init call
    reverse_seq = kseq_init(instream_reverse);

    // Set up the paired outfiles
    for (i = 0; i < num_samples; ++i) {
      // Forward reads outf
      tmp_fname = rya_format("%s/%s.sample_%d.1.fq", opt_outdir, opt_basename, i);
      if (tmp_fname == RYA_ERROR_PTR) {
        fprintf(stderr, "ERROR -- can't format tmp_fname forward idx %d\n", i);

        return 1;
      }

      tmp_outf = smpl_outf_new(tmp_fname);
      if (tmp_outf == RYA_ERROR_PTR) {
        fprintf(stderr, "ERROR -- can't make tmp_outf forward idx %d\n", i);

        return 1;
      }

      outfiles_forward[i] = tmp_outf;
      rya_free(tmp_fname);

      // Reverse reads outf
      tmp_fname = rya_format("%s/%s.sample_%d.2.fq", opt_outdir, opt_basename, i);
      if (tmp_fname == RYA_ERROR_PTR) {
        fprintf(stderr, "ERROR -- can't make tmp_fname reverse idx %d\n", i);

        return 1;
      }

      tmp_outf = smpl_outf_new(tmp_fname);
      if (tmp_outf == RYA_ERROR_PTR) {
        fprintf(stderr, "ERROR -- can't make tmp_outf reverse idx %d\n", i);

        return 1;
      }

      outfiles_reverse[i] = tmp_outf;
      rya_free(tmp_fname);
    }

    // Read the paired files
    // TODO mention in help screen that files must be properly paired.
    while (kseq_read(forward_seq) >= 0) {

      // TODO read one stream, then jump back the rng by number of sequences, then read the other stream.
      if (kseq_read(reverse_seq) < 0) {
        fprintf(stderr,
                "ERROR -- not enough reads in reverse reads file '%s'\n",
                opt_reverse_reads);

        return 1;
      }

      for (i = 0; i < num_samples; ++i) {
        if (smpl_random_double(&rng) < sampling_percent) {
          kseq_write(outfiles_forward[i]->fstream, forward_seq);
          kseq_write(outfiles_reverse[i]->fstream, reverse_seq);
        }
      }
    }

    // Clean up
    kseq_destroy(forward_seq);
    kseq_destroy(reverse_seq);
    gzclose(instream_forward);
    gzclose(instream_reverse);

    for (i = 0; i < num_samples; ++i) {
      smpl_outf_free(outfiles_forward[i]);
      smpl_outf_free(outfiles_reverse[i]);
    }
  }

  if (opt_single_reads != NULL) {
    // Set up the single reads

    rbool = rya_file_exist(opt_single_reads);
    if (rbool != rya_true) {
      fprintf(stderr, "-s %s does not exist!\n", opt_single_reads);

      return 1;
    }

    instream_single = gzopen(opt_single_reads, "r");
    if (instream_single == Z_NULL) {
      fprintf(stderr,
              "ERROR -- couldn't open '%s' for reading\n",
              opt_single_reads);

      return 1;
    }

    // TODO check init call
    single_seq = kseq_init(instream_single);

    // And set up the single reads outfiles

    for (i = 0; i < num_samples; ++i) {
      // Single reads outf
      tmp_fname = rya_format("%s/%s.sample_%d.U.fq", opt_outdir, opt_basename, i);
      if (tmp_fname == RYA_ERROR_PTR) {
        fprintf(stderr, "ERROR -- can't make tmp_fname single idx %d\n", i);

        return 1;
      }

      tmp_outf = smpl_outf_new(tmp_fname);
      if (tmp_outf == RYA_ERROR_PTR) {
        fprintf(stderr, "ERROR -- can't make tmp_outf single idx %d\n", i);

        return 1;
      }

      outfiles_single[i] = tmp_outf;
      rya_free(tmp_fname);
    }

    // Read the single sequences.
    while (kseq_read(single_seq) >= 0) {
      for (i = 0; i < num_samples; ++i) {
        if (smpl_random_double(&rng) < sampling_percent) {
          kseq_write(outfiles_single[i]->fstream, single_seq);
        }
      }
    }

    // Clean up
    kseq_destroy(single_seq);
    gzclose(instream_single);

    for (i = 0; i < num_samples; ++i) {
      smpl_outf_free(outfiles_single[i]);
    }
  }

  return 0;
}
