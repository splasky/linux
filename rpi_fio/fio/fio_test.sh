#!/bin/bash

# Remove any existing report file
rm -f report.txt
default_dir="/mnt/"
jobs=1
iodepth=1
bs=4k
size=4G

echo "jobs: $jobs, iodepth: $iodepth, bs=$bs, size=$size" >> report.txt
# Sequential read test
fio --name=seq_read --directory="$default_dir" --ioengine=libaio --iodepth="$iodepth" --rw=read --bs="$bs" --size="$size" --numjobs="$jobs" --runtime=60 --group_reporting >> report.txt

# Sequential write test
fio --name=seq_write --directory="$default_dir" --ioengine=libaio --iodepth="$iodepth" --rw=write --bs="$bs" --size="$size" --numjobs="$jobs" --runtime=60 --group_reporting >> report.txt

# Sequential read-write test
fio --name=seq_read_write --directory="$default_dir" --ioengine=libaio --iodepth="$iodepth" --rw=rw --bs="$bs" --size="$size" --numjobs="$jobs" --runtime=60 --group_reporting >> report.txt

# Random read test
fio --name=random_read --directory="$default_dir" --ioengine=libaio --iodepth="$iodepth" --rw=randread --bs="$bs" --size="$size" --numjobs="$jobs" --runtime=60 --group_reporting >> report.txt

# Random write test
fio --name=random_write --directory="$default_dir" --ioengine=libaio --iodepth="$iodepth" --rw=randwrite --bs="$bs" --size="$size" --numjobs="$jobs" --runtime=60 --group_reporting >> report.txt

# Random read-write test
fio --name=random_read_write --directory="$default_dir" --ioengine=libaio --iodepth="$iodepth" --rw=randrw --bs="$bs" --size="$size" --numjobs="$jobs" --runtime=60 --group_reporting >> report.txt

# You can continue appending other tests as needed
