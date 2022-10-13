
for n in {1 2 4 8 16}
do
    echo ${n}
    ./dbgen -T L -s ${n}

    mv lineitem.tbl lineitem_${n}.tbl
done
