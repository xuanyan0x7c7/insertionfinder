# insertionfinder

```
insertionfinder

General options:
    --version                             print version string
    -h [ --help ]                         produce help message

Commands:
    -s [ --solve ]                        find insertions!
    -v [ --verify ]                       verify cube state
    -g [ --generate ]                     generate algorithm files

Configurations:
    -a [ --algfile ] arg                  algorithm file
    --algs-dir arg (=/usr/local/share/insertionfinder/<version>/algorithms)
                                          algorithms directory
    --all-algs                            all algorithms
    --all-extra-algs                      all extra algorithms
    -f [ --file ] arg                     input file
    -o [ --optimal ]                      search for optimal solutions
    --target arg                          search target
    --enable-replacement                  enable replacement
    --greedy-threshold arg (=2)           suboptimal moves tolerance
    --replacement-threshold arg (=0)      tolerance to insert an algorithm
                                          remains number of insertions
    -j [ --jobs ] [=arg(=8)] (=1)         multiple threads
    --json                                use JSON output
    --verbose                             verbose
```
