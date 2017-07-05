if [ -z $exec_prefix ]
then
    exec_prefix=/usr/local
fi

for file in $(ls *.txt)
do
    $exec_prefix/bin/insertionfinder --generate-algfile -f $file -a ${file/.txt/.algs}
done
