# MBR_or_GPT_partition_creator

MBR/GPT partition creator

`MBR` folder:

This folder contains a project that creates an MBR partition table on an image filled with zeros, and also creates an SMBR with 6 partitions.

`GPT` folder:

This folder contains a project that, on an image filled with zeros, creates a GPT partition table with the number of partitions passed through the command line.

`GPT_filler` folder:

This folder contains a project that, on an image filled with zeros, creates a GPT partition table with the number of partitions equal to the number of files passed through the command line. At the same time, the program fills the partition with the contents of the file, which allows us to place different file systems for each volume inside the volumes.