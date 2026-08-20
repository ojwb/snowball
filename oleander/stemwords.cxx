/* This is a simple program which uses the generated C++ stemmers to provide a
 * command line interface for stemming using any of the algorithms provided.
 */

#ifndef NO_STEM
# include OLEANDER_HEADER
#endif
#define SNOWBALL_WIDE

#include <exception>
#include <fstream>
#include <iostream>

#include <ctype.h>  /* for tolower */
#include <string.h>  /* for strcmp */

#define STRINGIZE(X) STRINGIZE_(X)
#define STRINGIZE_(X) #X

const char * progname;
static int pretty = 1;

template<typename Char, typename Stemmer>
void
stem_file(Stemmer& stemmer,
          std::basic_istream<Char>& f_in,
          std::basic_ostream<Char>& f_out)
{
    std::basic_string<Char> word;
    size_t total_size = 0;

    while (true) {
        Char ch = f_in.get();
        if (ch < 0) {
            f_out << total_size << '\n';
            return;
        }
        {
            word.clear();
            int inlen = 0;
            while (ch != '\n' && ch >= 0) {
                if (sizeof(Char) == 1) {
                    /* Update count of utf-8 characters. */
                    if (ch < 0x80 || ch > 0xBF) inlen += 1;
                } else {
                    inlen += 1;
                }
                /* force lower case: */
                if (ch < 128) ch = tolower(ch);

                word += ch;
                ch = f_in.get();
            }
            if (pretty) {
                f_out << word;
            }

            {
                stemmer(word);
                if (pretty == 1) {
                    f_out << " -> ";
                } else if (pretty == 2) {
                    if (word.size() > 0) {
                        int j;
                        if (inlen < 30) {
                            for (j = 30 - inlen; j > 0; j--)
                                f_out << ' ';
                        } else {
                            f_out << '\n';
                            for (j = 30; j > 0; j--)
                                f_out << ' ';
                        }
                    }
                }

                // Don't write out word as it will be longer with NO_STEM
                // defined, but do something with it so the compiler can't
                // just optimise away any of the stemming step.
                total_size += word.size();
            }
        }
    }
    return;
}

/** Display the command line syntax, and then exit.
 *  @param n The value to exit with.
 */
static void
usage(int n)
{
    printf("usage: %s [-l <language>] [-i <input file>] [-o <output file>] [-p[2]] [-h]\n"
          "\n"
          "The input file consists of a list of words to be stemmed, one per\n"
          "line. Words should be in lower case, but (for English) A-Z letters\n"
          "are mapped to their a-z equivalents anyway. If omitted, stdin is\n"
          "used.\n"
          "\n",
          progname);
    printf(
          "If -p is given the output file consists of each word of the input\n"
          "file followed by \"->\" followed by its stemmed equivalent.\n"
          "If -p2 is given the output file is a two column layout containing\n"
          "the input words in the first column and the stemmed equivalents in\n"
          "the second column.\n"
          "Otherwise, the output file consists of the stemmed words, one per\n"
          "line.\n"
          "\n"
          "-h displays this help\n");
    exit(n);
}

int
main(int argc, char * argv[])
try {
    const char * in = NULL;
    const char * out = NULL;
    const char * language = "english";

    int i = 1;
    pretty = 0;

    progname = argv[0];

    while (i < argc) {
        const char * s = argv[i++];
        if (s[0] == '-') {
            if (strcmp(s, "-o") == 0) {
                if (i >= argc) {
                    fprintf(stderr, "%s requires an argument\n", s);
                    exit(1);
                }
                out = argv[i++];
            } else if (strcmp(s, "-i") == 0) {
                if (i >= argc) {
                    fprintf(stderr, "%s requires an argument\n", s);
                    exit(1);
                }
                in = argv[i++];
            } else if (strcmp(s, "-l") == 0) {
                if (i >= argc) {
                    fprintf(stderr, "%s requires an argument\n", s);
                    exit(1);
                }
                language = argv[i++];
            } else if (strcmp(s, "-p2") == 0) {
                pretty = 2;
            } else if (strcmp(s, "-p") == 0) {
                pretty = 1;
            } else if (strcmp(s, "-h") == 0) {
                usage(0);
            } else {
                fprintf(stderr, "option %s unknown\n", s);
                usage(1);
            }
        } else {
            fprintf(stderr, "unexpected parameter %s\n", s);
            usage(1);
        }
    }

#ifndef SNOWBALL_WIDE
# error non-widechars not handled here
    /* prepare the files */
    std::ifstream f_in;
    if (in) {
        f_in.open(in);
        if (!f_in.is_open()) {
            fprintf(stderr, "file %s not found\n", in);
            exit(1);
        }
    }
    std::ofstream f_out;
    if (out) {
        f_out.open(out);
        if (!f_out.is_open()) {
            fprintf(stderr, "file %s cannot be opened\n", out);
            exit(1);
        }
    }

    /* do the stemming process: */
    stem_file(*stemmer, in ? f_in : std::cin, out ? f_out : std::cout);
#else
    /* prepare the files */
    std::wifstream f_in;
    f_in.open(in ? in : "/dev/stdin");
    if (!f_in.is_open()) {
        fprintf(stderr, "file %s not found\n", in);
        exit(1);
    }
    std::wofstream f_out;
    f_out.open(out ? out : "/dev/stdout");
    if (!f_out.is_open()) {
        fprintf(stderr, "file %s cannot be opened\n", out);
        exit(1);
    }

    std::locale c_utf8("C.UTF8");
    f_in.imbue(c_utf8);
    f_out.imbue(c_utf8);

    /* do the stemming process: */
#ifdef NO_STEM
    // Allow skipping the stemming so we can measure the overhead
    // of stemwords to get just the time to do the actual stemming.
    (void)language;
    struct DummyStemmer {
        void operator()(const std::wstring&) {}
    };
    DummyStemmer stemmer;
    stem_file(stemmer, f_in, f_out);
#else
    if (strcmp(language, STRINGIZE(LANGUAGE)) != 0) {
	fprintf(stderr, "Built for '" STRINGIZE(LANGUAGE) "', not for '%s'\n", language);
	exit(1);
    }
    // Only compile for one language at a time - compiling in all the Oleander
    // stemmers more than doubles the percentage difference compared to
    // Snowball for some reason.
    stemming::OLEANDER_CLASS<> stemmer;
    stem_file(stemmer, f_in, f_out);
#endif
#endif

    return 0;
}
catch (const std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what());
    exit(1);
}
